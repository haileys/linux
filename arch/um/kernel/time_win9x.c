#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/smp.h>
#include <linux/stddef.h>
#include <linux/time.h>
#include <asm/div64.h>
#include <vdso/time64.h>
#include <wsl9x.h>
#include <wsl9x/time.h>

/*
static const TIMER_INTERVAL_MSEC = (NSEC_PER_SEC / HZ) / NSEC_PER_MSEC;

static VMM_TIMEOUT_HANDLE timeout_handle = NULL;
static bool is_periodic = false;

static void win9x_set_timeout(void);

static void _vmm_timer_callback win9x_timeout_callback(uint32_t millis_late, void* ctx)
{
	timeout_handle = NULL;
	if (is_periodic) {
		win9x_set_timeout();
	}
}

static void win9x_set_timeout(void)
{
	if (!timeout_handle) {
		timeout_handle = VMM_Set_Async_Time_Out(TIMER_INTERVAL_MSEC, NULL, win9x_timeout_callback);
	}
}

static void win9x_cancel_timeout(void)
{
	VMM_TIMEOUT_HANDLE handle = NULL;
	handle = xchg(&timeout_handle, handle);
	VMM_Cancel_Time_Out(handle);
}

static int win9x_timer_shutdown(struct clock_event_device *c)
{
	win9x_cancel_timeout();
}

static int win9x_timer_set_periodic(struct clock_event_device *c)
{
	is_periodic = true;
	win9x_set_timeout();
}

static int win9x_timer_one_shot(struct clock_event_device *c)
{
	is_periodic = false;
	win9x_set_timeout();
}

static int win9x_timer_next_event(unsigned long delta, struct clock_event_device *c)
{
	is_periodic = false;
	win9x_set_timeout(); // TODO what to do with delta?
}

static struct clock_event_device win9x_clockevent = {
	.name			= "win9x-timer",
	.rating			= 250,
	.features		= CLOCK_EVT_FEAT_PERIODIC |
				  CLOCK_EVT_FEAT_ONESHOT,
	.set_state_shutdown	= win9x_timer_shutdown,
	.set_state_periodic	= win9x_timer_set_periodic,
	.set_state_oneshot	= win9x_timer_one_shot,
	.set_next_event		= win9x_timer_next_event,
	.shift			= 0,
	.max_delta_ns		= 0xffffffff,
	.max_delta_ticks	= 0xffffffff,
	.min_delta_ns		= TIMER_MIN_DELTA,
	.min_delta_ticks	= TIMER_MIN_DELTA, // microsecond resolution should be enough for anyone, same as 640K RAM
	.irq			= 0,
	.mult			= 1,
};

static irqreturn_t win9x_timer_irq(int irq, void *dev)
{
	struct clock_event_device *evt = &win9x_clockevent;
	evt->event_handler(evt);

	return IRQ_HANDLED;
}
*/

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
	int err;

	// err = request_irq(TIMER_IRQ, win9x_timer_irq, IRQF_TIMER, "hr timer", NULL);
	// if (err != 0) {
	// 	panic("win9x_timer_init: request_irq error: %d", err);
	// }

	// win9x_clockevent.cpumask = cpumask_of(smp_processor_id());
	// clockevents_register_device(&win9x_clockevent);

	err = clocksource_register_hz(&win9x_clocksource, WIN9X_REAL_CLOCK_HZ);
	if (err) {
		panic("win9x_timer_init: clocksource_register_hz error: %d", err);
	}
}

void __init time_init(void)
{
	late_time_init = win9x_timer_init;
	lpj_fine = WIN9X_REAL_CLOCK_HZ / HZ;
}

void read_persistent_clock64(struct timespec64 *ts)
{
	u64 msecs = VTD_Get_Date_And_Time() + WIN9X_WALL_CLOCK_EPOCH;

	u32 remainder = 0;
	ts->tv_sec = div_u64_rem(msecs, MSEC_PER_SEC, &remainder);
	ts->tv_nsec = remainder * NSEC_PER_MSEC;
}
