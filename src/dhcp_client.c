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
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "timespec.h"

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
size_t bootp_check_reply(const uint8_t *msg, const uint8_t *mac_addr, uint32_t xid)
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
    else if (msg[OFFSET_HLEN] != DHCP_MAC_ADDR_LEN)
    {
        return OFFSET_HLEN;
    }
    else if (memcmp(&msg[OFFSET_XID], &xid, sizeof(uint32_t)) != 0)
    {
        return OFFSET_XID;
    } 
    else if (memcmp(&msg[OFFSET_CHADDR], mac_addr, DHCP_MAC_ADDR_LEN) != 0)
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
        struct dhcp_binding *binding
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
                    memcpy(&binding->dhcp_server, p_options, sizeof(in_addr_t));
                    break;
                case OPT_IP_ADDRESS_LEASE_TIME:
                    len_error |= (len != 4);
                    memcpy(&lease, p_options, sizeof(uint32_t));
                    lease = ntohl(lease);
                    if (lease >= INT_MAX)
                    {
                        binding->lease_t1 = TIMESPEC_INFINITY;
                        binding->lease_t2 = TIMESPEC_INFINITY;
                    }
                    else
                    {
                        struct timespec t1;
                        struct timespec t2;
                        struct timespec now;

                        t1.tv_sec = lease/2;
                        t1.tv_nsec = 0;
                        t2.tv_sec = lease - (lease/8);
                        t2.tv_nsec = 0;
                        clock_gettime(CLOCK_REALTIME, &now);
                        timespec_add(&now, &t1, &binding->lease_t1);
                        timespec_add(&now, &t2, &binding->lease_t2);
                    }
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
int dhcp_check_options(
        struct dhcp_binding *options)
{
    int ret;

    if (options->dhcp_server == INADDR_ANY)
    {
        ret = DHCP_ENOSERVERID;
    }
    else if (options->gateway == INADDR_ANY)
    {
        ret = DHCP_ENOGATEWAY;
    }
    else if (options->subnet == INADDR_ANY)
    {
        ret = DHCP_ENOSUBNET;
    }
    else
    {
        if (timespec_diff(&TIMESPEC_ZERO, &options->lease_t1, NULL) == 0)
        {
            options->lease_t1 = TIMESPEC_INFINITY;
        }
        if (timespec_diff(&TIMESPEC_ZERO, &options->lease_t2, NULL) == 0)
        {
            options->lease_t2 = TIMESPEC_INFINITY;
        }
        ret = 0;
    }

    return ret;
}

static
uint32_t gen_xid(const uint8_t *mac_addr)
{
    int i;
    uint32_t xid = XID;

    for (i = 0; i < DHCP_MAC_ADDR_LEN; i++)
    {
        xid *= mac_addr[i];
        xid += mac_addr[i];
    }
    xid ^= rand();

    return xid;
}

static
uint8_t *fill_bootp_request(uint8_t *dhcp_message)
{
    /* Construct BOOTP header */
    dhcp_message[OFFSET_OP] = BOOTREQUEST;
    dhcp_message[OFFSET_HTYPE] = 1;
    dhcp_message[OFFSET_HLEN] = DHCP_MAC_ADDR_LEN;
    dhcp_message[OFFSET_HOPS] = 0;
    setfield16(&dhcp_message[OFFSET_SECS], 0);
    setfield16(&dhcp_message[OFFSET_FLAGS], FLAG_BROADCAST);
    setfield32(&dhcp_message[OFFSET_YIADDR], 0);
    setfield32(&dhcp_message[OFFSET_SIADDR], 0);
    setfield32(&dhcp_message[OFFSET_GIADDR], 0);
    /* last part of MAC address, sname and file set to 0 */
    memset(
            &dhcp_message[OFFSET_CHADDR + DHCP_MAC_ADDR_LEN],
            0,
            (LEN_CHADDR - DHCP_MAC_ADDR_LEN) + LEN_SNAME + LEN_FILE
            );
    setfield32(&dhcp_message[OFFSET_OPTIONS], MAGIC);

    return &dhcp_message[OFFSET_OPTIONS + LEN_MAGIC];
}

static
uint8_t *dhcp_append_option32(uint8_t *p_options, uint8_t opt, uint32_t val)
{
    *p_options++ = opt;
    *p_options++ = 4;
    memcpy(p_options, &val, 4);
    p_options += 4;

    return p_options;
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

    p_options = dhcp_append_option32(p_options, OPT_IP_ADDRESS_LEASE_TIME, 0xFFFFFFFF);

    return p_options;
}

static
void dhcp_update_state_noresp(struct dhcp_binding *binding)
{
    enum dhcp_state newstate = binding->state;

    newstate = DHCP_INIT;

    if (newstate != binding->state)
    {
        binding->state = newstate;
    }
}

