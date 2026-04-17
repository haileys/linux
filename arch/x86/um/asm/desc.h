/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __UM_DESC_H
#define __UM_DESC_H

#include <asm/desc_defs.h>
#include <uapi/asm/ldt.h>

/* Taken from asm-i386/desc.h, it's the only thing we need. The rest wouldn't
 * compile, and has never been used. */
#define LDT_empty(info) (\
	(info)->base_addr	== 0	&& \
	(info)->limit		== 0	&& \
	(info)->contents	== 0	&& \
	(info)->read_exec_only	== 1	&& \
	(info)->seg_32bit	== 0	&& \
	(info)->limit_in_pages	== 0	&& \
	(info)->seg_not_present	== 1	&& \
	(info)->useable		== 0	)

#ifdef CONFIG_WIN9X

// taken from arch/x86/include/asm/desc.h
static inline void fill_ldt(struct desc_struct *desc, const struct user_desc *info)
{
	desc->limit0		= info->limit & 0x0ffff;

	desc->base0		= (info->base_addr & 0x0000ffff);
	desc->base1		= (info->base_addr & 0x00ff0000) >> 16;

	desc->type		= (info->read_exec_only ^ 1) << 1;
	desc->type	       |= info->contents << 2;
	/* Set the ACCESS bit so it can be mapped RO */
	desc->type	       |= 1;

	desc->s			= 1;
	desc->dpl		= 0x3;
	desc->p			= info->seg_not_present ^ 1;
	desc->limit1		= (info->limit & 0xf0000) >> 16;
	desc->avl		= info->useable;
	desc->d			= info->seg_32bit;
	desc->g			= info->limit_in_pages;

	desc->base2		= (info->base_addr & 0xff000000) >> 24;
	/*
	 * Don't allow setting of the lm bit. It would confuse
	 * user_64bit_mode and would get overridden by sysret anyway.
	 */
	desc->l			= 0;
}

#endif

#endif
