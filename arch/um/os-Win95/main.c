#include <as-layout.h>
#include <init.h>

const char * const elf_aux_platform = "i386";
uint32_t elf_aux_hwcap = 0;

void wsl9x_resume(void);

static void init_hwcap(void)
{
	asm volatile ("cpuid" : "=d"(elf_aux_hwcap) : "a"(1) : "%ecx", "%ebx");
}

int __init main(int argc, char **argv, char **envp)
{
	init_hwcap();

	if (linux_main(argc, argv, envp)) {
		return 1;
	}

	while (1) {
		wsl9x_resume();
	}

	return 0;
}

/* HARNESS FUNCS: */

void _start(void);
void _start(void)
{
	char* argv[] = {
		"vmlinux",
		"init=/bin/sh",
		"rdinit=/bin/sh",
	};
	char* envp[] = { 0 };
	int rc = main(sizeof(argv) / sizeof(*argv), argv, envp);
	asm volatile ("int $0x80" :: "a"(1 /* NR_exit */), "b"(rc));
}
