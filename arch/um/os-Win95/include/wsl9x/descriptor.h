#pragma once
#include "task.h"

extern uint16_t wsl9x_user_code;
extern uint16_t wsl9x_user_data;

/*
 *  Following fields are common to segment and control descriptors
 */
#define D_PRES      0x080       /* present in memory */
#define D_NOTPRES   0       /* not present in memory */

#define D_DPL0      0       /* Ring 0 */
#define D_DPL1      0x020       /* Ring 1 */
#define D_DPL2      0x040       /* Ring 2 */
#define D_DPL3      0x060       /* Ring 3 */

#define D_SEG       0x010       /* Segment descriptor */
#define D_CTRL      0       /* Control descriptor */

#define D_GRAN_BYTE 0x000       /* Segment length is byte granular */
#define D_GRAN_PAGE 0x080       /* Segment length is page granular */
#define D_DEF16     0x000       /* Default operation size is 16 bits */
#define D_DEF32     0x040       /* Default operation size is 32 bits */


/*
 *  Following fields are specific to segment descriptors
 */
#define D_CODE      0x08        /* code */
#define D_DATA      0       /* data */

#define D_X     0       /* if code, exec only */
#define D_RX        0x02        /* if code, readable */
#define D_C     0x04        /* if code, conforming */

#define D_R     0       /* if data, read only */
#define D_W     0x02        /* if data, writable */
#define D_ED        0x04        /* if data, expand down */

#define D_ACCESSED  1       /* segment accessed bit */


/*
 *  Useful combination access rights bytes
 */
#define RW_DATA_TYPE    (D_PRES+D_SEG+D_DATA+D_W)
#define R_DATA_TYPE (D_PRES+D_SEG+D_DATA+D_R)
#define CODE_TYPE   (D_PRES+D_SEG+D_CODE+D_RX)

#define D_PAGE32    (D_GRAN_PAGE+D_DEF32)   /* 32 bit Page granular */

uint32_t VMM_SetDescriptor(uint16_t selector, VMM_VMCB* vm, uint32_t desc_hi, uint32_t desc_lo, uint32_t flags);
uint64_t VMM_Allocate_LDT_Selector(VMM_VMCB* vm, uint32_t desc_hi, uint32_t desc_lo, uint32_t count, uint32_t flags);
uint64_t VMM_Allocate_GDT_Selector(uint32_t desc_hi, uint32_t desc_lo, uint32_t flags);
uint64_t VMM_BuildDescriptorDWORDs(uint32_t base, uint32_t limit, uint8_t type, uint32_t size);
