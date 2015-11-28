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


#define MAC_ADDR_LEN 6

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
#define LEN_MAGIC 4
#define FLAG_BROADCAST htons(0x8000)
#define XID htonl(0xdeadbee0)

#define DHCP_OPTIONS_LEN_MAX 312 /* including magic */
#define DHCP_MESSAGE_LEN_MAX (DHCP_MESSAGE_HEADER_LEN + DHCP_OPTIONS_LEN_MAX)
#define DHCP_MESSAGE_LEN_MIN (DHCP_MESSAGE_HEADER_LEN + LEN_MAGIC)

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
size_t dhcp_check_reply(const uint8_t *msg, const uint8_t *mac_addr, uint32_t xid)
{
    uint32_t magic = MAGIC;

    if (msg[OFFSET_OP] != BOOTREPLY)
    {
        return OFFSET_OP;
    }
    else if (msg[OFFSET_HTYPE] != 1)
    {
        return OFFSET_HTYPE;
    }
    else if (msg[OFFSET_HLEN] != 6)
    {
        return OFFSET_HLEN;
    }
    else if (memcmp(&msg[OFFSET_XID], &xid, sizeof(uint32_t)) != 0)
    {
        return OFFSET_XID;
    } 
    else if (memcmp(&msg[OFFSET_CHADDR], mac_addr, MAC_ADDR_LEN) != 0)
    {
        return OFFSET_CHADDR;
    }
    else if (memcmp(&msg[OFFSET_OPTIONS], &magic, LEN_MAGIC) != 0)
    {
        return OFFSET_OPTIONS;
    }
    return OFFSET_OPTIONS + LEN_MAGIC;
}

static
int dhcp_parse_options(
        const uint8_t *p_options,
        size_t options_size,
        uint8_t *type,
        struct dhcp_binding *binding,
        in_addr_t *server
        )
{
    const uint8_t * const p_end = p_options + options_size;
    int len_error = 0;

    while((p_options < p_end))
    {
        uint8_t option;

        option = *p_options++;
        if (option == OPT_PAD)
        {
            continue;
        }
        else if (option == OPT_END)
        {
            break;
        }
        else
        {
            uint8_t len;
            uint32_t lease;

            len = *p_options++;
            switch(option)
            {
                case OPT_DHCP_MESSAGE_TYPE:
                    len_error |= (len != 1);
                    *type = *p_options;
                    break;
                case OPT_SERVER_IDENTIFIER:
                    len_error |= (len != 4);
                    memcpy(server, p_options, sizeof(in_addr_t));
                    break;
                case OPT_IP_ADDRESS_LEASE_TIME:
                    len_error |= (len != 4);
                    memcpy(&lease, p_options, sizeof(uint32_t));
                    binding->lease = ntohl(lease);
                    break;
                case OPT_SUBNET:
                    len_error |= (len != 4);
                    memcpy(&binding->subnet, p_options, sizeof(in_addr_t));
                    break;
                case OPT_ROUTER:
                    len_error |= (len != 4);
                    memcpy(&binding->gateway, p_options, sizeof(in_addr_t));
                    break;
                case OPT_DOMAIN_NAME_SERVER:
                    len_error |= ((len % 4) != 0);
                    memcpy(&binding->dns_server, p_options, sizeof(in_addr_t));
                    break;
                default:
                    /* ignore other options */
                    break;
            }
            p_options += len;
        }
    }

    return len_error?-1:0;
}

static
int dhcp_check_offer(
        const uint8_t *p_options,
        size_t options_size,
        struct offer *offer)
{
    int ret;
    int isoffer;
    struct offer o;
    uint8_t type;

    memset(&o, 0, sizeof(struct offer)); /* INADDR_ANY */

    dhcp_parse_options(
            p_options,
            options_size,
            &type,
            &o.binding,
            &o.server);

    isoffer = (type == DHCPOFFER);

    if (!isoffer)
    {
        ret = DHCP_EOFFEREXPECTED;
    }
    if (o.server == INADDR_ANY)
    {
        ret = DHCP_ENOSERVERID;
    }
    else if (o.binding.gateway == INADDR_ANY)
    {
        ret = DHCP_ENOGATEWAY;
    }
    else if (o.binding.subnet == INADDR_ANY)
    {
        ret = DHCP_ENOSUBNET;
    }
    else
    {
        if (o.binding.lease == 0)
        {
            o.binding.lease = 0xFFFFFFFF; /* infinity */
        }
        /* DNS is not necessary */
        *offer = o;
        ret = 0;
    }

    return ret;
}

