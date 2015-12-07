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

static
int initialized = 0;

static
void configure(void)
{
    w5100_write_regx(W5100_SIPR, &binding.client);
    w5100_write_regx(W5100_GAR, &binding.gateway);
    w5100_write_regx(W5100_SUBR, &binding.subnet);
}

in_addr_t w5100_getdns(void)
{
    return binding.dns_server;
}

time_t w5100_dhcp(void)
{
    int ret;

    if (!initialized)
    {
        uint8_t mac_addr[6];

        w5100_read_regx(W5100_SHAR, mac_addr);

        /* TODO: check if we already have a preferred IP address */
        //w5100_read_reg(W5100_SIPR, &binding.client);
        //w5100_read_reg(W5100_GAR, &binding.gateway);
        //w5100_read_reg(W5100_SUBR, &binding.subnet);
        dhcp_init(mac_addr, &binding);
        initialized = 1;
    }
    while(1)
    {
        ret = dhcp_update(&binding);
        if (ret > 0 && binding.state == DHCP_BOUND)
        {
            configure();
            break;
        }
    }
    return ret;
}

