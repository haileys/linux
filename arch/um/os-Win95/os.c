#include <init.h>
#include <os.h>
#include <linux/string.h>
#include <wsl9x.h>

/* util */

static uint8_t get_random_byte(void)
{
	// simple LCG, it doesn't have to be good
	static uint32_t seed = 0x1ebec837;
	const uint32_t m = 0xffffffff;
	const uint32_t a = 1103515245;
	const uint32_t c = 12345;

	seed = (a * seed + c) % m;
	return (uint8_t)seed;
}

ssize_t os_getrandom(void *buf, size_t len, unsigned int flags)
{
	uint8_t *out = buf;

	for (size_t i = 0; i < len; i++) {
		out[i] = get_random_byte();
	}

	return len;
}

/* logging */

/*
 * The os_info/os_warn functions will be called by helper threads. These
 * have a very limited stack size and using the libc formatting functions
 * may overflow the stack.
 * So pull in the kernel vscnprintf and use that instead with a fixed
 * on-stack buffer.
 */
int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);

void os_info(const char *fmt, ...)
{
	char buf[256];
	va_list list;
	int len;

	va_start(list, fmt);
	len = vscnprintf(buf, sizeof(buf), fmt, list);
	WSL9X_Log_Info(buf, len);
	va_end(list);
}

void os_warn(const char *fmt, ...)
{
	char buf[256];
	va_list list;
	int len;

	va_start(list, fmt);
	len = vscnprintf(buf, sizeof(buf), fmt, list);
	WSL9X_Log_Warn(buf, len);
	va_end(list);
}

void um_early_printk(const char *s, unsigned int n)
{
	WSL9X_Debug_Print(s, n);
}
