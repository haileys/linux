#include <as-layout.h>
#include <linux/types.h>
#include <linux/panic.h>
#include <linux/atomic.h>
#include <wsl9x.h>
#include <wsl9x/descriptor.h>
#include <wsl9x/entry.h>
#include "process.h"
#include "irq.h"

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
	wsl9x_init_process();
	return linux_main(start->argc, start->argv, start->envp);
}

void _start(struct wsl9x_entry* entry)
{
	switch (entry->reason) {
	case WSL9X_START:
		entry->result = wsl9x_start(&entry->as.start);
		break;
	case WSL9X_RESUME:
		entry->result = wsl9x_resume();
		break;
	case WSL9X_SYSCALL:
		entry->result = wsl9x_syscall();
		break;
	case WSL9X_PAGE_FAULT:
		entry->result = wsl9x_page_fault(entry->as.page_fault.addr);
		break;
	case WSL9X_TRAP:
		entry->result = wsl9x_trap(entry->as.trap.number);
		break;
	case WSL9X_IRQ:
		entry->result = wsl9x_irq(entry->as.irq);
		break;
	}

	if (entry->result == WSL9X_PANIC) {
		entry->panic_msg = wsl9x_panic_msg;
	}
}
