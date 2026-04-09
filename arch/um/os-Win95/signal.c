#include <os.h>
#include "win9x.h"

/* TODO signal/interrupt blocking/unblocking. currently noop */

static int signals_enabled = 0;

void block_signals(void)
{
	signals_enabled = 0;
}

void unblock_signals(void)
{
	signals_enabled = 1;
}

int um_get_signals(void)
{
	return signals_enabled;
}

int um_set_signals(int enable)
{
	int prev = signals_enabled;
	signals_enabled = enable;
	return prev;
}
