#pragma once
#include "prelude.h"
#include <linux/compiler_attributes.h>

enum Fatal_Error_Flags {
	EF_Hang_On_Exit = 0x01
};

void __noreturn VMM_Fatal_Error(const char* msg, enum Fatal_Error_Flags flags);


struct VMM_Machine_Info {
	u8 msdos_ver_major;
	u8 msdos_ver_minor;
	u8 msdos_oem_serial;
	u8 machine_model;
	u16 machine_type_flags;
	u32 sys_config_params;
	u32 equipment_flags;
};

// machine type flags
#define GMIF_80486 (1 << 0)
#define GMIF_PCXT  (1 << 1)
#define GMIF_MCA   (1 << 2)
#define GMIF_EISA  (1 << 3)
#define GMIF_CPUID (1 << 4)

void VMM_Get_Machine_Info(struct VMM_Machine_Info* info);
