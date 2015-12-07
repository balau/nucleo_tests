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

#ifndef DHCP_CLIENT_H
#define DHCP_CLIENT_H

#include <netinet/in.h>
#include <stdint.h>
#include <time.h>

#define DHCP_MAC_ADDR_LEN 6

enum dhcp_state {
    DHCP_INIT,
    DHCP_SELECTING,
    DHCP_REQUESTING,
    DHCP_BOUND,
    DHCP_INIT_REBOOT,
    DHCP_REBOOTING,
    DHCP_RENEWING,
    DHCP_REBINDING,
};

struct dhcp_binding {
    enum dhcp_state state; /**< DHCP state. */
    uint8_t mac_addr[DHCP_MAC_ADDR_LEN]; /* Client MAC address. */
    in_addr_t client; /**< New client IP address. */
    in_addr_t dhcp_server; /**< Server that issued the client IP address. */
    in_addr_t gateway; /**< Gateway IP address. */
    in_addr_t subnet; /**< Subnet IP mask. */
    in_addr_t dns_server; /**< DNS server address. */
    uint32_t xid; /**< Transaction ID */
    struct timespec lease_t1; /**< Client IP address lease expiring. */
    struct timespec lease_t2; /**< Client IP address lease end. */
};

/**
 * Initialize DHCP client library.
 *
 * Other functions shall not be called before this.
 *
 * \param[in] mac_addr
 * \param[out] binding
 */
extern
void dhcp_init(const uint8_t *mac_addr, struct dhcp_binding *binding);

/**
 * Perform a step of DHCP procedure.
 *
 * \param[inout] binding
 *
 * \retval < 0 an error occurred.
 * \retval == 0 should be re-called as soon as possible.
 * \retval > 0 the number of seconds after which it should be called again.
 */
extern
time_t dhcp_step(struct dhcp_binding *binding);

/**
 * Get whether client is bound to an address with a DHCP server.
 *
 * \param[inout] binding
 *
 * \retval nonzero the interface is bound to an address.
 * \retval zero the interface is not bound to an address.
 */
extern
int dhcp_isbound(struct dhcp_binding *binding);

/**
 * Perform DHCP procedure blocking until client is bound to an address.
 *
 * \param[inout] binding
 *
 * \return the number of seconds after which it should be called again.
 */
extern
time_t dhcp_bind(struct dhcp_binding *binding);

#define DHCP_EINTERNAL (-1)
#define DHCP_ESYSCALL (-2)
#define DHCP_EYIADDR (-3)
#define DHCP_EOFFEREXPECTED (-4)
#define DHCP_ENOSERVERID (-5)
#define DHCP_ENOGATEWAY (-6)
#define DHCP_ENOSUBNET (-7)
#define DHCP_EAGAIN (-8)

#endif /* DHCP_CLIENT_H */

