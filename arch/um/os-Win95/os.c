#include <init.h>
#include <os.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

/* process management */

pid_t os_reap_child(void)
{
	unimplemented();
}

void os_kill_ptraced_process(int pid, int reap_child)
{
	unimplemented();
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

long long os_persistent_clock_emulation(void)
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

void os_alarm_process(int pid)
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

ssize_t os_getrandom(void *buf, size_t len, unsigned int flags)
{
	unimplemented();
}

void setup_hostinfo(char *buf, int len)
{
	/* TODO more hostinfo in here */
	strcpy(buf, "Win9x");
}

void setup_machinename(char *machine_out)
{
	/* TODO return actual arch level here */
	strcpy(machine_out, "i386");
}

/* logging */

static void os_log_message(const char* s, size_t n)
{
	while (n) {
		ssize_t rc = write(2, s, n);
		if (rc <= 0) {
			break;
		}
		n -= rc;
	}
}

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
	os_log_message(buf, len);
	va_end(list);
}

void os_warn(const char *fmt, ...)
{
	char buf[256];
	va_list list;
	int len;

	va_start(list, fmt);
	len = vscnprintf(buf, sizeof(buf), fmt, list);
	os_log_message(buf, len);
	va_end(list);
}

void um_early_printk(const char *s, unsigned int n)
{
	os_log_message(s, n);
}
