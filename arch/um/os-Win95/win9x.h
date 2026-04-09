#include <linux/compiler_attributes.h>
#include <wsl9x.h>

static inline __noreturn void unimplemented(void)
{
	__asm__ volatile ("int3" ::: "memory");
	__asm__ volatile ("ud2" ::: "memory");
	for (;;) ;
}
