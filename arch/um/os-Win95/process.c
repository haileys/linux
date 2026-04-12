#include <as-layout.h>
#include <asm/current.h>
#include <generated/asm-offsets.h>
#include <kern_util.h>
#include <linux/kmsg_dump.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/string.h>
#include <os.h>
#include <registers.h>
#include <sysdep/ptrace.h>
#include <wsl9x.h>

#define INIT_JMP_NEW_THREAD 0
#define INIT_JMP_CALLBACK 1
#define INIT_JMP_HALT 2
#define INIT_JMP_REBOOT 3
#define INIT_JMP_RETURN 4

static jmp_buf initial_jmpbuf;
static int initial_jmpbuf_ok = 0;

int start_idle_thread(void *stack, jmp_buf *switch_buf)
{
	initial_jmpbuf_ok = 1;
	int n = setjmp(initial_jmpbuf);

	if (n == 0) {
		(*switch_buf)[0].JB_IP = (unsigned long) uml_finishsetup;
		(*switch_buf)[0].JB_SP = (unsigned long) stack +
			UM_THREAD_SIZE - sizeof(void *);
	}

	longjmp(*switch_buf, 1);
	return 0;
	// (*switch_buf)[0].JB_IP = (unsigned long) uml_finishsetup;
	// (*switch_buf)[0].JB_SP = (unsigned long) stack +
	// 	UM_THREAD_SIZE - sizeof(void *);

	// initial_jmpbuf_ok = 1;
	// switch_threads(&initial_jmpbuf, switch_buf);
	// return 0;
}

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
	initial_jmpbuf_ok = 1;
	switch_threads(&initial_jmpbuf, &cur->thread.switch_buf);
}

void os_idle_prepare(void)
{
}

void os_idle_sleep(void)
{
	struct task_struct* cur = current;
	if (initial_jmpbuf_ok) {
		initial_jmpbuf_ok = 0;
		switch_threads(&cur->thread.switch_buf, &initial_jmpbuf);
	}
}

int start_userspace(struct mm_id *mm_id)
{
	// I don't think we have to do anything here yet.
	return 0;
}

void userspace(struct uml_pt_regs *regs)
{
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
	asm ("movl %%cs, %0" : "=r"(REGS_CS(regs)));
	asm ("movl %%ss, %0" : "=r"(REGS_SS(regs)));
	asm ("movl %%ds, %0" : "=r"(REGS_DS(regs)));
	asm ("movl %%es, %0" : "=r"(REGS_ES(regs)));

	if (fp_regs) {
		memset(fp_regs, 0, host_fp_size);
	}
}
