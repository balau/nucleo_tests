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
#ifndef POLL_H
#define POLL_H

struct pollfd
{
    int fd;
    short events;
    short revents;
};

typedef unsigned int nfds_t;

#define POLLIN      0x0001
#define POLLRDNORM  0x0002
#define POLLRDBAND  0x0004
#define POLLPRI     0x0008
#define POLLOUT     0x0010
#define POLLWRNORM  POLLOUT
#define POLLWRBAND  0x0040
#define POLLERR     0x0080
#define POLLHUP     0x0100
#define POLLNVAL    0x0200

int   poll(struct pollfd [], nfds_t, int);

#endif /* POLL_H */

