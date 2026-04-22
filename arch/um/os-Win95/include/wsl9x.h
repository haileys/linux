#pragma once
#include <wsl9x/prelude.h>
#include <wsl9x/task.h>
#include <wsl9x/mem.h>
#include <wsl9x/time.h>
#include <wsl9x/entry.h>

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

#define WSL9X__GET_VERSION 0x0000
u32 WSL9X_Get_Version(void);

#define WSL9X__DEBUG_PRINT 0x0001
void WSL9X_Debug_Print(const char* str, size_t len);

#define WSL9X__DEBUG_PRINTF 0x0002
void WSL9X_Debug_Printf(const char* str, ...);

#define WSL9X__LOG_INFO 0x0003
void WSL9X_Log_Info(const char* str, size_t len);

#define WSL9X__LOG_WARN 0x0004
void WSL9X_Log_Warn(const char* str, size_t len);

#define WSL9X__CONSOLE_PUT 0x0005
ssize_t WSL9X_Console_Put(u32 vtermno, const u8* buf, size_t len);

#define WSL9X__CONSOLE_GET 0x0006
ssize_t WSL9X_Console_Get(u32 vtermno, u8* buf, size_t len);
