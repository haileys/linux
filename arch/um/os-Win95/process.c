#include "asm/segment.h"
#include <as-layout.h>
#include <asm/current.h>
#include <asm/trapnr.h>
#include <asm/desc.h>
#include <generated/asm-offsets.h>
#include <kern_util.h>
#include <linux/compiler.h>
#include <linux/kmsg_dump.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <linux/panic.h>
#include <linux/printk.h>
#include <linux/mm_types.h>
#include <os.h>
#include <registers.h>
#include <sysdep/tls.h>
#include <sysdep/ptrace.h>
#include <wsl9x.h>
#include <wsl9x/mem.h>
#include <wsl9x/sync.h>
#include <wsl9x/task.h>
#include <wsl9x/descriptor.h>
#include <wsl9x/entry.h>
#include <wsl9x/vmm.h>
#include <uapi/linux/errno.h>
#include <uapi/asm/processor-flags.h>
#include <uapi/asm/ldt.h>
#include "process.h"

#define INIT_JMP_NEW_THREAD 0
#define INIT_JMP_CALLBACK 1
#define INIT_JMP_HALT 2
#define INIT_JMP_REBOOT 3
#define INIT_JMP_RETURN 4

static volatile int top_jmpbuf_ok = 0;
static jmp_buf top_jmpbuf;

static jmp_buf* resume_jmpbuf;

const char* wsl9x_panic_msg;

uint16_t wsl9x_user_code;
uint16_t wsl9x_user_data;

uint16_t tls_gdt_indexes[GDT_ENTRY_TLS_ENTRIES];

void __init wsl9x_init_process(void)
{
	uint64_t code = VMM_BuildDescriptorDWORDs(0, 0xfffff, CODE_TYPE | D_DPL3, D_PAGE32);
	wsl9x_user_code = (uint16_t)VMM_Allocate_GDT_Selector(code >> 32, code, 0);

	uint64_t data = VMM_BuildDescriptorDWORDs(0, 0xfffff, RW_DATA_TYPE | D_DPL3, D_PAGE32);
	wsl9x_user_data = (uint16_t)VMM_Allocate_GDT_Selector(data >> 32, data, 0);

	uint64_t tls = VMM_BuildDescriptorDWORDs(0, 0xfffff, R_DATA_TYPE | D_DPL3, D_PAGE32);

	for (int i = 0; i < GDT_ENTRY_TLS_ENTRIES; i++) {
		uint16_t sel = VMM_Allocate_GDT_Selector(tls >> 32, tls, 0);
		if (!sel) {
			VMM_Fatal_Error("VMM_Allocate_GDT_Selector failed", 0);
		}

		tls_gdt_indexes[i] = sel >> 3;
	}
}

int start_idle_thread(void *stack, jmp_buf *switch_buf)
{
	top_jmpbuf_ok = 1;
	int n = setjmp(top_jmpbuf);

	if (n == 0) {
		(*switch_buf)[0].JB_IP = (unsigned long) uml_finishsetup;
		(*switch_buf)[0].JB_SP = (unsigned long) stack +
			UM_THREAD_SIZE - sizeof(void *);

		longjmp(*switch_buf, 1);
	}

	return n;
}

static jmp_buf* take_top_jmpbuf(void)
{
	if (!xchg(&top_jmpbuf_ok, 0)) {
		panic("top_jmpbuf not ok");
	}

	return &top_jmpbuf;
}

static enum wsl9x_entry_reason wsl9x_yield(jmp_buf *me, enum wsl9x_result result)
{
	int n = UML_SETJMP(me);

	if (n == 0) {
		resume_jmpbuf = me;

		jmp_buf *top = take_top_jmpbuf();
		UML_LONGJMP(top, result);
	}

	return n;
}

static enum wsl9x_result do_resume(enum wsl9x_entry_reason reason)
{
	top_jmpbuf_ok = 1;
	int n = UML_SETJMP(&top_jmpbuf);

	if (n == 0) {
		jmp_buf* resume = xchg(&resume_jmpbuf, NULL);
		if (!resume) {
			panic("resume_jmpbuf not ok");
		}

		UML_LONGJMP(resume, reason);
	}

	return n;
}

enum wsl9x_result wsl9x_resume(void)
{
	return do_resume(WSL9X_RESUME);
}

enum wsl9x_result wsl9x_page_fault(u32 fault_addr)
{
	struct uml_pt_regs* regs = &current->thread.regs.regs;
	regs->faultinfo.cr2 = fault_addr;
	regs->faultinfo.trap_no = X86_TRAP_PF;
	regs->faultinfo.error_code = VMM_Get_Cur_Thread_Handle()->ClientPtr->Error;
	return do_resume(WSL9X_PAGE_FAULT);
}

enum wsl9x_result wsl9x_syscall(void)
{
	struct uml_pt_regs* regs = &current->thread.regs.regs;
	PT_SYSCALL_NR(regs->gp) = VMM_Get_Cur_Thread_Handle()->ClientPtr->EAX;
	return do_resume(WSL9X_SYSCALL);
}

enum wsl9x_result wsl9x_trap(u8 number)
{
	panic("wsl9x_trap: don't know how to handle trap %d", number);
}

void __noreturn wsl9x_panic(const char* msg)
{
	wsl9x_panic_msg = msg;

	if (xchg(&top_jmpbuf_ok, 0)) {
		longjmp(top_jmpbuf, WSL9X_PANIC);
	}

	// no jmpbuf in panic, nothing to do but crash it hard
	__asm__ volatile ("ud2");
	for (;;) ;
}

