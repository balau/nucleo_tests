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
#ifndef IN_H
#define IN_H

/* http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/netinet_in.h.html */

#include <inttypes.h>
#include "socket.h" // <sys/socket.h>
#include "inet.h" // <arpa/inet.h>

typedef uint16_t in_port_t;

typedef uint32_t in_addr_t;

struct in_addr {
    in_addr_t  s_addr;
};

struct sockaddr_in {
    sa_family_t     sin_family; /* AF_INET. */
    in_port_t       sin_port; /* Port number. */
    struct in_addr  sin_addr; /* IP address. */
};

/* Missing IPv6 stuff */

#define IPPROTO_IP 1 /* Internet protocol. */
#define IPPROTO_IPV6 2 /* [IP6]  Internet Protocol Version 6. */
#define IPPROTO_ICMP 3 /* Control message protocol. */
#define IPPROTO_RAW 4 /* [RS]  Raw IP Packets Protocol. */
#define IPPROTO_TCP 5 /* Transmission control protocol. */
#define IPPROTO_UDP 6 /* User datagram protocol. */

#define INADDR_ANY ((in_addr_t) 0x00000000) /* IPv4 local host address. */
#define INADDR_BROADCAST ((in_addr_t) 0xffffffff) /* IPv4 broadcast address */

#define INET_ADDRSTRLEN 16 /* Length of the string form for IP. */
#define INET6_ADDRSTRLEN 46 /* Length of the string form for IPv6. */

/* Missing IPv6 stuff */

#endif /* IN_H */

