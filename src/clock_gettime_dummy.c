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

#define NSECS_IN_SEC 1000000000

#ifndef CLOCK_GETTIME_DUMMY_STEP_SECS
#  define CLOCK_GETTIME_DUMMY_STEP_SECS 0
#endif

#ifndef CLOCK_GETTIME_DUMMY_STEP_NSECS
#  define CLOCK_GETTIME_DUMMY_STEP_NSECS 1000
#endif

static struct timespec realtime;

static struct timespec monotonic;

static const struct timespec dummy_step = {
    .tv_sec = CLOCK_GETTIME_DUMMY_STEP_SECS,
    .tv_nsec = CLOCK_GETTIME_DUMMY_STEP_NSECS,
};

static
void timespec_incr(struct timespec *x, const struct timespec *step)
{
    x->tv_sec += step->tv_sec;
    x->tv_nsec += step->tv_nsec;
    if (x->tv_nsec >= NSECS_IN_SEC)
    {
        x->tv_nsec -= NSECS_IN_SEC;
        x->tv_sec++;
    }
}

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

static
void clock_dummy_advance(clockid_t clock_id)
{
    timespec_incr(clock_get(clock_id), &dummy_step);
}

static
void clock_dummy_advance_all(void)
{
    clock_dummy_advance(CLOCK_MONOTONIC);
    clock_dummy_advance(CLOCK_REALTIME);
}

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    int ret;
    struct timespec *clk;

    clock_dummy_advance_all();

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
            *res = dummy_step;
        }
    }
    return ret;
}

