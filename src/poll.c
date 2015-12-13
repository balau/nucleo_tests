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
#include <poll.h>
#include <errno.h>
#include "file.h"
#include "time.h"
#include "timespec.h"

static
short poll_one(struct pollfd *p)
{
    short revents;

    if (p->fd < 0)
    {
        revents = 0;
    }
    else
    {
        struct fd *f;

        f = file_struct_get(p->fd);
        if (f == NULL)
        {
            revents = POLLNVAL;
        }
        else if (f->poll == NULL)
        {
            revents = POLLNVAL;
        }
        else
        {
            revents = f->poll(p->fd);
            if (revents == -1)
            {
                revents = POLLNVAL;
            }
        }
    }
    if (revents >= 0)
    {
        short events;

        events = p->events;
        events |= POLLHUP|POLLERR|POLLNVAL;
        revents &= events;
        p->revents = revents;
    }

    return revents;
}

static
int poll_tentative(struct pollfd fds[], nfds_t nfds)
{
    int ret;
    nfds_t i;
    int nfiles_ready;

    nfiles_ready = 0;

    for (i = 0; i < nfds; i++)
    {
        short revents;

        revents = poll_one(&fds[i]);
        if (revents == -1)
        {
            break;
        }
        else if (revents > 0)
        {
            nfiles_ready++;
        }
    }
    if (i == nfds)
    {
        ret = nfiles_ready;
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
    int ret;

    if (timeout == 0)
    {
        ret = poll_tentative(fds, nfds);
    }
    else
    {
        struct timespec tend;
        int timeout_expired;

        if (timeout == -1)
        {
            tend = TIMESPEC_INFINITY;
        }
        else
        {
            struct timespec tcurrent;
            struct timespec timeout_ts;

            timeout_ts.tv_sec = timeout / MSECS_IN_SEC;
            timeout -= timeout_ts.tv_sec * MSECS_IN_SEC;
            timeout_ts.tv_nsec = timeout * (NSECS_IN_SEC / MSECS_IN_SEC);

            ret = clock_gettime(CLOCK_MONOTONIC, &tcurrent);

            timespec_add(&tcurrent, &timeout_ts, &tend);
        }
        do
        {
            struct timespec tcurrent;

            ret = clock_gettime(CLOCK_MONOTONIC, &tcurrent);
            if (ret != 0)
            {
                break;
            }

            ret = poll_tentative(fds, nfds);
            if (ret != 0)
            {
                break;
            }

            timeout_expired = (timespec_diff(&tcurrent, &tend, NULL) >= 0);
        } while(!timeout_expired);

    }

    return ret;
}