static
void dhcp_update_state(struct dhcp_binding *binding, uint8_t dhcp_message_type)
{
    enum dhcp_state newstate = binding->state;

    switch(binding->state)
    {
        case DHCP_INIT:
            if (dhcp_message_type == DHCPDISCOVER)
            {
                newstate = DHCP_SELECTING;
            }
            break;
        case DHCP_SELECTING:
            if (dhcp_message_type == DHCPREQUEST)
            {
                newstate = DHCP_REQUESTING;
            }
            break;
        case DHCP_REQUESTING:
        case DHCP_REBINDING:
        case DHCP_RENEWING:
        case DHCP_REBOOTING:
            if (dhcp_message_type == DHCPACK)
            {
                newstate = DHCP_BOUND;
            }
            else if (dhcp_message_type == DHCPNAK)
            {
                newstate = DHCP_INIT;
            }
            break;
        case DHCP_BOUND:
            if (dhcp_message_type == DHCPREQUEST)
            {
                newstate = DHCP_RENEWING;
            }
            break;
        case DHCP_INIT_REBOOT:
            if (dhcp_message_type == DHCPREQUEST)
            {
                newstate = DHCP_REBOOTING;
            }
            break;
        default:
            /* TODO: manage error */
            break;
    }
    if (newstate != binding->state)
    {
        binding->state = newstate;
    }
}

static
int send_bootp_request(int sock, struct dhcp_binding *binding)
{
    int ret;
    uint8_t dhcp_message[DHCP_MESSAGE_LEN_MAX];
    uint8_t *p_options;
    uint8_t type;
    uint32_t ciaddr;
    
    if (binding->state != DHCP_SELECTING)
    {
        binding->xid = gen_xid(binding->mac_addr);
    }
    setfield32(&dhcp_message[OFFSET_XID], binding->xid);
    if (
            (binding->state == DHCP_BOUND) ||
            (binding->state == DHCP_RENEWING) ||
            (binding->state == DHCP_REBINDING)
       )
    {
        ciaddr = binding->client;
    }
    else
    {
        ciaddr = 0;
    }
    setfield32(&dhcp_message[OFFSET_CIADDR], ciaddr);
    memcpy(&dhcp_message[OFFSET_CHADDR], binding->mac_addr, DHCP_MAC_ADDR_LEN);

    p_options = fill_bootp_request(dhcp_message);

    /* Append DHCP options */
    switch (binding->state)
    {
        case DHCP_INIT:
            type = DHCPDISCOVER;
            break;
        case DHCP_SELECTING:
        case DHCP_BOUND:
        case DHCP_RENEWING:
            type = DHCPREQUEST;
            break;
        default:
            /* should never get here TODO: manage error */
            type = DHCPREQUEST;
            break;
    }
    p_options = dhcp_append_common_options(p_options, type);
    
    if (binding->state == DHCP_SELECTING)
    {
        p_options = dhcp_append_option32(p_options, OPT_SERVER_IDENTIFIER, binding->dhcp_server);
    }

    if (binding->state == DHCP_SELECTING)
    {
        p_options = dhcp_append_option32(p_options, OPT_REQUESTED_IP_ADDRESS, binding->client);
    }

    *p_options++ = OPT_END;
    memset(p_options, 0, sizeof(dhcp_message) - (p_options - dhcp_message));

    if (send(sock, dhcp_message, sizeof(dhcp_message), 0) < 0)
    {
        ret = DHCP_ESYSCALL;
    }
    else
    {
        dhcp_update_state(binding, type);
        ret = 0;
    }

    return ret;
}

static
int recv_bootp_reply(int sock, struct dhcp_binding *binding)
{
    int ret;

    while(1) /* TODO: timeout */
    {
        uint8_t dhcp_message[DHCP_MESSAGE_LEN_MAX];
        ssize_t dhcp_message_size;
        struct sockaddr_in server;
        socklen_t server_addr_len;

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
            break;
        }
        else if (dhcp_message_size == 0)
        {
            continue;
        }
        else if (bootp_check_reply(dhcp_message, binding->mac_addr, binding->xid) <= OFFSET_OPTIONS)
        {
            continue;
        }
        else /* It's a DHCP reply for us */
        {
            in_addr_t yiaddr;
            struct dhcp_binding options;
            uint8_t type = 0; /* default no type */
            int len_error;

            memset(&options, 0, sizeof(struct dhcp_binding));
            len_error = dhcp_parse_options(
                    &dhcp_message[OFFSET_OPTIONS + LEN_MAGIC],
                    dhcp_message_size - (OFFSET_OPTIONS + LEN_MAGIC),
                    &type,
                    &options);
            if (len_error == -1)
            {
                continue; /* malformed answer */
            }
            memcpy(&yiaddr, &dhcp_message[OFFSET_YIADDR], sizeof(in_addr_t));

            if ((type == DHCPOFFER) && (binding->state == DHCP_SELECTING))
            {
                ret = dhcp_check_options(&options);
                if (ret == 0)
                {
                    binding->client = yiaddr;
                    binding->dhcp_server = options.dhcp_server;
                }
            }
            else if (
                    (binding->state == DHCP_REQUESTING) ||
                    (binding->state == DHCP_RENEWING) ||
                    (binding->state == DHCP_REBINDING) ||
                    (binding->state == DHCP_REBOOTING)
                    )
            {
                if (type == DHCPACK)
                {
                    ret = dhcp_check_options(&options);
                    binding->client = yiaddr;
                    binding->dhcp_server = options.dhcp_server;
                    binding->gateway = options.gateway;
                    binding->subnet = options.subnet;
                    binding->dns_server = options.dns_server;
                    binding->lease_t1 = options.lease_t1;
                    binding->lease_t2 = options.lease_t2;
                }
                else if (type == DHCPNAK)
                {
                    binding->client = INADDR_ANY;
                    binding->dhcp_server = INADDR_ANY;
                    ret = 0;
                }
                else
                {
                    ret = 0; /* ignore others */
                }
            }
            else
            {
                ret = 0; /* ignore others */
            }
            if (ret == 0)
            {
                dhcp_update_state(binding, type);
            }
            break;
        }
    }
    return ret;
}

