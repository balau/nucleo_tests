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

static
int timespec_diff(const struct timespec *from, const struct timespec *to, struct timespec *diff)
{
    long diff_nsec;
    long diff_sec;
    int ret;

    /* assuming to and from have tv_nsec 0 up to 1000000000 excluded */

    diff_nsec = to->tv_nsec - from->tv_nsec;
    diff_sec = to->tv_sec - from->tv_sec;

    if (diff_nsec < 0)
    {
        diff_nsec += 1000000000;
        diff_sec--; 
    }
    if (diff != NULL)
    {
        diff->tv_nsec = diff_nsec;
        diff->tv_sec = diff_sec;
    }

    if (diff_sec == 0)
    {
        ret = diff_nsec;
    }
    else
    {
        ret = diff_sec;
    }
    return ret;
}

/* polling implementation */
int clock_nanosleep(
        clockid_t clock_id,
        int flags,
        const struct timespec *rqtp,
        struct timespec *rmtp)
{
    int ret;
    struct timespec tbegin;
    struct timespec tend;

    ret = clock_gettime(clock_id, &tbegin);
    if (flags & TIMER_ABSTIME)
    {
        timespec_diff(&tbegin, rqtp, &tend);
        rqtp = &tend;
    }

    while (ret != 0)
    {
        struct timespec tcurrent;

        ret = clock_gettime(clock_id, &tcurrent);
        if (flags & TIMER_ABSTIME)
        {
        }
        else
        {
            struct timespec telapsed;
            
            if (timespec_diff(&tbegin, &tcurrent, &telapsed) < 0)
            {
                /* time goes backwards */
            }
            if (timespec_diff(&telapsed, rqtp, NULL) <= 0)
            {
                if (rmtp != NULL)
                {
                    rmtp->tv_sec = 0;
                    rmtp->tv_nsec = 0;
                }
                break;
            }

        }
    }

    return ret;
}

