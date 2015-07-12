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
#ifndef SOCKET_H
#define SOCKET_H

/* http://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/socket.h.html */

#include <stdint.h>
#include <sys/types.h>

typedef int32_t socklen_t;

typedef unsigned int sa_family_t;

struct sockaddr
{
    sa_family_t sa_family;
    char sa_data[6];
};

struct sockaddr_storage
{
    sa_family_t sa_family;
    char sa_data[6];
};

#define AF_INET 0x1
#define AF_INET6 0x2
#define AF_UNIX 0x3
#define AF_UNSPEC 0x4

#define SOCK_STREAM    0x1
#define SOCK_DGRAM     0x2
#define SOCK_RAW       0x3
#define SOCK_SEQPACKET 0x4
#define SOCK_NONBLOCK 0x8

#define SOMAXCONN 1

#define SHUT_RD   0x1
#define SHUT_WR   0x2
#define SHUT_RDWR 0x3

extern
int     accept(int, struct sockaddr *__restrict, socklen_t *__restrict);

extern
int     bind(int, const struct sockaddr *, socklen_t);

extern
int     connect(int, const struct sockaddr *, socklen_t);

extern
int     getpeername(int, struct sockaddr *__restrict, socklen_t *__restrict);

extern
int     getsockname(int, struct sockaddr *__restrict, socklen_t *__restrict);

#if 0
extern
int     getsockopt(int, int, int, void *__restrict, socklen_t *__restrict);
#endif

extern
int     listen(int, int);

extern
ssize_t recv(int, void *, size_t, int);

extern
ssize_t recvfrom(int, void *__restrict, size_t, int,
        struct sockaddr *__restrict, socklen_t *__restrict);

#if 0
extern
ssize_t recvmsg(int, struct msghdr *, int);
#endif

extern
ssize_t send(int, const void *, size_t, int);

#if 0
extern
ssize_t sendmsg(int, const struct msghdr *, int);
#endif

extern
ssize_t sendto(int, const void *, size_t, int, const struct sockaddr *,
        socklen_t);
#if 0
extern
int     setsockopt(int, int, int, const void *, socklen_t);
#endif

extern
int     shutdown(int, int);

extern
int     socket(int, int, int);

#if 0
extern
int     sockatmark(int);

extern
int     socketpair(int, int, int, int[2]);
#endif

#endif
