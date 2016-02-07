/*
 * Copyright (c) 2016 Francesco Balducci
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
#include "timesync.h"
#include <time.h>
#include "timespec.h"

#define TIMESYNC_METHOD_RFC868 1
#define TIMESYNC_METHOD_SNTP   2

#ifndef TIMESYNC_METHOD
#  define TIMESYNC_METHOD TIMESYNC_METHOD_SNTP
#endif

#if (TIMESYNC_METHOD != TIMESYNC_METHOD_RFC868) && (TIMESYNC_METHOD != TIMESYNC_METHOD_SNTP)
/* TODO: NTP */
#  error Supported TIMESYNC_METHOD values: TIMESYNC_METHOD_RFC868 TIMESYNC_METHOD_SNTP
#endif

#ifndef TIMESYNC_INTERVAL
#  define TIMESYNC_INTERVAL (60*60*24) /* 1 day */
#endif

static
struct timespec next_sync = {0, 0};

static
struct timespec sync_interval = {
    .tv_sec = TIMESYNC_INTERVAL,
    .tv_nsec = 0};

static
int time_to_sync(const struct timespec *now)
{
    int res;

    if (now == NULL)
    {
        res = 1; /* assuming TIMESPEC_ZERO */
    }
    else
    {
        res = (timespec_diff(now, &next_sync, NULL) >= 0)?1:0;
    }

    return res;
}

int timesync_now_timespec(struct timespec *out)
{
    int res;
    int gettime_ret;
    struct timespec now;

#if (TIMESYNC_METHOD == TIMESYNC_METHOD_RFC868)
    gettime_ret = rfc868_gettime(&now);
#elif (TIMESYNC_METHOD == TIMESYNC_METHOD_SNTP)
    gettime_ret = sntp_gettime(&now);
#endif

    if (gettime_ret == 0)
    {
        int clk_ret;

        clk_ret = clock_settime(CLOCK_REALTIME, &now);
        if (clk_ret == 0)
        {
            if (out != NULL)
            {
                *out = now;
            }
            timespec_add(&now, &sync_interval, &next_sync);
            res = 1;
        }
        else
        {
            res = -1;
        }
    }
    else
    {
        res = -1;
    }
    return res;
}

int timesync_timespec(struct timespec *now)
{
    int res;

    if (time_to_sync(now))
    {
        res = timesync_now_timespec(now);
    }
    else
    {
        res = 0;
    }

    return res;
}

int timesync(void)
{
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    return timesync_timespec(&now);
}

int timesync_now(void)
{
    return timesync_now_timespec(NULL);
}

