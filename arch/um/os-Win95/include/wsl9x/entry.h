#pragma once
#include "prelude.h"
#include "task.h"

enum wsl9x_entry_reason {
	WSL9X_START = 1,
	WSL9X_RESUME = 2,
	WSL9X_SYSCALL = 4,
	WSL9X_PAGE_FAULT = 3,
	WSL9X_TRAP = 5,
	WSL9X_IRQ = 6,
};

enum wsl9x_result {
	// Kernel is yielding but has more work to do. Must resume again.
	WSL9X_YIELD = 1,
	// Kernel is idle.
	WSL9X_IDLE = 2,
	// Returning to userspace
	WSL9X_USER = 3,
	// Kernel has panicked.
	WSL9X_PANIC = 4,
};

enum wsl9x_irq {
	WSL9X_IRQ_CONSOLE,
};

struct wsl9x_services {
	ssize_t(*console_put)(u32 vtermno, const u8* buf, size_t len);
	ssize_t(*console_get)(u32 vtermno, u8* buf, size_t len);
};

struct wsl9x_start_param {
	const struct wsl9x_services* services;
	int argc;
	char** argv;
	char** envp;
};

struct wsl9x_page_fault {
	u32 addr;
};

struct wsl9x_trap {
	u8 number;
};

struct wsl9x_entry {
	enum wsl9x_entry_reason reason;
	enum wsl9x_result result;

	// set if result == WSL9X_PANIC:
	const char* panic_msg;

	// carries params according to reason field:
	union {
		struct wsl9x_start_param start;
		struct wsl9x_page_fault page_fault;
		struct wsl9x_trap trap;
		enum wsl9x_irq irq;
	} as;
};

typedef void(*wsl9x_entry_func_t)(struct wsl9x_entry* entry);

void _start(struct wsl9x_entry* entry);

