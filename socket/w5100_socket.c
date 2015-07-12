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
#include <errno.h>
/* #include <sys/socket.h> */
#include "socket.h"
#include "w5100.h"

#define W5100_SOCKET_FREE (-1)

void w5100_socket_init(void);

static struct w5100_socket {
    int fd;
} w5100_sockets[W5100_N_SOCKETS];

static
int isocket_to_fd(int isocket)
{
    //TODO
    return isocket;
}

static
int fd_to_isocket(int fd)
{
    //TODO
    return fd;
}

static
int socket_alloc(void)
{
    int i;
    int ret;

    for (i = 0; i < W5100_N_SOCKETS; i++)
    {
        if (w5100_sockets[i].fd == W5100_SOCKET_FREE)
        {
            w5100_sockets[i].fd = i;
            break;
        }
    }
    if (i < W5100_N_SOCKETS)
    {
        ret = i;
    }
    else
    {
        errno = ENFILE;
        ret = -1;
    }
    return ret;
}

static
void socket_free(int isocket)
{
    w5100_sockets[isocket].fd = W5100_SOCKET_FREE;
}

static
int tcp_create(void)
{
    int isocket;

    isocket = socket_alloc();
    if (isocket != -1)
    {
        w5100_write_reg(
                w5100_sock_reg_get(W5100_Sn_MR, isocket),
                0x01 /* TCP */
                );
    }
    return isocket;
}

int socket(int domain, int type, int protocol)
{
    int ret;

    if (domain == AF_INET)
    {
        if (type == SOCK_STREAM)
        {
            int isocket;
            (void)protocol; /* ignored */
            isocket = tcp_create();
            if (isocket == -1)
            {
                ret = -1;
            }
            else
            {
                ret = isocket_to_fd(isocket);
            }
        }
        else
        {
            errno = EPROTONOSUPPORT;
            ret = -1;
        }
    }
    else
    {
        errno = EAFNOSUPPORT;
        ret = -1;
    }
    return ret;
}


__attribute__((__constructor__))
void w5100_socket_init(void)
{
    int i;

    w5100_init();

    for (i = 0; i < W5100_N_SOCKETS; i++)
    {
        w5100_sockets[i].fd = W5100_SOCKET_FREE;
    }
    w5100_write_reg(W5100_RMSR, 0x55); /* 2KiB per socket */
    
}