static
int bootp_socket_create(struct dhcp_binding *binding)
{
    int sock;
    struct sockaddr_in client;
    struct sockaddr_in server;
    struct timespec timeout = {1, 0}; /* 1s */

    /* create UDP socket */
    sock = socket(AF_INET , SOCK_DGRAM , 0);
    if (sock == -1)
    {
        return DHCP_ESYSCALL;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timespec)) == -1)
    {
        close(sock);
        return DHCP_ESYSCALL;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timespec)) == -1)
    {
        close(sock);
        return DHCP_ESYSCALL;
    }

    client.sin_addr.s_addr = binding->client;
    client.sin_family = AF_INET;
    client.sin_port = htons(68);
    if (bind(sock, (struct sockaddr *)&client, sizeof(client)) == -1)
    {
        close(sock);
        return DHCP_ESYSCALL;
    }
    if (binding->state == DHCP_BOUND)
    {
        server.sin_addr.s_addr = binding->dhcp_server;
    }
    else
    {
        int can_broadcast;

        server.sin_addr.s_addr = INADDR_BROADCAST;
        can_broadcast = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &can_broadcast, sizeof(int)) == -1)
        {
            close(sock);
            return DHCP_ESYSCALL;
        }
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(67);
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        close(sock);
        return DHCP_ESYSCALL;
    }

    return sock;
}

static
int bootp_transaction(struct dhcp_binding *binding)
{
    int attempt;
    int ret;
    int sock;

    sock = bootp_socket_create(binding);
    if (sock < 0)
    {
        return sock;
    }

    for (attempt = 0; attempt < DHCP_DISCOVER_RETRIES; attempt++)
    {
        ret = send_bootp_request(sock, binding);
        if (ret != 0)
        {
            continue;/* retry */
        }
        ret = recv_bootp_reply(sock, binding);
        if (ret != 0)
        {
            /* TODO: random backoff increasing with attempt */
            dhcp_update_state_noresp(binding);
            continue;/* retry */
        }
        break;
    }
    close(sock);
    return ret;
}

static
time_t get_dhcp_next_event(const struct dhcp_binding *binding)
{
    time_t next = 0;

    if (binding->state == DHCP_BOUND || binding->state == DHCP_RENEWING)
    {
        struct timespec now;
        struct timespec diff_t1;
        struct timespec diff_t2;
        int expired_t1;
        int expired_t2;

        clock_gettime(CLOCK_REALTIME, &now);
        expired_t1 = (timespec_diff(&binding->lease_t1, &now, &diff_t1) <= 0);
        expired_t2 = (timespec_diff(&binding->lease_t2, &now, &diff_t2) <= 0);

        if (binding->state == DHCP_BOUND)
        {
            if (expired_t1)
            {
                /* expired_t2 ==> expired_t1 */
                next = 0;
            }
            else
            {
                next = diff_t1.tv_sec; /* wait for T1 */
            }
        }
        else if (binding->state == DHCP_RENEWING)
        {
            if (expired_t2)
            {
                next = 0;
            }
            else
            {
                next = diff_t2.tv_sec; /* wait for T2 */
            }
        }
    }
    return next;
}

void dhcp_init(const uint8_t *mac_addr, struct dhcp_binding *binding)
{
    memset(binding, 0, sizeof(struct dhcp_binding));
    binding->state = DHCP_INIT;
    memcpy(binding->mac_addr, mac_addr, DHCP_MAC_ADDR_LEN);
}

int dhcp_isbound(struct dhcp_binding *binding)
{
    time_t next;

    next = get_dhcp_next_event(binding);

    return (binding->state == DHCP_BOUND) && (next > 0);
}

time_t dhcp_step(struct dhcp_binding *binding)
{
    time_t next;

    next = get_dhcp_next_event(binding);
    if (next == 0)
    {
        int ret;

        ret = bootp_transaction(binding);
        if (ret != 0)
        {
            next = (time_t)ret;
        }
        else
        {
            next = get_dhcp_next_event(binding);
        }
    }

    return next;
}

time_t dhcp_bind(struct dhcp_binding *binding)
{
    time_t next;

    do
    {
        next = dhcp_step(binding);
    } while(!dhcp_isbound(binding));

    return next;
}

