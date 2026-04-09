#include <as-layout.h>
#include <init.h>
#include <os.h>

void __init os_check_bugs(void)
{
	/* do nothing */
}

void __init os_early_checks(void)
{
	/* do nothing */
}

void  __init get_host_cpu_features(
		void (*flags_helper_func)(char *line),
		void (*cache_helper_func)(char *line))
{
	/* TODO return actual cpu feature info */
	flags_helper_func("fpu");
	cache_helper_func("16");
}

int __init set_umid(char *name)
{
	/* ignore */
	return 0;
}
