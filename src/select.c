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
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include "timespec.h"

static
int get_fd_set_mask_idx(int fd)
{
    fd_set *s;

    return (fd / (8*sizeof(s->mask[0])));
}

static
unsigned long get_fd_set_mask_bitmask(int fd)
{
    fd &= ((8*sizeof(unsigned long)) - 1);

    return (1UL<<fd);
}

void FD_CLR(int fd, fd_set *fdset)
{
    if ((fd >= 0) && (fd < FD_SETSIZE) && (fdset != NULL))
    {
        int i;
        unsigned long bitmask;

        i = get_fd_set_mask_idx(fd);
        bitmask = get_fd_set_mask_bitmask(fd);

        fdset->mask[i] &= ~bitmask;
    }
}

int FD_ISSET(int fd, fd_set *fdset)
{
    int ret;

    if ((fd >= 0) && (fd < FD_SETSIZE) && (fdset != NULL))
    {
        int i;
        unsigned long bitmask;

        i = get_fd_set_mask_idx(fd);
        bitmask = get_fd_set_mask_bitmask(fd);

        ret = ((fdset->mask[i] & bitmask) != 0);
    }
    else
    {
        ret = 0;
    }

    return ret;
}

void FD_SET(int fd, fd_set *fdset)
{
    if ((fd >= 0) && (fd < FD_SETSIZE) && (fdset != NULL))
    {
        int i;
        unsigned long bitmask;

        i = get_fd_set_mask_idx(fd);
        bitmask = get_fd_set_mask_bitmask(fd);

        fdset->mask[i] |= bitmask;
    }
}

void FD_ZERO(fd_set *fdset)
{
    if (fdset != NULL)
    {
        memset(fdset, 0, sizeof(fd_set));
    }
}

static
int pselect_one(
        int fd,
        fd_set *readfds,
        fd_set *writefds,
        fd_set *errorfds)
{
    int ret;

    if (FD_ISSET(fd, readfds) || FD_ISSET(fd, writefds) || FD_ISSET(fd, errorfds))
    {
        struct pollfd p;
        int pollret;

        p.fd = fd;
        p.events =
            POLLIN|POLLRDNORM|POLLRDBAND|POLLPRI|
            POLLOUT|POLLWRNORM|POLLWRBAND|
            POLLERR|POLLHUP|POLLNVAL;
        pollret = poll(&p, 1, 0);
        if ((pollret == -1) || (p.revents & POLLNVAL))
        {
            errno = EBADF;
            ret = -1;
        }
        else
        {
            if (!(p.revents & (POLLIN|POLLRDNORM|POLLERR|POLLHUP)))
            {
                FD_CLR(fd, readfds);
            }
            if (!(p.revents & (POLLOUT|POLLWRNORM|POLLERR|POLLHUP)))
            {
                FD_CLR(fd, writefds);
            }
            if (!(p.revents & (POLLRDBAND|POLLWRBAND|POLLPRI)))
            {
                FD_CLR(fd, errorfds);
            }
            if (FD_ISSET(fd, readfds) || FD_ISSET(fd, writefds) || FD_ISSET(fd, errorfds))
            {
                ret = 1;
            }
            else
            {
                ret = 0;
            }
        }
    }
    else
    {
        ret = 0;
    }

    return ret;
}

static
int pselect_tentative(
        int nfds,
        fd_set *readfds,
        fd_set *writefds,
        fd_set *errorfds)
{
    int ret;
    int fd;

    ret = 0;
    for (fd = 0; fd < nfds; fd++)
    {
        int ret_one;

        ret_one = pselect_one(fd, readfds, writefds, errorfds);
        if (ret_one == -1)
        {
            ret = -1;
            break;
        }
        else
        {
            ret += ret_one;
        }
    }

    return ret;
}

int pselect(int nfds, fd_set *readfds,
       fd_set *writefds, fd_set *errorfds,
       const struct timespec *timeout,
       const sigset_t *sigmask)
{
    int ret;
    struct timespec tend;
    int timeout_expired;
    fd_set readfds_in;
    fd_set writefds_in;
    fd_set errorfds_in;

    (void)sigmask; /* TODO when we have signals */

    if (timeout == NULL)
    {
        tend = TIMESPEC_INFINITY;
    }
    else
    {
        ret = clock_gettime(CLOCK_MONOTONIC, &tend);

        timespec_add(&tend, timeout, &tend);
    }

    /* save fd sets to reinit them after each tentative */
    readfds_in = *readfds;
    writefds_in = *writefds;
    errorfds_in = *errorfds;
    do
    {
        struct timespec tcurrent;

        ret = clock_gettime(CLOCK_MONOTONIC, &tcurrent);
        if (ret != 0)
        {
            break;
        }

        ret = pselect_tentative(nfds, readfds, writefds, errorfds);
        if (ret != 0)
        {
            break;
        }

        timeout_expired = (timespec_diff(&tcurrent, &tend, NULL) >= 0);
        if (!timeout_expired)
        {
            /* reinit fd sets */
            *readfds = readfds_in;
            *writefds = writefds_in;
            *errorfds = errorfds_in;
        }
    } while(!timeout_expired);

    return ret;
}


int select(int nfds, fd_set *readfds,
       fd_set *writefds, fd_set *errorfds,
       struct timeval *timeout)
{
    struct timespec timeout_ts;

    timeval_to_timespec(timeout, &timeout_ts);

    return pselect(nfds, readfds, writefds, errorfds, &timeout_ts, NULL);
}


