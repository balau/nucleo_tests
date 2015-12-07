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
#ifndef W5100_DHCP
#define W5100_DHCP

#include <netinet/in.h>
#include <time.h>

/**
 * Manage DHCP.
 *
 * \retval < 0 an error occurred.
 * \retval == 0 should be re-called as soon as possible.
 * \retval > 0 the number of seconds after which it should be called again.
 */
extern
time_t w5100_dhcp(void);

/**
 * Manage DHCP and ensure that interface is bound to an address on exit.
 *
 * \return the number of seconds after which it should be called again.
 */
extern
time_t w5100_dhcp_bind(void);

/**
 * Get DNS server retrieved by DHCP
 */
extern
in_addr_t w5100_getdns(void);

/*
 * Get whether the interface has an IP address bound with a DHCP server.
 */
extern
int w5100_dhcp_isbound(void);

#endif /* W5100_DHCP */

