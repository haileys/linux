#pragma once
#include "prelude.h"

typedef struct VirtualMemory* HMEM;
#define HMEM_FAIL ((HMEM)(-1))

uint32_t VMM_PageReserve(uint32_t virt_pfn, uint32_t npages, uint32_t flags);

/* PageReserve arena values */
#define PR_PRIVATE  0x80000400  /* anywhere in private arena */
#define PR_SHARED   0x80060000  /* anywhere in shared arena */
#define PR_SYSTEM   0x80080000  /* anywhere in system arena */
/* PageReserve flags */
#define PR_FIXED    0x00000008  /* don't move during PageReAllocate */
#define PR_4MEG     0x00000001  /* allocate on 4mb boundary */
#define PR_STATIC   0x00000010  /* see PageReserve documentation */

uint32_t VMM_PageCommit(uint32_t pagenum, uint32_t npages, uint32_t pager, uint32_t pagerdata, uint32_t flags);

uint32_t VMM_PageCommitPhys(uint32_t pagenum, uint32_t npages, uint32_t phys, uint32_t flags);

/* PageCommit default pager handle values */
#define PD_ZEROINIT 0x00000001  /* swappable zero-initialized pages */
#define PD_NOINIT   0x00000002  /* swappable uninitialized pages */
#define PD_FIXEDZERO    0x00000003  /* fixed zero-initialized pages */
#define PD_FIXED    0x00000004  /* fixed uninitialized pages */

/* PageCommit flags */
#define PC_FIXED    0x00000008  /* pages are permanently locked */
#define PC_LOCKED   0x00000080  /* pages are made present and locked*/
#define PC_LOCKEDIFDP   0x00000100  /* pages are locked if swap via DOS */
#define PC_WRITEABLE    0x00020000  /* make the pages writeable */
#define PC_USER     0x00040000  /* make the pages ring 3 accessible */
#define PC_INCR     0x40000000  /* increment "pagerdata" each page */
#define PC_PRESENT  0x80000000  /* make pages initially present */
#define PC_STATIC   0x20000000  /* allow commit in PR_STATIC object */
#define PC_DIRTY    0x08000000  /* make pages initially dirty */

/* PageCommitContig additional flags */
#define PCC_ZEROINIT    0x00000001  /* zero-initialize new pages */
#define PCC_NOLIN   0x10000000  /* don't map to any linear address */

uint32_t VMM_PageDecommit(uint32_t virt_pfn, uint32_t npages, uint32_t flags);

uint32_t VMM_PageFree(HMEM mem, uint32_t flags);

uint32_t VMM_CopyPageTable(uint32_t pagenum, uint32_t npages, uint32_t* buffer, uint32_t flags);

uint32_t VMM_PageModifyPermissions(uint32_t pagenum, uint32_t npages, uint32_t perm_and, uint32_t perm_or);

uint32_t VMM_GetFreePageCount(uint32_t flags);

typedef struct MemoryContext* HCONTEXT;
HCONTEXT VMM_ContextCreate(void);
uint32_t VMM_ContextDestroy(HCONTEXT);
HCONTEXT VMM_ContextSwitch(HCONTEXT);
HCONTEXT VMM_GetCurrentContext(void);
