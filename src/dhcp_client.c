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

#ifndef DHCP_ALLOCATE_RETRIES
#  define DHCP_ALLOCATE_RETRIES 3
#endif

#ifndef DHCP_DISCOVER_RETRIES
#  define DHCP_DISCOVER_RETRIES 3
#endif

#ifndef DHCP_REQUEST_RETRIES
#  define DHCP_REQUEST_RETRIES 3
#endif

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
int dhcp_discover(
        const uint8_t *mac_addr,
        struct offer *offer)
{
    return DHCP_EINTERNAL;
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

