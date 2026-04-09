#include <linux/types.h>
#include <as-layout.h>
#include <init.h>

const char * const elf_aux_platform = "i386";
uint32_t elf_aux_hwcap = 0;

static void init_hwcap(void)
{
	asm volatile ("cpuid" : "=d"(elf_aux_hwcap) : "a"(1) : "%ecx", "%ebx");
}

int __init main(int argc, char **argv, char **envp)
{
	init_hwcap();
	return linux_main(argc, argv, envp);
}
