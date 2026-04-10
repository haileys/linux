// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2000 - 2007 Jeff Dike (jdike@{addtoit,linux.intel}.com)
 */

#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched/signal.h>

#include <asm/tlbflush.h>
#include <asm/mmu_context.h>
#include <as-layout.h>
#include <mem_user.h>
#include <os.h>
#include <skas.h>
#include <kern_util.h>
#include <wsl9x/mem.h>

#ifndef CONFIG_WIN9X
struct vm_ops {
	struct mm_id *mm_idp;

	int (*mmap)(struct mm_id *mm_idp,
		    unsigned long virt, unsigned long len, int prot,
		    int phys_fd, unsigned long long offset);
	int (*unmap)(struct mm_id *mm_idp,
		     unsigned long virt, unsigned long len);
};

static int kern_map(struct mm_id *mm_idp,
		    unsigned long virt, unsigned long len, int prot,
		    int phys_fd, unsigned long long offset)
{
	/* TODO: Why is executable needed to be always set in the kernel? */
	return os_map_memory((void *)virt, phys_fd, offset, len,
			     prot & UM_PROT_READ, prot & UM_PROT_WRITE,
			     1);
}

static int kern_unmap(struct mm_id *mm_idp,
		      unsigned long virt, unsigned long len)
{
	return os_unmap_memory((void *)virt, len);
}
#else
struct vm_ops;
#endif

void report_enomem(void)
{
	printk(KERN_ERR "UML ran out of memory on the host side! "
			"This can happen due to a memory limitation or "
			"vm.max_map_count has been reached.\n");
}

static inline int update_pte_range(pmd_t *pmd, unsigned long addr,
				   unsigned long end,
				   struct vm_ops *ops)
{
	pte_t *pte;
	int ret = 0;

	pte = pte_offset_kernel(pmd, addr);
	do {
		if (!pte_needsync(*pte))
			continue;

		unsigned long phys = pte_val(*pte) & PAGE_MASK;

#ifdef CONFIG_WIN9X
		uint32_t pagenum = addr >> PAGE_SHIFT;
		uint32_t physnum = phys >> PAGE_SHIFT;
		uint32_t flags = PC_LOCKED;

		if (pte_present(*pte) && !pte_get_bits(*pte, _PAGE_PROTNONE)) {
			flags |= PC_PRESENT;
		}
		if (pte_get_bits(*pte, _PAGE_USER)) {
			flags |= PC_USER;
		}
		if (pte_get_bits(*pte, _PAGE_RW)) {
			flags |= PC_WRITEABLE;
		}

		if (!pte_young(*pte)) {
			flags &= ~PC_PRESENT;
		} else if (!pte_dirty(*pte)) {
			flags &= ~PC_WRITEABLE;
		}

		if (flags & PC_PRESENT) {
			VMM_PageCommitPhys(pagenum, 1, physnum, flags);
		} else {
			VMM_PageDecommit(pagenum, 1, 0);
		}
#else
		if (pte_present(*pte)) {
			__u64 offset;
			int fd = phys_mapping(phys, &offset);
			int r, w, x, prot;

			r = pte_read(*pte);
			w = pte_write(*pte);
			x = pte_exec(*pte);
			if (!pte_young(*pte)) {
				r = 0;
				w = 0;
			} else if (!pte_dirty(*pte))
				w = 0;

			prot = (r ? UM_PROT_READ : 0) |
			       (w ? UM_PROT_WRITE : 0) |
			       (x ? UM_PROT_EXEC : 0);

			ret = ops->mmap(ops->mm_idp, addr, PAGE_SIZE,
					prot, fd, offset);
		} else {
			ret = ops->unmap(ops->mm_idp, addr, PAGE_SIZE);
		}
#endif

		*pte = pte_mkuptodate(*pte);
	} while (pte++, addr += PAGE_SIZE, ((addr < end) && !ret));
	return ret;
}

static inline int update_pmd_range(pud_t *pud, unsigned long addr,
				   unsigned long end,
				   struct vm_ops *ops)
{
	pmd_t *pmd;
	unsigned long next;
	int ret = 0;

