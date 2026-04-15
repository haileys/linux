#pragma once
#include <wsl9x/prelude.h>
#include <wsl9x/task.h>
#include <wsl9x/mem.h>
#include <wsl9x/time.h>

void __noreturn unimplemented(void);

struct u32divrem {
	uint32_t quo;
	uint32_t rem;
};

static inline struct u32divrem udiv64(uint64_t num, uint32_t denom)
{
	struct u32divrem out;
	__asm__ ("divl %[denom]" : "=a"(out.quo), "=d"(out.rem) : "A"(num), [denom]"r"(denom));
	return out;
}

void WSL9x_Printks(const char* str);
void WSL9x_Printk(const char* str, size_t len);
void WSL9x_Log_Info(const char* str, size_t len);
void WSL9x_Log_Warn(const char* str, size_t len);

void wsl9x_resume(void);
