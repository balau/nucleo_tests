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
#include "inet.h" // <arpa/inet.h>

#define W5100_SOCKET_FREE (-1)

#ifndef W5100_IP_ADDR
#  define W5100_IP_ADDR "192.168.1.99"
#endif
#ifndef W5100_SUBNET
#  define W5100_SUBNET "255.255.255.0"
#endif
#ifndef W5100_GATEWAY_ADDR
#  define W5100_GATEWAY_ADDR "192.168.1.1"
#endif

void w5100_socket_init(void);

static struct w5100_socket {
    int fd;
} w5100_sockets[W5100_N_SOCKETS];

static uint8_t w5100_mac_addr[6] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85};

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

static
void w5100_command(int isocket, uint8_t cmd)
{
    w5100_write_sock_reg(W5100_Sn_CR, isocket, cmd);
    while (w5100_read_sock_reg(W5100_Sn_CR, isocket))
    {
        continue;
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;
    int isocket;

    isocket = isocket_to_fd(sockfd);

    if ( addr->sa_family != AF_INET )
    {
        errno = EAFNOSUPPORT;
        ret = -1;
    }
    else /* TODO: UPD and RAW */
    {
        struct sockaddr_in *server;
        uint8_t sr;

        (void)addrlen;

        server = (struct sockaddr_in *)addr;
        /* TODO: check if already in use EADDRINUSE */
        /* TODO: check EBADF */
        /* TODO: check ENOTSOCK */
        w5100_write_sock_reg(W5100_Sn_MR, isocket, 0x01); /* TCP */
        w5100_write_sock_regx(W5100_Sn_PORT, isocket, &server->sin_port);
        w5100_command(isocket, 0x01); /* OPEN */
        do {
            sr = w5100_read_sock_reg(W5100_Sn_SR, isocket);
        } while (sr != 0x13); /* INIT */

        w5100_write_sock_regx(W5100_Sn_DIPR, isocket, &server->sin_addr.s_addr);
        w5100_write_sock_regx(W5100_Sn_DPORT, isocket, &server->sin_port);
        w5100_command(isocket, 0x04); /* CONNECT */
        do {
            sr = w5100_read_sock_reg(W5100_Sn_SR, isocket);
        } while ((sr != 0x00) && (sr != 0x17)); /* CLOSED or ESTABLISHED */
        if (sr == 0x17) /* ESTABLISHED */
        {
            ret = 0;
        }
        else if (sr == 0x00)
        {
            errno = ETIMEDOUT;
            ret = -1;
        }
        else
        {
            errno = ETIMEDOUT; /* TODO: better error? */
            ret = -1;
        }
    }
    return ret;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    (void)sockfd;
    (void)buf;
    (void)len;
    (void)flags;
    /* TODO */
    errno = ENOTCONN;
    return -1;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    (void)sockfd;
    (void)buf;
    (void)len;
    (void)flags;
    /* TODO */
    errno = ENOTCONN;
    return -1;
}

__attribute__((__constructor__))
void w5100_socket_init(void)
{
    int i;
    in_addr_t addr;

    w5100_init();

    w5100_write_reg(W5100_MR, 0x80); /* RST */
    while(w5100_read_reg(W5100_MR) & 0x80) /* RST bit clears by itself */
    {
        continue;
    }
    
    for (i = 0; i < W5100_N_SOCKETS; i++)
    {
        w5100_sockets[i].fd = W5100_SOCKET_FREE;
    }
    w5100_write_reg(W5100_RMSR, 0x55); /* 2KiB per socket */
    w5100_write_regx(W5100_SHAR, w5100_mac_addr);
    addr = inet_addr(W5100_IP_ADDR);
    w5100_write_regx(W5100_SIPR, &addr);
    addr = inet_addr(W5100_GATEWAY_ADDR);
    w5100_write_regx(W5100_GAR, &addr);
    addr = inet_addr(W5100_SUBNET);
    w5100_write_regx(W5100_SUBR, &addr);
}

