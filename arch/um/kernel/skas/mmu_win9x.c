#include <asm/mmu_context.h>
#include <linux/mm.h>
#include <wsl9x.h>

int init_new_context(struct task_struct *task, struct mm_struct *mm)
{
	unimplemented();
}

void destroy_context(struct mm_struct *mm)
{
	unimplemented();
}