	pmd = pmd_offset(pud, addr);
	do {
		next = pmd_addr_end(addr, end);
		if (!pmd_present(*pmd)) {
			if (pmd_needsync(*pmd)) {
#ifdef CONFIG_WIN9X
				uint32_t pagenum = addr >> PAGE_SHIFT;
				VMM_PageDecommit(pagenum, 1, 0);
#else
				ret = ops->unmap(ops->mm_idp, addr,
						 next - addr);
#endif
				pmd_mkuptodate(*pmd);
			}
		}
		else ret = update_pte_range(pmd, addr, next, ops);
	} while (pmd++, addr = next, ((addr < end) && !ret));
	return ret;
}

static inline int update_pud_range(p4d_t *p4d, unsigned long addr,
				   unsigned long end,
				   struct vm_ops *ops)
{
	pud_t *pud;
	unsigned long next;
	int ret = 0;

	pud = pud_offset(p4d, addr);
	do {
		next = pud_addr_end(addr, end);
		if (!pud_present(*pud)) {
			if (pud_needsync(*pud)) {
#ifndef CONFIG_WIN9X
				ret = ops->unmap(ops->mm_idp, addr,
						 next - addr);
#endif
				pud_mkuptodate(*pud);
			}
		}
		else ret = update_pmd_range(pud, addr, next, ops);
	} while (pud++, addr = next, ((addr < end) && !ret));
	return ret;
}

static inline int update_p4d_range(pgd_t *pgd, unsigned long addr,
				   unsigned long end,
				   struct vm_ops *ops)
{
	p4d_t *p4d;
	unsigned long next;
	int ret = 0;

	p4d = p4d_offset(pgd, addr);
	do {
		next = p4d_addr_end(addr, end);
		if (!p4d_present(*p4d)) {
			if (p4d_needsync(*p4d)) {
#ifndef CONFIG_WIN9X
				ret = ops->unmap(ops->mm_idp, addr,
						 next - addr);
#endif
				p4d_mkuptodate(*p4d);
			}
		} else
			ret = update_pud_range(p4d, addr, next, ops);
	} while (p4d++, addr = next, ((addr < end) && !ret));
	return ret;
}

int um_tlb_sync(struct mm_struct *mm)
{
	pgd_t *pgd;
	unsigned long addr, next;
	int ret = 0;

	guard(spinlock_irqsave)(&mm->context.sync_tlb_lock);

	if (mm->context.sync_tlb_range_to == 0)
		return 0;

#ifndef CONFIG_WIN9X
	struct vm_ops ops;
	ops.mm_idp = &mm->context.id;
	if (mm == &init_mm) {
		ops.mmap = kern_map;
		ops.unmap = kern_unmap;
	} else {
		ops.mmap = map;
		ops.unmap = unmap;
	}

	struct vm_ops* ops_ptr = &ops;
#else
	struct vm_ops* ops_ptr = NULL;
#endif

	addr = mm->context.sync_tlb_range_from;
	pgd = pgd_offset(mm, addr);
	do {
		next = pgd_addr_end(addr, mm->context.sync_tlb_range_to);
		if (!pgd_present(*pgd)) {
			if (pgd_needsync(*pgd)) {
#ifndef CONFIG_WIN9X
				ret = ops.unmap(ops.mm_idp, addr,
						next - addr);
				pgd_mkuptodate(*pgd);
#endif
			}
		} else
			ret = update_p4d_range(pgd, addr, next, ops_ptr);
	} while (pgd++, addr = next,
		 ((addr < mm->context.sync_tlb_range_to) && !ret));

	if (ret == -ENOMEM)
		report_enomem();

	mm->context.sync_tlb_range_from = 0;
	mm->context.sync_tlb_range_to = 0;

	return ret;
}

void flush_tlb_all(void)
{
	/*
	 * Don't bother flushing if this address space is about to be
	 * destroyed.
	 */
	if (atomic_read(&current->mm->mm_users) == 0)
		return;

	flush_tlb_mm(current->mm);
}

void flush_tlb_mm(struct mm_struct *mm)
{
	struct vm_area_struct *vma;
	VMA_ITERATOR(vmi, mm, 0);

	for_each_vma(vmi, vma)
		um_tlb_mark_sync(mm, vma->vm_start, vma->vm_end);
}
