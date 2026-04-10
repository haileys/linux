#include <init.h>
#include <os.h>
#include <stdarg.h>
#include <linux/string.h>
#include "win9x.h"

/* debugging */

void os_dump_core(void)
{
	unimplemented();
}

/* memory management */

int os_map_memory(void *virt, int fd, unsigned long long off,
	unsigned long len, int r, int w, int x)
{
	unimplemented();
}

int os_protect_memory(void *addr, unsigned long len,
	int r, int w, int x)
{
	unimplemented();
}

int os_unmap_memory(void *addr, int len)
{
	unimplemented();
}

int os_drop_memory(void *addr, int length)
{
	unimplemented();
}

int can_drop_memory(void)
{
	unimplemented();
}

int create_mem_file(unsigned long long len)
{
	/* TODO questionable, see if we can remove use of this */
	unimplemented();
}

void stack_protections(unsigned long address)
{
	/* stack protection is a no-op under win9x */
	(void)address;
}

/* idling */

void os_idle_prepare(void)
{
	unimplemented();
}

void os_idle_sleep(void)
{
	unimplemented();
}

/* timers */

long long os_nsecs(void)
{
	unimplemented();
}

int os_timer_create(void)
{
	unimplemented();
}

void os_timer_disable(int cpu)
{
	unimplemented();
}

int os_timer_set_interval(int cpu, unsigned long long nsecs)
{
	unimplemented();
}

int os_timer_one_shot(int cpu, unsigned long long nsecs)
{
	unimplemented();
}

void timer_set_signal_handler(void)
{
	unimplemented();
}

/* file I/O */

int os_open_file(const char *file, struct openflags flags, int mode)
{
	unimplemented();
}

void os_close_file(int fd)
{
	unimplemented();
}

int os_read_file(int fd, void *buf, int len)
{
	unimplemented();
}

int os_write_file(int fd, const void *buf, int count)
{
	unimplemented();
}

int os_seek_file(int fd, unsigned long long offset)
{
	unimplemented();
}

int os_file_size(const char *file, unsigned long long *size_out)
{
	unimplemented();
}

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

void setup_hostinfo(char *buf, int len)
{
	/* TODO more hostinfo in here */
	sized_strscpy(buf, "Win9x", len);
}

void setup_machinename(char *machine_out)
{
	/* TODO return actual arch level here */
	sized_strscpy(machine_out, "i386", 64);
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
	WSL9x_Log_Info(buf, len);
	va_end(list);
}

void os_warn(const char *fmt, ...)
{
	char buf[256];
	va_list list;
	int len;

	va_start(list, fmt);
	len = vscnprintf(buf, sizeof(buf), fmt, list);
	WSL9x_Log_Warn(buf, len);
	va_end(list);
}

void um_early_printk(const char *s, unsigned int n)
{
	WSL9x_Printk(s, n);
}
