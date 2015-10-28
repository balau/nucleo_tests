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

/* http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_socket.h.html */

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

#define AF_INET   0x1 /* Internet domain sockets for use with IPv4 addresses. */
#define AF_INET6  0x2 /* Internet domain sockets for use with IPv6 addresses. */
#define AF_UNIX   0x3 /* UNIX domain sockets. */
#define AF_UNSPEC 0x4 /* Unspecified. */

#define SOCK_STREAM    0x1 /* Byte-stream socket. */
#define SOCK_DGRAM     0x2 /* Datagram socket. */
#define SOCK_RAW       0x3 /* Raw Protocol Interface. */
#define SOCK_SEQPACKET 0x4 /* Sequenced-packet socket. */

#define MSG_CTRUNC    0x01 /* Control data truncated. */
#define MSG_DONTROUTE 0x02 /* Send without using routing tables. */
#define MSG_EOR       0x04 /* Terminates a record (if supported by the protocol). */
#define MSG_OOB       0x08 /* Out-of-band data. */
#define MSG_PEEK      0x10 /* Leave received data in queue. */
#define MSG_TRUNC     0x20 /* Normal data truncated. */
#define MSG_WAITALL   0x40 /* Attempt to fill the read buffer. */

#define SO_ACCEPTCONN 0x00 /* Socket is accepting connections. */
#define SO_BROADCAST 0x00 /* Transmission of broadcast messages is supported. */
#define SO_DEBUG 0x00 /* Debugging information is being recorded. */
#define SO_DONTROUTE 0x00 /* Bypass normal routing. */
#define SO_ERROR 0x00 /* Socket error status. */
#define SO_KEEPALIVE 0x00 /* Connections are kept alive with periodic messages. */
#define SO_LINGER 0x00 /* Socket lingers on close. */
#define SO_OOBINLINE 0x00 /* Out-of-band data is transmitted in line. */
#define SO_RCVBUF 0x00 /* Receive buffer size. */
#define SO_RCVLOWAT 0x00 /* Receive ``low water mark''. */
#define SO_RCVTIMEO 0x00 /* Receive timeout. */
#define SO_REUSEADDR 0x00 /* Reuse of local addresses is supported. */
#define SO_SNDBUF 0x00 /* Send buffer size. */
#define SO_SNDLOWAT 0x00 /* Send ``low water mark''. */
#define SO_SNDTIMEO 0x00 /* Send timeout. */
#define SO_TYPE 0x00 /* Socket type. */

#define SOL_SOCKET 0xFF /* Options to be accessed at socket level, not protocol level. */

#define SOMAXCONN 1 /* The maximum backlog queue length. */

#define SHUT_RD   0x1 /* Disables further receive operations. */
#define SHUT_WR   0x2 /* Disables further send and receive operations. */
#define SHUT_RDWR 0x3 /* Disables further send operations. */

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
