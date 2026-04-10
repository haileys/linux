#include <os.h>
#include <registers.h>
#include <sysdep/ptrace.h>
#include <generated/asm-offsets.h>
#include <linux/string.h>
#include <wsl9x.h>

int start_idle_thread(void *stack, jmp_buf *switch_buf)
{
	unimplemented();
}

int start_userspace(struct mm_id *mm_id)
{
	unimplemented();
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
