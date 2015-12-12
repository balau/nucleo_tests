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
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

#define NAME_MAX_LEN 256
#define ADDRESSES_MAX 4

static
struct hostent host;

static
in_addr_t addresses[ADDRESSES_MAX];

static
char *paddresses[ADDRESSES_MAX + 1]; /* NULL-terminated */

static
char hostname[NAME_MAX_LEN];

static
char *alias_null[1] = { NULL }; /* NULL-terminated immediately */

struct hostent *gethostbyname(const char *name)
{
    struct addrinfo *res;
    int ret;

    if (strlen(name) >= NAME_MAX_LEN)
    {
        return NULL;
    }
    ret = getaddrinfo(name, NULL, NULL, &res);
    if ((ret != 0) || (res == NULL))
    {
        return NULL;
    }
    else
    {
        int i_addr;

        strncpy(hostname, name, NAME_MAX_LEN);
        i_addr = 0;
        while((res != NULL) && (i_addr < ADDRESSES_MAX))
        {
            struct sockaddr_in *addr;

            addr = (struct sockaddr_in *)res->ai_addr;
            addresses[i_addr] = addr->sin_addr.s_addr;
            paddresses[i_addr] = (char *)&addresses[i_addr];
            res = res->ai_next;
            i_addr++;
        }
        paddresses[i_addr] = NULL;
        host.h_name = hostname;
        host.h_aliases = &alias_null[0];
        host.h_addrtype = AF_INET;
        host.h_length = 4;
        host.h_addr_list = paddresses;

        freeaddrinfo(res);

        return &host;
    }
}

