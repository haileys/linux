#include <asm/cpufeature.h>
#include <asm/cpufeatures.h>
#include <linux/seq_file.h>
#include <shared/os.h>
#include <wsl9x/vmm.h>
#include "internal.h"

char * elf_aux_platform = "i386";
long elf_aux_hwcap = 0;

static int show_cpuinfo(struct seq_file *m, void *v)
{
	int i = 0;

#if IS_ENABLED(CONFIG_SMP)
	i = (uintptr_t) v - 1;
	if (!cpu_online(i))
		return 0;
#endif

	seq_printf(m, "processor\t: %d\n", i);
	seq_printf(m, "vendor_id\t: WSL9x\n");
	seq_printf(m, "model name\t: WSL9x\n");
	seq_printf(m, "mode\t\t: skas\n");
	// seq_printf(m, "host\t\t: %s\n", host_info);
	seq_printf(m, "fpu\t\t: %s\n", str_yes_no(cpu_has(&boot_cpu_data, X86_FEATURE_FPU)));
	seq_printf(m, "flags\t\t:");
	for (i = 0; i < 32*NCAPINTS; i++)
		if (cpu_has(&boot_cpu_data, i) && (x86_cap_flags[i] != NULL))
			seq_printf(m, " %s", x86_cap_flags[i]);
	seq_printf(m, "\n");
	seq_printf(m, "cache_alignment\t: %d\n", boot_cpu_data.cache_alignment);
	// seq_printf(m, "bogomips\t: %lu.%02lu\n",
	// 	   loops_per_jiffy/(500000/HZ),
	// 	   (loops_per_jiffy/(5000/HZ)) % 100);

	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	if (*pos < nr_cpu_ids)
		return (void *)(uintptr_t)(*pos + 1);
	return NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return c_start(m, pos);
}

static void c_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= show_cpuinfo,
};

void setup_machinename(char *machine_out)
{
	/* TODO return actual arch level here */
	sized_strscpy(machine_out, "i386", 64);
}

void __init wsl9x_init_cpu(void)
{
	struct VMM_Machine_Info info;
	VMM_Get_Machine_Info(&info);

	if (info.machine_type_flags & GMIF_80486) {
		elf_aux_platform[1] = '4';
	}

	if (info.machine_type_flags & GMIF_CPUID) {
		asm volatile ("cpuid" : "=d"(elf_aux_hwcap) : "a"(1) : "%ecx", "%ebx");
	}
}
