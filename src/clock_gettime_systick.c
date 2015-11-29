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

#define SYSTICK_NSEC 1000000
#define SYSTICK_FREQ_HZ (NSECS_IN_SEC/SYSTICK_NSEC)

void sys_tick_handler(void);
void clock_gettime_systick_init(void);

static struct timespec realtime;

static struct timespec monotonic;

static const struct timespec systick_step = {
    .tv_sec = 0,
    .tv_nsec = SYSTICK_NSEC
};

static
struct timespec *clock_get(clockid_t clock_id)
{
    struct timespec *clk;

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

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    int ret;
    struct timespec *clk;

    clk = clock_get(clock_id);
    if (clk == NULL)
    {
        ret = -1;
        errno = EINVAL;
    }
    else
    {
        *tp = *clk;
        ret = 0;
    }
    return ret;
}

int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
    int ret;
    struct timespec *clk;

    clk = clock_get(clock_id);
    if (clk == NULL)
    {
        ret = EINVAL;
    }
    else if (clock_id == CLOCK_MONOTONIC)
    {
        ret = EINVAL;
    }
    else if (tp == NULL)
    {
        ret = EINVAL;
    }
    else if (tp->tv_nsec < 0)
    {
        ret = EINVAL;
    }
    else if (tp->tv_nsec >= NSECS_IN_SEC)
    {
        ret = EINVAL;
    }
    else
    {
        *clk = *tp;
        ret = 0;
    }

    return ret;
}

int clock_getres(clockid_t clock_id, struct timespec *res)
{
    int ret;

    if (clock_get(clock_id) == NULL)
    {
        ret = EINVAL;
    }
    else
    {
        ret = 0;
        if (res != NULL)
        {
            *res = systick_step;
        }
    }
    return ret;
}

void sys_tick_handler(void)
{
    timespec_incr(clock_get(CLOCK_MONOTONIC), &systick_step);
    timespec_incr(clock_get(CLOCK_REALTIME), &systick_step);
}

__attribute__((__constructor__))
void clock_gettime_systick_init(void)
{
    systick_set_frequency(SYSTICK_FREQ_HZ, rcc_ahb_frequency);
    systick_interrupt_enable();
    systick_counter_enable();
}

