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
#include "dhcp_client.h"
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

//#define DHCP_DEBUG

#ifndef DHCP_ALLOCATE_RETRIES
#  define DHCP_ALLOCATE_RETRIES 3
#endif

#ifndef DHCP_DISCOVER_RETRIES
#  define DHCP_DISCOVER_RETRIES 3
#endif

#ifndef DHCP_REQUEST_RETRIES
#  define DHCP_REQUEST_RETRIES 3
#endif

/* A drawing of the format of BOOTP message is in RFC 1542 */


#define MAC_ADDR_LEN 8

/* OP */
#define BOOTREQUEST 1
#define BOOTREPLY   2

/* Message field offsets */
#define OFFSET_OP        0
#define OFFSET_HTYPE     1
#define OFFSET_HLEN      2
#define OFFSET_HOPS      3
#define OFFSET_XID       4
#define OFFSET_SECS      8
#define OFFSET_FLAGS    10
#define OFFSET_CIADDR   12
#define OFFSET_YIADDR   16
#define OFFSET_SIADDR   20
#define OFFSET_GIADDR   24
#define OFFSET_CHADDR   28
#define OFFSET_SNAME    44
#define OFFSET_FILE    108
#define OFFSET_OPTIONS 236 

/* Message field lengths */
#define LEN_OP        1
#define LEN_HTYPE     1
#define LEN_HLEN      1
#define LEN_HOPS      1
#define LEN_XID       4
#define LEN_SECS      2
#define LEN_FLAGS     2
#define LEN_CIADDR    4
#define LEN_YIADDR    4
#define LEN_SIADDR    4
#define LEN_GIADDR    4
#define LEN_CHADDR   16
#define LEN_SNAME    64
#define LEN_FILE    128

#define DHCP_MESSAGE_HEADER_LEN ( \
        LEN_OP + LEN_HTYPE + LEN_HLEN + LEN_HOPS + \
        LEN_XID + \
        LEN_SECS + LEN_FLAGS + \
        LEN_CIADDR + \
        LEN_YIADDR + \
        LEN_SIADDR + \
        LEN_GIADDR + \
        LEN_CHADDR + \
        LEN_SNAME + \
        LEN_FILE) /* 236 excluding magic */

/* DHCP-related Vendor extensions as for RFC 2132 */
#define OPT_PAD 0
#define OPT_END 255
#define OPT_SUBNET 1
#define OPT_ROUTER 3
#define OPT_DOMAIN_NAME_SERVER 6
#define OPT_REQUESTED_IP_ADDRESS 50
#define OPT_IP_ADDRESS_LEASE_TIME 51
#define OPT_DHCP_MESSAGE_TYPE 53
#define OPT_SERVER_IDENTIFIER 54
#define OPT_PARAMETER_REQUEST_LIST 55
#define OPT_MAXIMUM_DHCP_MESSAGE_SIZE 57

/* DHCP Message Type */
#define DHCPDISCOVER 1
#define DHCPOFFER    2
#define DHCPREQUEST  3
#define DHCPDECLINE  4
#define DHCPACK      5
#define DHCPNAK      6
#define DHCPRELEASE  7
#define DHCPINFORM   8

#define MAGIC htonl(0x63825363)
#define FLAG_BROADCAST htons(0x8000)

#define DHCP_OPTIONS_LEN_MAX (4 + 64) /* including magic */
#define DHCP_MESSAGE_LEN_MAX (DHCP_MESSAGE_HEADER_LEN + DHCP_OPTIONS_LEN_MAX)

struct offer
{
    in_addr_t server;
    struct dhcp_binding binding;
};

struct ack
{
    uint8_t message_type;
    in_addr_t server;
    struct dhcp_binding binding;
};

static
uint8_t *setfield16(uint8_t *field, uint16_t val)
{
    memcpy(field, &val, sizeof(uint16_t));
    return field + sizeof(uint16_t);
}

static
uint8_t *setfield32(uint8_t *field, uint32_t val)
{
    memcpy(field, &val, sizeof(uint32_t));
    return field + sizeof(uint32_t);
}

static
int dhcp_offer_recv(
        int sock,
        const uint8_t *mac_addr,
        struct offer *offer)
{
    return DHCP_EINTERNAL;
}

