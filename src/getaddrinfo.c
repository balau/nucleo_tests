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
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>

in_addr_t getaddrinfo_dns_server = 0x08080808; /* google */

void freeaddrinfo(struct addrinfo * ai)
{
    struct addrinfo *next;

    for (; ai != NULL; ai = next)
    {
        if (ai->ai_addr != NULL)
        {
            free(ai->ai_addr);
        }
        if (ai->ai_canonname != NULL)
        {
            free(ai->ai_canonname);
        }
        next = ai->ai_next;
        free(ai);
    }
}

int getaddrinfo(
        const char *__restrict nodename,
        const char *__restrict servname,
        const struct addrinfo *__restrict hints,
        struct addrinfo **__restrict res)
{
    int ret;

    ret = EAI_FAIL;

    return ret;
}