static
ssize_t dhcp_reply_recv(
        int sock,
        const uint8_t *mac_addr,
        uint32_t xid,
        uint8_t *dhcp_message
        )
{
    int ret;
    ssize_t dhcp_message_size;
    struct sockaddr_in server;
    socklen_t server_addr_len;

    /* TODO: timeout */
    dhcp_message_size = recvfrom(
            sock,
            dhcp_message,
            sizeof(dhcp_message),
            0,
            (struct sockaddr *)&server,
            &server_addr_len);
    if (dhcp_message_size < 0)
    {
        ret = DHCP_ESYSCALL;
    }
    else if (server.sin_port != htons(67))
    {
        ret = 0;
    }
    else if (dhcp_message_size < DHCP_MESSAGE_LEN_MIN)
    {
        ret = 0;
    }
    else if (dhcp_check_reply(dhcp_message, mac_addr, xid) <= OFFSET_OPTIONS)
    {
        ret = 0;
    }
    else
    {
        ret = dhcp_message_size;
    }

    return ret;
}

static
int dhcp_offer_recv(
        int sock,
        const uint8_t *mac_addr,
        uint32_t xid,
        struct offer *offer)
{
    int ret;

    while(1)
    {
        uint8_t dhcp_message[DHCP_MESSAGE_LEN_MAX];
        ssize_t dhcp_message_size;

        /* TODO: timeout */
        dhcp_message_size = dhcp_reply_recv(
                sock,
                mac_addr,
                xid,
                dhcp_message);
        if (dhcp_message_size == 0)
        {
            continue;
        }
        else if (dhcp_message_size < 0)
        {
            ret = dhcp_message_size;
            break;
        }
        else /* It's a DHCP reply for us */
        {
            in_addr_t yiaddr;

            memcpy(&yiaddr, &dhcp_message[OFFSET_YIADDR], sizeof(in_addr_t));
            if ((yiaddr == INADDR_ANY) || (yiaddr == INADDR_BROADCAST))
            {
                ret = DHCP_EYIADDR;
            }
            else
            {
                ret = dhcp_check_offer(
                        &dhcp_message[DHCP_MESSAGE_HEADER_LEN + LEN_MAGIC],
                        dhcp_message_size - DHCP_MESSAGE_HEADER_LEN - LEN_MAGIC,
                        offer);
                if (ret == 0)
                {
                    offer->binding.client = yiaddr;
                }
            }
            break;
        }
    }
    return ret;
}

static
uint8_t *dhcp_prepare_bootp(
        uint8_t *dhcp_message,
        const uint8_t *mac_addr,
        uint32_t *xid
        )
{
    /* Construct BOOTP header */
    dhcp_message[OFFSET_OP] = BOOTREQUEST;
    dhcp_message[OFFSET_HTYPE] = 1;
    dhcp_message[OFFSET_HLEN] = 6;
    dhcp_message[OFFSET_HOPS] = 0;
    *xid = XID; /* TODO: random */
    setfield32(&dhcp_message[OFFSET_XID], *xid);
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
    setfield32(&dhcp_message[OFFSET_OPTIONS], MAGIC);

    return &dhcp_message[OFFSET_OPTIONS + LEN_MAGIC];
}

