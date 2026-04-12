// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2002 - 2007 Jeff Dike (jdike@{addtoit,linux.intel}.com)
 */

#include "linux/compiler.h"
#include <linux/init.h>
#include <linux/sched/mm.h>
#include <linux/sched/task_stack.h>
#include <linux/sched/task.h>
#include <linux/smp-internal.h>

#include <asm/tlbflush.h>

#include <as-layout.h>
#include <kern.h>
#include <os.h>
#include <skas.h>
#include <kern_util.h>

#define __NR_close_range 436

extern void start_kernel(void);

static int __init start_kernel_proc(void *unused)
{
	block_signals_trace();

	start_kernel();
	return 0;
}

#ifndef CONFIG_WIN9X
char cpu_irqstacks[NR_CPUS][THREAD_SIZE] __aligned(THREAD_SIZE);
#endif

int __init start_uml(void)
{
#ifndef CONFIG_WIN9X
	stack_protections((unsigned long) &cpu_irqstacks[0]);
	set_sigstack(cpu_irqstacks[0], THREAD_SIZE);

	init_new_thread_signals();
#endif

	init_task.thread.request.thread.proc = start_kernel_proc;
	init_task.thread.request.thread.arg = NULL;

// #ifdef CONFIG_WIN9X
// 	uml_finishsetup();
// // 	// uml_finishsetup drops straight into userspace
// 	unreachable();
// #else
	return start_idle_thread(task_stack_page(&init_task),
				 &init_task.thread.switch_buf);
// #endif
}

#ifndef CONFIG_WIN9X
unsigned long current_stub_stack(void)
{
	if (current->mm == NULL)
		return 0;

	return current->mm->context.id.stack;
}

struct mm_id *current_mm_id(void)
{
	if (current->mm == NULL)
		return NULL;

	return &current->mm->context.id;
}
#endif

void current_mm_sync(void)
{
	if (current->mm == NULL)
		return;

	um_tlb_sync(current->mm);
}

static DEFINE_SPINLOCK(initial_jmpbuf_spinlock);

void initial_jmpbuf_lock(void)
{
	spin_lock_irq(&initial_jmpbuf_spinlock);
}

void initial_jmpbuf_unlock(void)
{
	spin_unlock_irq(&initial_jmpbuf_spinlock);
}
