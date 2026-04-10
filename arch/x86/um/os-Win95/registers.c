#include <sysdep/archsetjmp.h>
#include <sysdep/ptrace.h>

unsigned long host_fp_size;

unsigned long get_thread_reg(int reg, jmp_buf *buf)
{
	switch (reg) {
	case HOST_IP:
		return buf[0]->__eip;
	case HOST_SP:
		return buf[0]->__esp;
	case HOST_BP:
		return buf[0]->__ebp;
	default:
		printk(UM_KERN_ERR "get_thread_regs - unknown register %d\n",
		       reg);
		return 0;
	}
}
