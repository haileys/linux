/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2002 - 2007 Jeff Dike (jdike@{addtoit,linux.intel}.com)
 */

#ifndef __ARCH_UM_MMU_H
#define __ARCH_UM_MMU_H

#include "linux/types.h"
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <mm_id.h>

#ifdef CONFIG_WIN9X
#include <wsl9x/mem.h>
#endif

typedef struct mm_context {
#ifndef CONFIG_WIN9X
	struct mm_id id;
	struct mutex turnstile;

	struct list_head list;
#endif
	/* Address range in need of a TLB sync */
	spinlock_t sync_tlb_lock;
	unsigned long sync_tlb_range_from;
	unsigned long sync_tlb_range_to;
} mm_context_t;

#ifdef CONFIG_WIN9X
#  define INIT_MM_TURNSTILE
#else
#  define INIT_MM_TURNSTILE .turnstile = __MUTEX_INITIALIZER(mm.context.turnstile),
#endif

#define INIT_MM_CONTEXT(mm)						\
	.context = {							\
		INIT_MM_TURNSTILE	\
		.sync_tlb_lock = __SPIN_LOCK_INITIALIZER(mm.context.sync_tlb_lock), \
	}

#endif
