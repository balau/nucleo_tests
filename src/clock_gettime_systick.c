/*
 * Copyright (c) 2015 Francesco Balducci
 *
 * This file is part of nucleo_tests.
 *
 *    nucleo_tests is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    nucleo_tests is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with nucleo_tests.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <time.h>
#include <errno.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include "timespec.h"

#ifndef CLOCK_GETTIME_SYNC_DISABLED
#  include "timesync.h"
#endif

#define SYSTICK_NSEC 1000000
#define SYSTICK_FREQ_HZ (NSECS_IN_SEC/SYSTICK_NSEC)

void sys_tick_handler(void);
void clock_gettime_systick_init(void);

static volatile struct timespec realtime;

static volatile struct timespec monotonic;

static volatile int timer_update_flag;

static const struct timespec systick_step = {
    .tv_sec = 0,
    .tv_nsec = SYSTICK_NSEC
};

static
volatile struct timespec *clock_get(clockid_t clock_id)
{
    volatile struct timespec *clk;

    switch(clock_id)
    {
        case CLOCK_MONOTONIC:
            clk = &monotonic;
            break;
        case CLOCK_REALTIME:
            clk = &realtime;
            break;
        default:
            clk = NULL;
            break;
    }
    return clk;
}

static
void systick_fraction_to_timespec(uint32_t fraction, struct timespec *tp)
{
    uint32_t ticks;
    
    ticks = (rcc_ahb_frequency/SYSTICK_FREQ_HZ) - fraction;
    tp->tv_sec = 0; /* assuming  SYSTICK_NSEC < NSECS_IN_SEC */
    tp->tv_nsec = ticks * (NSECS_IN_SEC / rcc_ahb_frequency);
}

#ifndef CLOCK_GETTIME_SYNC_DISABLED

/* Empty default that does nothing, in case
 * there is no implementation of timesync
 * linked in the program.
 */
__attribute__((__weak__))
int timesync_timespec(struct timespec *t)
{
    (void)t;
    return 0;
}

#endif

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    int ret;
    volatile struct timespec *clk;

    clk = clock_get(clock_id);
    if (clk == NULL)
    {
        ret = -1;
        errno = EINVAL;
    }
    else
    {
        int flag_before;
        int flag_after;
        uint32_t fraction_ticks;
        struct timespec fraction_ts;

        do {
            flag_before = timer_update_flag;
            *tp = *clk;
            fraction_ticks = systick_get_value();
            flag_after = timer_update_flag;
            /* if they are the same, no systick occurred.
             * note that seqlock is unnecessary because
             * systick is an interrupt, not a thread.
             */
        } while (flag_before != flag_after);
        systick_fraction_to_timespec(fraction_ticks, &fraction_ts);
        timespec_incr(tp, &fraction_ts);
#ifndef CLOCK_GETTIME_SYNC_DISABLED
        if (clock_id == CLOCK_REALTIME)
        {
            int sync_ret;

            /* timesync_timespec does not use clock_gettime,
             * so that we do not get stuck in a recursion.
             */
            sync_ret = timesync_timespec(tp);

            (void)sync_ret; /* ignore */
        }
#endif
        ret = 0;
    }
    return ret;
}

int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
    int ret;
    volatile struct timespec *clk;

    clk = clock_get(clock_id);
    if (clk == NULL)
    {
        ret = -1;
        errno = EINVAL;
    }
    else if (clock_id == CLOCK_MONOTONIC)
    {
        ret = -1;
        errno = EINVAL;
    }
    else if (tp == NULL)
    {
        ret = -1;
        errno = EINVAL;
    }
    else if (tp->tv_nsec < 0)
    {
        ret = -1;
        errno = EINVAL;
    }
    else if (tp->tv_nsec >= NSECS_IN_SEC)
    {
        ret = -1;
        errno = EINVAL;
    }
    else
    {
        int flag_before;
        int flag_after;
        do {
            flag_before = timer_update_flag;
            *clk = *tp;
            flag_after = timer_update_flag;
            /* if they are the same, no systick occurred.
             * note that seqlock is unnecessary because
             * systick is an interrupt, not a thread.
             */
        } while (flag_before != flag_after);
        ret = 0;
    }

    return ret;
}

int clock_getres(clockid_t clock_id, struct timespec *res)
{
    int ret;

    if (clock_get(clock_id) == NULL)
    {
        ret = -1;
        errno = EINVAL;
    }
    else
    {
        ret = 0;
        if (res != NULL)
        {
            systick_fraction_to_timespec(1, res);
        }
    }
    return ret;
}

static
void sys_tick_incr(clockid_t clock_id)
{
    struct timespec *clk;

    /* we can discard volatile because we are already in sys_tick_handler */
    clk = (struct timespec *)clock_get(clock_id);

    timespec_incr(clk, &systick_step);
}

void sys_tick_handler(void)
{
    sys_tick_incr(CLOCK_MONOTONIC);
    sys_tick_incr(CLOCK_REALTIME);
    timer_update_flag++;
}

__attribute__((__constructor__))
void clock_gettime_systick_init(void)
{
    systick_set_frequency(SYSTICK_FREQ_HZ, rcc_ahb_frequency);
    systick_interrupt_enable();
    systick_counter_enable();
}

