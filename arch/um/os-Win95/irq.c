#include <linux/interrupt.h>
#include "irq.h"

DEFINE_PER_CPU_SHARED_ALIGNED(irq_cpustat_t, irq_stat);

void __init init_IRQ(void)
{
	// irq_set_chip_and_handler(TIMER_IRQ, &alarm_irq_type, handle_percpu_irq);
}

int __init arch_probe_nr_irqs(void)
{
	return NR_IRQS;
}

enum wsl9x_result wsl9x_irq(int irq)
{
	irq_enter();
	generic_handle_irq(irq);
	irq_exit();
	return WSL9X_YIELD;
}
