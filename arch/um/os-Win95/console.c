#include <linux/interrupt.h>
#include <linux/console.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/irq.h>
#include <hvc_console.h>
#include <wsl9x.h>

static const struct hv_ops wsl9x_console_ops = {
	.get_chars = WSL9X_Console_Get,
	.put_chars = WSL9X_Console_Put,
	.notifier_add = notifier_add_irq,
	.notifier_del = notifier_del_irq,
	.notifier_hangup = notifier_hangup_irq,
};

static int __init wsl9x_console_init(void)
{
	for (int idx = 0; idx < 4; idx++) {
		struct hvc_struct* con = hvc_alloc(idx, WSL9X_IRQ_CONSOLE, &wsl9x_console_ops, 128);
		BUG_ON(IS_ERR(con));
		con->flags |= IRQF_SHARED;
	}

	return 0;
}
device_initcall(wsl9x_console_init);

static int __init wsl9x_console_early_init(void)
{
	hvc_instantiate(0, 0, &wsl9x_console_ops);
	add_preferred_console("hvc", 0, NULL);

	return 0;
}
console_initcall(wsl9x_console_early_init);
