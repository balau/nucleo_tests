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
#include "timespec.h"
#include <limits.h>

const struct timespec TIMESPEC_ZERO = {0, 0};
const struct timespec TIMESPEC_INFINITY = {INT_MAX, LONG_MAX};

void timespec_add(const struct timespec *t1, const struct timespec *t2, struct timespec *dst)
{
    dst->tv_sec = t1->tv_sec + t2->tv_sec;
    dst->tv_nsec = t1->tv_nsec + t2->tv_nsec;
    if (dst->tv_nsec >= NSECS_IN_SEC)
    {
        dst->tv_nsec -= NSECS_IN_SEC;
        dst->tv_sec++;
    }
}

void timespec_incr(struct timespec *x, const struct timespec *step)
{
    struct timespec tmp;

    tmp = *x;
    timespec_add(&tmp, step, x);
}

int timespec_diff(const struct timespec *to, const struct timespec *from, struct timespec *diff)
{
    long diff_nsec;
    long diff_sec;
    int ret;

    /* assuming to and from have tv_nsec 0 up to 1000000000 excluded */

    diff_nsec = to->tv_nsec - from->tv_nsec;
    diff_sec = to->tv_sec - from->tv_sec;

    if (diff_nsec < 0)
    {
        diff_nsec += NSECS_IN_SEC;
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

void timespec_to_timeval(const struct timespec *src, struct timeval *dst)
{
    dst->tv_sec = src->tv_sec;
    dst->tv_usec = src->tv_nsec / (NSECS_IN_SEC / USECS_IN_SEC);
}

extern
void timeval_to_timespec(const struct timeval *src, struct timespec *dst)
{
    dst->tv_sec = src->tv_sec;
    dst->tv_nsec = src->tv_usec * (NSECS_IN_SEC / USECS_IN_SEC);
}

