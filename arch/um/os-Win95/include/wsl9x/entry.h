#pragma once

enum wsl9x_entry_reason {
	// Initial start of kernel
	WSL9X_START = 1,
	// Resuming kernel
	WSL9X_RESUME = 2,
};

enum wsl9x_result {
	// Kernel is yielding but has more work to do. Must resume again.
	WSL9X_YIELD = 1,
	// Kernel is idle.
	WSL9X_IDLE = 2,
	// Kernel has panicked.
	WSL9X_PANIC = 3,
};

struct wsl9x_start_param {
	int argc;
	char** argv;
	char** envp;
};

struct wsl9x_entry {
	enum wsl9x_entry_reason reason;
	enum wsl9x_result result;
	union {
		struct wsl9x_start_param start;
	} as;
};

typedef void(*wsl9x_entry_func_t)(struct wsl9x_entry* entry);

void _start(struct wsl9x_entry* entry);

