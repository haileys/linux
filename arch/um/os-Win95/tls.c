#include <sysdep/tls.h>
#include <uapi/linux/errno.h>

extern int os_set_thread_area(user_desc_t *info, VMM_THREAD_HANDLE th)
{
	// TODO - TLS not yet implemented
	return ESRCH;
}
