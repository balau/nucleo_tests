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
#include "timespec.h"

/* polling implementation */
int clock_nanosleep(
        clockid_t clock_id,
        int flags,
        const struct timespec *rqtp,
        struct timespec *rmtp)
{
    int ret;
    struct timespec tcurrent;
    struct timespec tend;

    ret = clock_gettime(clock_id, &tcurrent);

    if (!(flags & TIMER_ABSTIME))
    {
        ret = clock_gettime(clock_id, &tcurrent);
        timespec_add(&tcurrent, rqtp, &tend);
        rqtp = &tend;
    }

    while (ret == 0)
    {
        if (timespec_diff(&tcurrent, rqtp, NULL) <= 0)
        {
            if (rmtp != NULL)
            {
                rmtp->tv_sec = 0;
                rmtp->tv_nsec = 0;
            }
            break;
        }
        ret = clock_gettime(clock_id, &tcurrent);
    }

    return ret;
}

