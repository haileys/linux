#pragma once
#include <wsl9x/prelude.h>
#include <wsl9x/thread.h>
#include <wsl9x/mem.h>

static inline __noreturn void unimplemented(void)
{
	__asm__ volatile ("int3" ::: "memory");
	__asm__ volatile ("ud2" ::: "memory");
	for (;;) ;
}

void WSL9x_Printks(const char* str);
void WSL9x_Printk(const char* str, size_t len);
void WSL9x_Log_Info(const char* str, size_t len);
void WSL9x_Log_Warn(const char* str, size_t len);