static
int dhcp_send(
        int sock,
        const uint8_t *dhcp_message,
        size_t dhcp_message_size
        )
{
    struct sockaddr_in server;

    /* destination is 255.255.255.255:67 broadcast address */
    server.sin_addr.s_addr = INADDR_BROADCAST;
    server.sin_family = AF_INET;
    server.sin_port = htons(67);

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
uint8_t *dhcp_append_common_options(uint8_t *p_options, uint8_t type)
{
    *p_options++ = OPT_DHCP_MESSAGE_TYPE;
    *p_options++ = 1;
    *p_options++ = type;

    *p_options++ = OPT_PARAMETER_REQUEST_LIST;
    *p_options++ = 3;
    *p_options++ = OPT_ROUTER;
    *p_options++ = OPT_SUBNET;
    *p_options++ = OPT_DOMAIN_NAME_SERVER;

    return p_options;
}

static
int dhcp_discover_send(
        int sock,
        const uint8_t *mac_addr,
        uint32_t *xid
        )
{
    uint8_t dhcp_message[DHCP_MESSAGE_LEN_MAX];
    uint8_t *p_options;
    
    p_options = dhcp_prepare_bootp(dhcp_message, mac_addr, xid);

    /* Append DHCP options */
    p_options = dhcp_append_common_options(p_options, DHCPDISCOVER);
    
    *p_options++ = OPT_END;
    memset(p_options, 0, sizeof(dhcp_message) - (p_options - dhcp_message));

    return dhcp_send(sock, dhcp_message, sizeof(dhcp_message));
}

static
int dhcp_socket_create(void)
{
    int sock;
    struct sockaddr_in client;

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

    return sock;
}

static
int dhcp_discover(
        const uint8_t *mac_addr,
        struct offer *offer)
{
    int attempt;
    int ret;
    int sock;
   
    sock = dhcp_socket_create();
    if (sock < 0)
    {
        return sock;
    }

    for (attempt = 0; attempt < DHCP_DISCOVER_RETRIES; attempt++)
    {
        uint32_t xid;

        ret = dhcp_discover_send(sock, mac_addr, &xid);
        if (ret != 0)
        {
            continue;/* retry */
        }
        ret = dhcp_offer_recv(sock, mac_addr, xid, offer);
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
int dhcp_request_send(
        int sock,
        const uint8_t *mac_addr,
        const struct offer *offer,
        uint32_t *xid)
{
    uint8_t dhcp_message[DHCP_MESSAGE_LEN_MAX];
    uint8_t *p_options;
    
    p_options = dhcp_prepare_bootp(dhcp_message, mac_addr, xid);

    /* Append DHCP options */
    p_options = dhcp_append_common_options(p_options, DHCPREQUEST);
    
    *p_options++ = OPT_SERVER_IDENTIFIER;
    *p_options++ = 4;
    memcpy(p_options, &offer->server, 4);
    p_options += 4;

    *p_options++ = OPT_REQUESTED_IP_ADDRESS;
    *p_options++ = 4;
    memcpy(p_options, &offer->binding.client, 4);
    p_options += 4;

    *p_options++ = OPT_END;
    memset(p_options, 0, sizeof(dhcp_message) - (p_options - dhcp_message));

    return dhcp_send(sock, dhcp_message, sizeof(dhcp_message));
}

static
int dhcp_ack_check(
        const uint8_t *p_options,
        size_t options_size,
        struct ack *ack)
{
    int ret;
    int isoffer;
    struct ack a;
    uint8_t type;

    memset(&a, 0, sizeof(struct offer)); /* INADDR_ANY */

    dhcp_parse_options(
            p_options,
            options_size,
            &type,
            &a.binding,
            &a.server);

    isoffer = (type == DHCPACK);

    if (!isoffer)
    {
        ret = DHCP_EOFFEREXPECTED;
    }
    if (a.server == INADDR_ANY)
    {
        ret = DHCP_ENOSERVERID;
    }
    else if (a.binding.gateway == INADDR_ANY)
    {
        ret = DHCP_ENOGATEWAY;
    }
    else if (a.binding.subnet == INADDR_ANY)
    {
        ret = DHCP_ENOSUBNET;
    }
    else
    {
        if (a.binding.lease == 0)
        {
            a.binding.lease = 0xFFFFFFFF; /* infinity */
        }
        /* DNS is not necessary */
        *ack = a;
        ret = 0;
    }

    return ret;
}

static
int dhcp_ack_recv(
        int sock,
        const uint8_t *mac_addr,
        uint32_t xid,
        const struct offer *offer,
        struct ack *ack)
{
    int ret;

    while(1)
    {
        uint8_t dhcp_message[DHCP_MESSAGE_LEN_MAX];
        ssize_t dhcp_message_size;

        /* TODO: timeout */
        dhcp_message_size = dhcp_reply_recv(
                sock,
                mac_addr,
                xid,
                dhcp_message);
        if (dhcp_message_size == 0)
        {
            continue;
        }
        else if (dhcp_message_size < 0)
        {
            ret = dhcp_message_size;
            break;
        }
        else /* It's a DHCP reply for us */
        {
            in_addr_t yiaddr;

            memcpy(&yiaddr, &dhcp_message[OFFSET_YIADDR], sizeof(in_addr_t));
            if ((yiaddr == INADDR_ANY) || (yiaddr == INADDR_BROADCAST))
            {
                ret = DHCP_EYIADDR;
            }
            else
            {
                ret = dhcp_ack_check(
                        &dhcp_message[DHCP_MESSAGE_HEADER_LEN + LEN_MAGIC],
                        dhcp_message_size - DHCP_MESSAGE_HEADER_LEN - LEN_MAGIC,
                        ack);
                (void)offer; /* TODO: check that ack has same params */
                if (ret == 0)
                {
                    ack->binding.client = yiaddr;
                }
            }
            break;
        }
    }
    return ret;
}

static
int dhcp_request(
        const uint8_t *mac_addr,
        const struct offer *offer,
        struct ack *ack)
{
    int attempt;
    int ret;
    int sock;
   
    sock = dhcp_socket_create();
    if (sock < 0)
    {
        return sock;
    }

    for (attempt = 0; attempt < DHCP_REQUEST_RETRIES; attempt++)
    {
        uint32_t xid;

        ret = dhcp_request_send(sock, mac_addr, offer, &xid);
        if (ret != 0)
        {
            continue;/* retry */
        }
        ret = dhcp_ack_recv(sock, mac_addr, xid, offer, ack);
        if (ret != 0)
        {
            continue;/* retry */
        }
        break;
    }
    close(sock);
    return ret;
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

