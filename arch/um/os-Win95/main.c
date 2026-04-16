#include <as-layout.h>
#include <linux/types.h>
#include <linux/panic.h>
#include <init.h>
#include <wsl9x.h>
#include <wsl9x/descriptor.h>

const char * const elf_aux_platform = "i386";
uint32_t elf_aux_hwcap = 0;

void wsl9x_resume(void);

static void init_hwcap(void)
{
	asm volatile ("cpuid" : "=d"(elf_aux_hwcap) : "a"(1) : "%ecx", "%ebx");
}

void __init main(int argc, char **argv, char **envp)
{
	if (linux_main(argc, argv, envp)) {
		panic("linux_main error");
	}
}

void __noreturn unimplemented(void)
{
	panic("unimplemented");
	for (;;) ;
}

/* HARNESS FUNCS: */

static int started = 0;

void _start(void);
void _start(void)
{
	// _start serves as a dual entry point for the wsl9x VXD.
	// if the kernel is already started, just resume it until next idle:
	if (started) {
		wsl9x_resume();
		return;
	}

	started = 1;
	init_hwcap();
	wsl9x_allocate_descriptors();

	char* argv[] = {
		"vmlinux",
		"init=/bin/sh",
		"rdinit=/bin/sh",
	};

	char* envp[] = { 0 };

	main(sizeof(argv) / sizeof(*argv), argv, envp);
}