static
int dhcp_discover_send(
        int sock,
        const uint8_t *mac_addr)
{
    struct sockaddr_in server;
    uint8_t dhcp_message[DHCP_MESSAGE_LEN_MAX];
    ssize_t dhcp_message_size;
    uint32_t xid;
    uint8_t *p_options;
    
    /* destination is 255.255.255.255:67 broadcast address */
    server.sin_addr.s_addr = INADDR_BROADCAST;
    server.sin_family = AF_INET;
    server.sin_port = htons(67);

    /* Construct BOOTP header */
    dhcp_message[OFFSET_OP] = BOOTREQUEST;
    dhcp_message[OFFSET_HTYPE] = 1;
    dhcp_message[OFFSET_HLEN] = 6;
    dhcp_message[OFFSET_HOPS] = 0;
    xid = htonl(0xdeadbee0); /* TODO: random */
    setfield32(&dhcp_message[OFFSET_XID], xid);
    setfield16(&dhcp_message[OFFSET_SECS], 0);
    setfield16(&dhcp_message[OFFSET_FLAGS], FLAG_BROADCAST);
    /* ciaddr = 0 */
    /* yiaddr = 0 */
    /* siaddr = 0 */
    /* giaddr = 0 */
    memset(
            &dhcp_message[OFFSET_CIADDR],
            0,
            LEN_CIADDR + LEN_YIADDR + LEN_SIADDR + LEN_GIADDR
            );
    /* MAC address */
    memcpy(&dhcp_message[OFFSET_CHADDR], mac_addr, MAC_ADDR_LEN);
    /* last part of MAC address, sname and file set to 0 */
    memset(
            &dhcp_message[OFFSET_CHADDR + MAC_ADDR_LEN],
            0,
            (LEN_CHADDR - MAC_ADDR_LEN) + LEN_SNAME + LEN_FILE
            );

    /* Append DHCP options */
    p_options = &dhcp_message[OFFSET_OPTIONS];
    p_options = setfield32(p_options, MAGIC);

    *p_options++ = OPT_DHCP_MESSAGE_TYPE;
    *p_options++ = 1;
    *p_options++ = DHCPDISCOVER;
    
    *p_options++ = OPT_END;

    dhcp_message_size = p_options - dhcp_message;

    if (sendto(
            sock,
            dhcp_message,
            dhcp_message_size,
            0,
            (struct sockaddr *)&server,
            sizeof(server)
            ) < 0)
    {
        return DHCP_ESYSCALL;
    }
    return 0;
}

static
int dhcp_discover(
        const uint8_t *mac_addr,
        struct offer *offer)
{
    int sock;
    struct sockaddr_in client;
    int attempt;
    int ret;

    /* create UDP socket */
    sock = socket(AF_INET , SOCK_DGRAM , 0);
    if (sock == -1)
    {
        return DHCP_ESYSCALL;
    }

    /* TODO: set socket broadcast option */

    /* bind socket with 0.0.0.0:68 client address */
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_family = AF_INET;
    client.sin_port = htons(68);
    if (bind(sock, (struct sockaddr *)&client, sizeof(client)) == -1)
    {
        close(sock);
        return DHCP_ESYSCALL;
    }
    
    for (attempt = 0; attempt < DHCP_DISCOVER_RETRIES; attempt++)
    {
        ret = dhcp_discover_send(sock, mac_addr);
        if (ret != 0)
        {
            continue;/* retry */
        }
        ret = dhcp_offer_recv(sock, mac_addr, offer);
        if (ret != 0)
        {
            continue;/* retry */
        }
        break;
    }
    close(sock);
    return ret;
}

static
int dhcp_request(
        const uint8_t *mac_addr,
        const struct offer *offer,
        struct ack *ack)
{
    return DHCP_EINTERNAL;
}

int dhcp_allocate(const uint8_t *mac_addr, struct dhcp_binding *binding)
{
    int ret;
    struct offer offer;
    struct ack ack;
    int attempt;

    for (attempt = 0; attempt < DHCP_ALLOCATE_RETRIES; attempt++)
    {
        ret = dhcp_discover(mac_addr, &offer);
        if (ret != 0)
        {
            continue; /* retry */
        }
        ret = dhcp_request(mac_addr, &offer, &ack);
        if (ret != 0)
        {
            continue; /* retry */
        }
        *binding = ack.binding;
        ret = 0;
        break;
    }
    return ret;
}

