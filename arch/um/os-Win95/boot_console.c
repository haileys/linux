// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/console.h>
#include <os.h>

static void wsl9x_boot_console_write(struct console *console, const char *string,
				 unsigned len)
{
	WSL9x_Printk(string, len);
}

static struct console wsl9x_boot_console = {
	.name		= "bootcon",
	.write		= wsl9x_boot_console_write,
	.flags		= CON_BOOT,
};

static int __init wsl9x_boot_console_init(void)
{
	register_console(&wsl9x_boot_console);
	return 0;
}
console_initcall(wsl9x_boot_console_init);
