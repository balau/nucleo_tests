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
#include "w5100_dhcp.h"
#include "dhcp_client.h"
#include "w5100.h"
#include "timespec.h"

static
struct dhcp_binding binding;

static struct config {
    in_addr_t client;
    in_addr_t gateway;
    in_addr_t subnet;
} config;

static
int initialized = 0;

static
void configure(void)
{
    if (binding.client != config.client)
    {
        w5100_write_regx(W5100_SIPR, &binding.client);
        config.client = binding.client;
    }
    if (binding.gateway != config.gateway)
    {
        w5100_write_regx(W5100_GAR, &binding.gateway);
        config.gateway = binding.gateway;
    }
    if (binding.subnet != config.subnet)
    {
        w5100_write_regx(W5100_SUBR, &binding.subnet);
        config.subnet = binding.subnet;
    }
}

in_addr_t w5100_getdns(void)
{
    return binding.dns_server;
}

int w5100_dhcp_isbound(void)
{
    return dhcp_isbound(&binding);
}

time_t w5100_dhcp(void)
{
    time_t next;

    if (!initialized)
    {
        uint8_t mac_addr[6];

        w5100_read_regx(W5100_SHAR, mac_addr);

        /* TODO: check if we already have a preferred IP address */
        dhcp_init(mac_addr, &binding);
        initialized = 1;
    }
    next = dhcp_step(&binding);

    if (w5100_dhcp_isbound())
    {
        configure();
    }
    return next;
}

time_t w5100_dhcp_bind(void)
{
    time_t ret;

    do
    {
        ret = w5100_dhcp();
    } while(!w5100_dhcp_isbound());

    return ret;
}

