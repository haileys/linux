#include <linux/console.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/irq.h>
#include <hvc_console.h>
#include <wsl9x.h>

static struct hvc_struct *wsl9x_console_dev;

static const struct hv_ops wsl9x_console_ops = {
	.get_chars = WSL9X_Console_Get,
	.put_chars = WSL9X_Console_Put,
	.notifier_add = notifier_add_irq,
	.notifier_del = notifier_del_irq,
	.notifier_hangup = notifier_hangup_irq,
};

static int __init wsl9x_console_init(void)
{
	wsl9x_console_dev = hvc_alloc(0, WSL9X_IRQ_CONSOLE, &wsl9x_console_ops, 16);
	BUG_ON(!wsl9x_console_dev);

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
