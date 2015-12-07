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
#ifndef NETDB_H
#define NETDB_H

#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct hostent {
    char   *h_name;
    char  **h_aliases;
    int     h_addrtype;
    int     h_length;
    char  **h_addr_list;
};

struct netent {
    char     *n_name;
    char    **n_aliases;
    int       n_addrtype;
    uint32_t  n_net;
};

struct protoent {
    char   *p_name;
    char  **p_aliases;
    int     p_proto;
};

struct servent {
    char   *s_name;
    char  **s_aliases;
    int     s_port;
    char   *s_proto;
};

#define IPPORT_RESERVED 1024

struct addrinfo {
    int               ai_flags;
    int               ai_family;
    int               ai_socktype;
    int               ai_protocol;
    socklen_t         ai_addrlen;
    struct sockaddr  *ai_addr;
    char             *ai_canonname;
    struct addrinfo  *ai_next;
};

#define AI_PASSIVE     0x01
#define AI_CANONNAME   0x02
#define AI_NUMERICHOST 0x04
#define AI_NUMERICSERV 0x08
#define AI_V4MAPPED    0x10
#define AI_ALL         0x20
#define AI_ADDRCONFIG  0x40

#define NI_NOFQDN       0x01
#define NI_NUMERICHOST  0x02
#define NI_NAMEREQD     0x04
#define NI_NUMERICSERV  0x08
#define NI_NUMERICSCOPE 0x10
#define NI_DGRAM        0x20

#define EAI_AGAIN    1
#define EAI_BADFLAGS 2
#define EAI_FAIL     3
#define EAI_FAMILY   4
#define EAI_MEMORY   5
#define EAI_NONAME   6

void endhostent(void);

void endnetent(void);

void endprotoent(void);

void endservent(void);

void freeaddrinfo(struct addrinfo *);

const char *gai_strerror(int);

int getaddrinfo(
        const char *__restrict,
        const char *__restrict,
        const struct addrinfo *__restrict,
        struct addrinfo **__restrict);

struct hostent *gethostent(void);

int getnameinfo(
        const struct sockaddr *__restrict,
        socklen_t,
        char *__restrict,
        socklen_t,
        char *__restrict,
        socklen_t,
        int);

struct netent *getnetbyaddr(uint32_t, int);

struct netent *getnetbyname(const char *);

struct netent *getnetent(void);

struct protoent *getprotobyname(const char *);

struct protoent *getprotobynumber(int);

struct protoent *getprotoent(void);

struct servent *getservbyname(const char *, const char *);

struct servent *getservbyport(int, const char *);

struct servent *getservent(void);

void sethostent(int);

void setnetent(int);

void setprotoent(int);

void setservent(int);

#endif /* NETDB_H */

