#include <linux/kernel_stat.h>
#include <kern_util.h>
#include <as-layout.h>
#include <os.h>

DEFINE_PER_CPU_SHARED_ALIGNED(irq_cpustat_t, irq_stat);

void __init init_IRQ(void)
{
}

int __init arch_probe_nr_irqs(void)
{
	return NR_IRQS;
}

/*
 * do_IRQ handles all normal device IRQs (the special
 * SMP cross-CPU interrupts have their own specific
 * handlers).
 */
unsigned int do_IRQ(int irq, struct uml_pt_regs *regs)
{
	abort();
	return 1;
}

void um_irqs_resume(void)
{
	abort();
}

void um_irqs_suspend(void)
{
	abort();
}
