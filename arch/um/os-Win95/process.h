#pragma once
#include <linux/types.h>
#include <wsl9x/entry.h>

extern const char* wsl9x_panic_msg;

enum wsl9x_result wsl9x_resume(void);
enum wsl9x_result wsl9x_syscall(void);
enum wsl9x_result wsl9x_page_fault(u32 fault_addr);
enum wsl9x_result wsl9x_trap(u8 number);