void win9x_dump_log(void);
void win9x_dump_log(void)
{
	kmsg_dump(KMSG_DUMP_UNDEF);
}

void os_idle_prepare(void)
{
}

void os_idle_sleep(void) {
	wsl9x_yield(&current->thread.switch_buf, WSL9X_IDLE);
}

static void set_vmm_regs(VMM_Client_Regs* out, const struct uml_pt_regs *regs)
{
	out->EIP 	= regs->gp[HOST_IP];
	out->ESP 	= regs->gp[HOST_SP];
	out->EFlags 	= regs->gp[HOST_EFLAGS];
	out->EAX 	= regs->gp[HOST_AX];
	out->EBX 	= regs->gp[HOST_BX];
	out->ECX 	= regs->gp[HOST_CX];
	out->EDX 	= regs->gp[HOST_DX];
	out->ESI 	= regs->gp[HOST_SI];
	out->EDI 	= regs->gp[HOST_DI];
	out->EBP 	= regs->gp[HOST_BP];
	out->CS 	= regs->gp[HOST_CS];
	out->SS 	= regs->gp[HOST_SS];
	out->DS 	= regs->gp[HOST_DS];
	out->ES 	= regs->gp[HOST_ES];
	out->FS 	= regs->gp[HOST_FS];
	out->GS 	= regs->gp[HOST_GS];
}

static void fetch_vmm_regs(struct uml_pt_regs *out, const VMM_Client_Regs* regs)
{
	out->gp[HOST_IP] 	= regs->EIP;
	out->gp[HOST_SP] 	= regs->ESP;
	out->gp[HOST_EFLAGS] 	= regs->EFlags;
	out->gp[HOST_AX] 	= regs->EAX;
	out->gp[HOST_BX] 	= regs->EBX;
	out->gp[HOST_CX] 	= regs->ECX;
	out->gp[HOST_DX] 	= regs->EDX;
	out->gp[HOST_SI] 	= regs->ESI;
	out->gp[HOST_DI] 	= regs->EDI;
	out->gp[HOST_BP] 	= regs->EBP;
	out->gp[HOST_CS] 	= regs->CS;
	out->gp[HOST_SS] 	= regs->SS;
	out->gp[HOST_DS] 	= regs->DS;
	out->gp[HOST_ES] 	= regs->ES;
	out->gp[HOST_FS] 	= regs->FS;
	out->gp[HOST_GS] 	= regs->GS;
}

void userspace(struct uml_pt_regs *regs)
{
	HTHREAD thread = VMM_Get_Cur_Thread_Handle();

	// Handle any immediate reschedules or signals
	interrupt_end();

	while (1) {
		// sync memory context
		current_mm_sync();

		// restore regs
		set_vmm_regs(thread->ClientPtr, regs);

		// TODO - call VMCPD to restore floating point regs

		if (singlestepping()) {
			thread->ClientPtr->EFlags |= X86_EFLAGS_TF;
		} else {
			thread->ClientPtr->EFlags &= ~X86_EFLAGS_TF;
		}

		// return to userspace
		enum wsl9x_entry_reason reason = wsl9x_yield(&current->thread.switch_buf, WSL9X_USER);

		// save regs
		fetch_vmm_regs(regs, thread->ClientPtr);
		regs->is_user = 1;

		// handle particular reason for returning from userspace
		switch (reason) {
		case WSL9X_START:
			unreachable();
			break;
		case WSL9X_RESUME:
			// what do we do here?
			panic("WSL9X_RESUME after wsl9x_yield");
			break;
		case WSL9X_PAGE_FAULT:
			segv(regs->faultinfo, 0, 1, NULL, NULL);
			break;
		case WSL9X_SYSCALL:
			handle_syscall(regs);
			break;
		case WSL9X_TRAP:
			panic("resuming from WSL9X_TRAP unimplemented");
			break;
		}

		// go around again
		interrupt_end();
	}
}

void new_thread(void *stack, jmp_buf *buf, void (*handler)(void))
{
	(*buf)[0].JB_IP = (unsigned long) handler;
	(*buf)[0].JB_SP = (unsigned long) stack + UM_THREAD_SIZE -
		sizeof(void *);
}

void switch_threads(jmp_buf *me, jmp_buf *you)
{
	wsl9x_yield(me, WSL9X_YIELD);

	if (UML_SETJMP(me) == 0)
		UML_LONGJMP(you, 1);
}

void get_safe_registers(unsigned long *regs, unsigned long *fp_regs)
{
	memset(regs, 0, MAX_REG_OFFSET);
	REGS_CS(regs) = wsl9x_user_code;
	REGS_SS(regs) = wsl9x_user_data;
	REGS_DS(regs) = wsl9x_user_data;
	REGS_ES(regs) = wsl9x_user_data;

	if (fp_regs) {
		memset(fp_regs, 0, host_fp_size);
	}
}

int os_set_thread_area(user_desc_t *info)
{
	VMM_VMCB* vm = VMM_Get_Cur_Thread_Handle()->VMHandle;
	uint16_t sel = info->entry_number << 3;

	union {
		struct desc_struct struc;
		uint32_t words[2];
	} desc;

	fill_ldt(&desc.struc, info);

	VMM_SetDescriptor(sel, vm, desc.words[1], desc.words[0], 0);

	return 0;
}
