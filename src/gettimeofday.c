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
#include <sys/time.h>
#include <time.h>
#include "timespec.h"

/* Used by newlib as system call */
extern
int _gettimeofday(struct timeval *tp, void *tzp);

int _gettimeofday(struct timeval *tp, void *tzp)
{
    int ret;

    struct timespec ts;

    (void)tzp; /* ignore */

    ret = clock_gettime(CLOCK_REALTIME, &ts);

    timespec_to_timeval(&ts, tp);

    (void)ret; /* ignore and return 0 anyway */
    return 0;
}

