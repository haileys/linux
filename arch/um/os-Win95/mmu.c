#include "asm/current.h"
#include "linux/mm_types.h"
#include <asm/mmu_context.h>
#include <linux/mm.h>

#include <wsl9x.h>
#include <wsl9x/mem.h>
#include <wsl9x/task.h>
#include <wsl9x/vmm.h>

#include "internal.h"

HTHREAD wsl9x_thread;
HCONTEXT wsl9x_context;

static struct mm_struct* current_mm(void)
{
	struct task_struct* task = current;

	if (task && task->mm) {
		return task->mm;
	}

	return &init_mm;
}

void switch_mm(struct mm_struct *prev, struct mm_struct *next, struct task_struct *tsk)
{
	wsl9x_context = next->context.handle;
	VMM_ContextSwitch(wsl9x_context);
}

void wsl9x__thread_switched(HTHREAD new_thread);
void wsl9x__thread_switched(HTHREAD new_thread)
{
	if (wsl9x_thread == new_thread) {
		VMM_ContextSwitch(wsl9x_context);
	}
}

static __naked __used void thread_switched_asm(void)
{
	__asm__ volatile (
		"pushl %edi\n"
		"call wsl9x__thread_switched\n"
		"add $4, %esp\n"
		"ret\n"
	);
}

void __init wsl9x_init_mmu(void)
{
	wsl9x_thread = VMM_Get_Cur_Thread_Handle();

	wsl9x_context = VMM_ContextCreate();
	if (!wsl9x_context) {
		VMM_Fatal_Error("VMM_ContextCreate failed", 0);
	}

	init_mm.context.handle = wsl9x_context;

	VMM_ContextSwitch(wsl9x_context);
	VMM_Call_When_Thread_Switched(thread_switched_asm);
}

int init_new_context(struct task_struct *task, struct mm_struct *mm)
{
	mm->context.handle = VMM_ContextCreate();
	if (!mm->context.handle) {
		panic("VMM_Context_Create failed");
		return 1;
	}

	// reserve virtual address range for user processes
	u32 user_page_start = WIN9X_PRIVATE_ARENA_START >> PAGE_SHIFT;
	u32 user_page_end = WIN9X_PRIVATE_ARENA_END >> PAGE_SHIFT;
	u32 user_page_count = user_page_end - user_page_start;

	// we must first switch into the new context to do the page reservation
	// make sure to switch back asap
	HCONTEXT prev = VMM_ContextSwitch(mm->context.handle);
	u32 user_addr = VMM_PageReserve(user_page_start, user_page_count, PR_FIXED);
	VMM_ContextSwitch(prev);

	if (user_addr != WIN9X_PRIVATE_ARENA_START) {
		panic("VMM_PageReserve failed for user range: start=%08x count=%08x",
			user_page_start,
			user_page_end);
	}

	return 0;
}

void destroy_context(struct mm_struct *mm)
{
}
