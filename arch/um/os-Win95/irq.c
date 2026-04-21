#include <linux/interrupt.h>
#include "irq.h"

DEFINE_PER_CPU_SHARED_ALIGNED(irq_cpustat_t, irq_stat);

/*
 * irq_chip must define at least enable/disable and ack when
 * the edge handler is used.
 */
static void dummy(struct irq_data *d)
{
}

/* This is used for everything other than the timer. */
static struct irq_chip normal_irq_type = {
	.name = "SIGIO",
	.irq_disable = dummy,
	.irq_enable = dummy,
	.irq_ack = dummy,
	.irq_mask = dummy,
	.irq_unmask = dummy,
	.irq_set_wake = NULL,
};

void __init init_IRQ(void)
{
	for (int i = 0; i < WSL9X_NR_IRQS; i++) {
		irq_set_chip_and_handler(i, &normal_irq_type, handle_edge_irq);
	}
}

int __init arch_probe_nr_irqs(void)
{
	return WSL9X_NR_IRQS;
}

enum wsl9x_result wsl9x_irq(int irq)
{
	irq_enter();
	generic_handle_irq(irq);
	irq_exit();
	return WSL9X_YIELD;
}
