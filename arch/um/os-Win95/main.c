#include <as-layout.h>
#include <linux/types.h>
#include <linux/panic.h>
#include <linux/atomic.h>
#include <wsl9x.h>
#include <wsl9x/descriptor.h>
#include <wsl9x/entry.h>

static int started = 0;

const char * const elf_aux_platform = "i386";
uint32_t elf_aux_hwcap = 0;

static void init_hwcap(void)
{
	asm volatile ("cpuid" : "=d"(elf_aux_hwcap) : "a"(1) : "%ecx", "%ebx");
}

void __noreturn unimplemented(void)
{
	panic("unimplemented");
	for (;;) ;
}

static enum wsl9x_result wsl9x_start(struct wsl9x_start_param* start)
{
	if (xchg(&started, 1)) {
		panic("wsl9x already started");
	}

	init_hwcap();
	wsl9x_allocate_descriptors();
	return linux_main(start->argc, start->argv, start->envp);
}

void _start(struct wsl9x_entry* entry)
{
	switch (entry->reason) {
	case WSL9X_START:
		entry->result = wsl9x_start(&entry->as.start);
		return;
	case WSL9X_RESUME:
		entry->result = wsl9x_resume();
		return;
	}
}
