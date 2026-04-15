#include <asm/mmu_context.h>
#include <linux/mm.h>
#include <wsl9x.h>
#include <wsl9x/mem.h>

int init_new_context(struct task_struct *task, struct mm_struct *mm)
{
	HCONTEXT handle = VMM_ContextCreate();
	if (!handle) {
		panic("VMM_ContextCreate failed");
		return -ENOMEM;
	}

	mm->context.handle = handle;
	return 0;
}

void destroy_context(struct mm_struct *mm)
{
	unimplemented();
}
