#include "sysdep/ptrace.h"
#include <os.h>
#include <string.h>
#include <registers.h>
#include "win9x.h"

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
	unimplemented();
}

void switch_threads(jmp_buf *me, jmp_buf *you)
{
	unimplemented();
}

int map(struct mm_id *mm_idp, unsigned long virt, unsigned long len, int prot,
	int phys_fd, unsigned long long offset)
{
	unimplemented();
}

int unmap(struct mm_id *mm_idp, unsigned long addr, unsigned long len)
{
	unimplemented();
}

void get_safe_registers(unsigned long *regs, unsigned long *fp_regs)
{
	/* TODO get_safe_registers - handle eflags */
	memset(regs, 0, MAX_REG_OFFSET);

	if (fp_regs)
		memset(fp_regs, 0, host_fp_size);
}
