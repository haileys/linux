#include <as-layout.h>
#include <asm/current.h>
#include <asm/trapnr.h>
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

void __init wsl9x_init_process(void)
{
	uint64_t code = VMM_BuildDescriptorDWORDs(0, 0xfffff, CODE_TYPE | D_DPL3, D_PAGE32);
	wsl9x_user_code = (uint16_t)VMM_Allocate_GDT_Selector(code >> 32, code, 0);

	uint64_t data = VMM_BuildDescriptorDWORDs(0, 0xfffff, RW_DATA_TYPE | D_DPL3, D_PAGE32);
	wsl9x_user_data = (uint16_t)VMM_Allocate_GDT_Selector(data >> 32, data, 0);
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
	out->EIP = UPT_IP(regs);
	out->ESP = UPT_SP(regs);
	out->EFlags = UPT_EFLAGS(regs);
	out->EAX = UPT_AX(regs);
	out->EBX = UPT_BX(regs);
	out->ECX = UPT_CX(regs);
	out->EDX = UPT_DX(regs);
	out->ESI = UPT_SI(regs);
	out->EDI = UPT_DI(regs);
	out->EBP = UPT_BP(regs);
	out->CS = UPT_CS(regs);
	out->SS = UPT_SS(regs);
	out->DS = UPT_DS(regs);
	out->ES = UPT_ES(regs);

	// TODO - FS and GS for TLS
}

static void fetch_vmm_regs(struct uml_pt_regs *out, const VMM_Client_Regs* regs)
{
	UPT_IP(out) = regs->EIP;
	UPT_SP(out) = regs->ESP;
	UPT_EFLAGS(out) = regs->EFlags;
	UPT_AX(out) = regs->EAX;
	UPT_BX(out) = regs->EBX;
	UPT_CX(out) = regs->ECX;
	UPT_DX(out) = regs->EDX;
	UPT_SI(out) = regs->ESI;
	UPT_DI(out) = regs->EDI;
	UPT_BP(out) = regs->EBP;
	UPT_CS(out) = regs->CS;
	UPT_SS(out) = regs->SS;
	UPT_DS(out) = regs->DS;
	UPT_ES(out) = regs->ES;

	// TODO - FS and GS for TLS
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

int os_set_thread_area(user_desc_t *info, HTHREAD th)
{
	// TODO - TLS not yet implemented
	return ESRCH;
}
