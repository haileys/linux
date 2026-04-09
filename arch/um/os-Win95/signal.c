#include <os.h>
#include "win9x.h"

void block_signals(void)
{
	unimplemented();
}

void unblock_signals(void)
{
	unimplemented();
}

int um_get_signals(void)
{
	unimplemented();
}

int um_set_signals(int enable)
{
	unimplemented();
}

void set_sigstack(void *sig_stack, int size)
{
	/* TODO set_sigstack - pipe this into a supporting kernel driver */
}

void init_new_thread_signals(void)
{
	/* TODO init_new_thread_signals */
}
