#pragma once
#include "prelude.h"
#include <linux/compiler_attributes.h>

enum Fatal_Error_Flags {
	EF_Hang_On_Exit = 0x01
};

void __noreturn VMM_Fatal_Error(const char* msg, enum Fatal_Error_Flags flags);
