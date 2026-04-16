#include <as-layout.h>
#include <asm/current.h>
#include <generated/asm-offsets.h>
#include <kern_util.h>
#include <linux/kmsg_dump.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <linux/panic.h>
#include <os.h>
#include <registers.h>
#include <sysdep/tls.h>
#include <sysdep/ptrace.h>
#include <wsl9x.h>
#include <wsl9x/task.h>
#include <wsl9x/descriptor.h>
#include <uapi/linux/errno.h>

#define INIT_JMP_NEW_THREAD 0
#define INIT_JMP_CALLBACK 1
#define INIT_JMP_HALT 2
#define INIT_JMP_REBOOT 3
#define INIT_JMP_RETURN 4

static jmp_buf top_jmpbuf;
static int top_jmpbuf_ok = 0;

uint16_t wsl9x_user_code;
uint16_t wsl9x_user_data;

void __init wsl9x_allocate_descriptors(void)
{
	// uint64_t code = VMM_BuildDescriptorDWORDs(0, 0xfffff, CODE_TYPE | D_DPL3, D_PAGE32);
	// wsl9x_user_code = (uint16_t)VMM_Allocate_GDT_Selector(code >> 32, code, 0);

	// uint64_t data = VMM_BuildDescriptorDWORDs(0, 0xfffff, RW_DATA_TYPE | D_DPL3, D_PAGE32);
	// wsl9x_user_data = (uint16_t)VMM_Allocate_GDT_Selector(data >> 32, data, 0);
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

	return 0;
	// (*switch_buf)[0].JB_IP = (unsigned long) uml_finishsetup;
	// (*switch_buf)[0].JB_SP = (unsigned long) stack +
	// 	UM_THREAD_SIZE - sizeof(void *);

	// initial_jmpbuf_ok = 1;
	// switch_threads(&initial_jmpbuf, switch_buf);
	// return 0;
}

void win9x_dump_log(void);
void win9x_dump_log(void)
{
	kmsg_dump(KMSG_DUMP_UNDEF);
}

void wsl9x_resume(void)
{
	unimplemented();
	schedule();
	interrupt_end();
	current_mm_sync();
	struct task_struct* cur = current;

	top_jmpbuf_ok = 1;
	switch_threads(&top_jmpbuf, &cur->thread.switch_buf);
}

void os_idle_prepare(void)
{
}

void os_idle_sleep(void)
{
	if (xchg(&top_jmpbuf_ok, 0)) {
		switch_threads(&current->thread.switch_buf, &top_jmpbuf);
	} else {
		panic("top_jmpbuf not ok");
	}
}

struct thread_init_data {
	// struct task_struct* task;
	struct completion* done;
};

static __used void thread_init(struct thread_init_data* data)
{
	panic("thread_init");
	complete(data->done);
}

static __naked void thread_init_trampoline(void)
{
	__asm__ volatile (
		"pushl %edx\n"
		"call thread_init\n"
		"addl 4, %esp\n"
		"ret\n"
		// :: "s"(thread_init)
	);
}

static int wsl9x_thread_type[0];

static HTHREAD start_userspace_thread(struct uml_pt_regs *regs)
{
	DECLARE_COMPLETION_ONSTACK(completion);

	// // assert this to make myself feel better. why is regs passed in here anyway?
	// if (&current->thread.regs.regs == regs) {
	// 	panic("called with regs of not-current task");
	// }

	struct thread_init_data init_data = {0};
	init_data.done = &completion;
	// init_data.task = current;

	// HTHREAD thread = VMM_VMMCreateThread(
	// 	UPT_SS(regs),
	// 	UPT_SP(regs),
	// 	UPT_CS(regs),
	// 	UPT_IP(regs),
	// 	UPT_DS(regs),
	// 	UPT_ES(regs),
	// 	(u32)&wsl9x_thread_type,
	// 	thread_init_trampoline,
	// 	&init_data);

	wait_for_completion(init_data.done);

	return 0;
}

void userspace(struct uml_pt_regs *init_regs)
{
	start_userspace_thread(init_regs);
	unimplemented();
}

void new_thread(void *stack, jmp_buf *buf, void (*handler)(void))
{
	(*buf)[0].JB_IP = (unsigned long) handler;
	(*buf)[0].JB_SP = (unsigned long) stack + UM_THREAD_SIZE -
		sizeof(void *);
}

void switch_threads(jmp_buf *me, jmp_buf *you)
{
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
