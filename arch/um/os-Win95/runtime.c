// must include these headers in a precise order to get things to compile...
#undef __KERNEL__
#define __EXPORTED_HEADERS__
// #include <asm-generic/posix_types.h>
#include <linux/time.h>
#undef __EXPORTED_HEADERS__
#define __KERNEL__
// typedef __kernel_long_t	__kernel_time_t;
// #include <linux/time.h>
// #define _SSIZE_T
// #define _PTRDIFF_T
// #define _NOLIBC_STD_H
// #include <linux/types.h>
#include <stdbool.h>
#include <asm-generic/unistd.h>
#define _NOLIBC_SYS_SELECT_H
#include <nolibc.h>
#include <sys.h>

// Linux-based runtime harness for Win9x UML
#include <wsl9x.h>

// size_t strlen(const char *s);

void WSL9x_Printks(const char* str)
{
	WSL9x_Printk(str, strlen(str));
}

static void sys_write_all(int fd, const char* buf, size_t len)
{
	while (len) {
		ssize_t rc = sys_write(fd, buf, len);
		if (rc < 0) {
			break;
		}
		buf += rc;
		len -= rc;
	}
}

void WSL9x_Printk(const char* str, size_t len)
{
	sys_write_all(2, str, len);
}

void WSL9x_Log_Info(const char* str, size_t len)
{
	WSL9x_Printks("[info] ");
	WSL9x_Printk(str, len);
}

void WSL9x_Log_Warn(const char* str, size_t len)
{
	WSL9x_Printks("[WARN] ");
	WSL9x_Printk(str, len);
}

HMEM VMM_PageReserve(uint32_t virt_pfn, uint32_t npages, uint32_t flags_)
{
	const uint32_t NR_mmap2 = 192;
	// const uint32_t PROT_READ = 1;
	// const uint32_t PROT_WRITE = 2;
	// const uint32_t MAP_PRIVATE = 0x02;
	// const uint32_t MAP_ANONYMOUS = 0x20;
	// const uint32_t MAP_32BIT = 0x40;
	uint32_t len = npages * 4096;
	uint32_t prot = PROT_READ | PROT_WRITE;
	uint32_t flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT;
	void* rc;
	asm volatile ("pushl %%ebp \n xorl %%ebp, %%ebp \n int $0x80 \n popl %%ebp"
		: "=a"(rc)
		: "a"(NR_mmap2), "b"(0), "c"(len), "d"(prot), "S"(flags), "D"(-1), "ebp"(0));
	return rc;
}

void VMMTerminateThread(VMM_THREAD_HANDLE thread)
{
	WSL9x_Printks("VMMTerminateThread not implemented!\n");
}
