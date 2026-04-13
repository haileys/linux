// Linux-based runtime harness for Win9x UML

// must include these headers in a precise order to get things to compile...
// #include <asm/mman.h>
#include <asm/mman.h>
#undef __KERNEL__
#define __EXPORTED_HEADERS__
#include <linux/time.h>
#undef __EXPORTED_HEADERS__
#define __KERNEL__

#include <asm/unistd_32.h>
#include <stdbool.h>
#define _NOLIBC_SYS_SELECT_H
#define NOLIBC_NO_RUNTIME
// #include <nolibc.h>
#include <arch-x86.h>

// we're good
#include <vdso/time64.h>
#include <mem_user.h>
#include <wsl9x.h>
#include <asm-generic/mman-common.h>
#include <linux/mman.h>

size_t strlen(const char* s);

#define __NR_write 4
#define __NR_mmap2 192
#define __NR_munmap 91
#define __NR_clock_gettime64 403
#define __NR_memfd_create 356
#define __NR_ftruncate 93
#define __NR_fallocate 324
#define __NR_mprotect 125

void WSL9x_Printks(const char* str)
{
	WSL9x_Printk(str, strlen(str));
}

static void sys_write_all(int fd, const char* buf, size_t len)
{
	while (len) {
		ssize_t rc = my_syscall3(__NR_write, fd, buf, len);
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

uint32_t VMM_PageReserve(uint32_t virt_pfn, uint32_t npages, uint32_t flags_)
{
	if (virt_pfn != PR_SYSTEM) {
		panic("virt_pfn != PR_SYSTEM");
	}


	// take next allocated system arena addr and bump it to next 4M boundary
	static uint32_t system_addr = 0xc1000000;
	uint32_t addr = system_addr;
	system_addr = ROUND_4M(system_addr + ((npages + 1) * 4096));

	return (HMEM)addr;

	uint32_t len = npages * 4096;
	uint32_t prot = PROT_READ | PROT_WRITE;
	uint32_t flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT | MAP_FIXED;

	if (my_syscall6(__NR_mmap2, addr, len, prot, flags, -1, 0) != addr) {
		WSL9x_Printks("mmap failed!\n");
		unimplemented();
	}

	return (HMEM)addr;
}

static int memfd = -1;

uint32_t VMM_PageCommit(uint32_t pagenum, uint32_t npages, uint32_t pager, uint32_t pagerdata, uint32_t flags_)
{
	if (pager != PD_FIXEDZERO || flags_ != PC_FIXED) {
		panic("bad flags to VMM_PageCommit");
		return 1;
	}

	if (memfd >= 0) {
		panic("VMM_PageCommit already called");
		return 1;
	}

	memfd = my_syscall2(__NR_memfd_create, "-", 0);
	if (memfd < 0) {
		panic("memfd failed: %d", memfd);
		return 1;
	}

	uint32_t len = npages * 4096;

	int err = my_syscall2(__NR_ftruncate, memfd, len);
	if (err) {
		panic("ftruncate failed: %d", err);
		return 1;
	}

	// int err = my_syscall4(__NR_fallocate, memfd, 0, 0, len);
	// if (err) {
	// 	panic("fallocate failed: %d", err);
	// 	return 1;
	// }

	uint32_t addr = pagenum * 4096;
	uint32_t prot = PROT_READ | PROT_WRITE;
	uint32_t flags = MAP_PRIVATE | MAP_32BIT | MAP_FIXED;

	if (my_syscall6(__NR_mmap2, addr, len, prot, flags, memfd, 0) != addr) {
		panic("mmap failed!");
		return 0;
	}

	return 1;
}

uint32_t VMM_PageCommitPhys(uint32_t pagenum, uint32_t npages, uint32_t physnum, uint32_t flags_)
{
	if (memfd < 0) {
		panic("VMM_PageCommit not yet called to setup physmem");
		return 1;
	}

	uint32_t addr = pagenum * 4096;
	uint32_t len = npages * 4096;

	uint32_t prot = PROT_READ;
	if (flags_ & PC_WRITEABLE) {
		prot |= PROT_WRITE;
	} else {
		prot |= PROT_EXEC;
	}

	uint32_t flags = MAP_PRIVATE | MAP_32BIT | MAP_FIXED;

	if (my_syscall6(__NR_mmap2, addr, len, prot, flags, memfd, physnum) != addr) {
		panic("mmap failed!");
		return 0;
	}

	return 1;
}

uint32_t VMM_PageDecommit(uint32_t pagenum, uint32_t npages, uint32_t flags)
{
	uint32_t addr = pagenum * 4096;
	uint32_t len = npages * 4096;
	if (my_syscall2(__NR_munmap, addr, len)) {
		panic("munmap failed!");
		return 0;
	}

	return 1;
}

void VMMTerminateThread(VMM_THREAD_HANDLE thread)
{
	WSL9x_Printks("VMMTerminateThread not implemented!\n");
}

struct timespec64_ {
	uint64_t tv_sec;
	uint64_t tv_nsec;
};

static struct timespec64_ get_clock(uint32_t clock)
{
	struct timespec64_ tp;
	int err = my_syscall2(__NR_clock_gettime64, CLOCK_BOOTTIME, &tp);
	if (err) {
		panic("sys_clock_gettime failed: err=%d", err);
	}
	return tp;
}

uint64_t VTD_Get_Real_Time(void)
{
	struct timespec64_ tp = get_clock(CLOCK_BOOTTIME);
	uint64_t nsec_clocks = udiv64(tp.tv_nsec, WIN9X_NSEC_PER_REAL_CLOCK).quo;
	uint64_t clocks = tp.tv_sec * WIN9X_REAL_CLOCK_HZ;
	return clocks + nsec_clocks;
}

uint64_t VTD_Get_Date_And_Time(void)
{
	struct timespec64_ tp = get_clock(CLOCK_REALTIME);
	tp.tv_sec -= WIN9X_WALL_CLOCK_EPOCH;
	return tp.tv_sec + udiv64(tp.tv_nsec, NSEC_PER_SEC).quo;
}

uint32_t VMM_PageModifyPermissions(uint32_t pagenum, uint32_t npages, uint32_t perm_and, uint32_t perm_or)
{
	if (perm_and != ~PC_WRITEABLE || perm_or != 0) {
		panic("unsupported flags in VMM_PageModifyPermissions");
	}

	if (my_syscall3(__NR_mprotect, pagenum * 4096, npages * 4096, PROT_READ)) {
		panic("mprotect failed");
	}

	return 1;
}
