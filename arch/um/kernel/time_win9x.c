#include "vdso/time64.h"
#include <linux/time.h>
#include <linux/init.h>
#include <linux/clocksource.h>
#include <wsl9x/time.h>

void read_persistent_clock64(struct timespec64 *ts)
{
	u64 msecs = VTD_Get_Date_And_Time() + WIN9X_WALL_CLOCK_EPOCH;
	ts->tv_sec = msecs / MSEC_PER_SEC;
	ts->tv_nsec = (msecs % MSEC_PER_SEC) * NSEC_PER_MSEC;
}

static u64 win9x_timer_read(struct clocksource* cs)
{
	return VTD_Get_Real_Time();
}

static struct clocksource win9x_clocksource = {
	.name		= "win9x-vtd",
	.rating		= 200,
	.read		= win9x_timer_read,
	.mask		= CLOCKSOURCE_MASK(64),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __init win9x_timer_init(void)
{
	int err = clocksource_register_hz(&win9x_clocksource, WIN9X_REAL_CLOCK_HZ);
	if (err) {
		panic("clocksource_register_hz err: %d", err);
	}
}

void __init time_init(void)
{
	late_time_init = win9x_timer_init;
}
