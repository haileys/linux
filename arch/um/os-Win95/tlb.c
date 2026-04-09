#include <asm/tlbflush.h>
#include "win9x.h"

int um_tlb_sync(struct mm_struct *mm)
{
	unimplemented();
}

void flush_tlb_all(void)
{
	unimplemented();
}

void flush_tlb_mm(struct mm_struct *mm)
{
	unimplemented();
}
