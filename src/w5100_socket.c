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
#include <arpa/inet.h>
#include <errno.h>
#include <file.h>
#include "w5100.h"

/******* defines and macros ********/

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

enum w5100_socket_state {
    W5100_SOCK_STATE_NONE = 0,
    W5100_SOCK_STATE_CREATED,
    W5100_SOCK_STATE_CONNECTED,
    W5100_SOCK_STATE_BOUND,
    W5100_SOCK_STATE_LISTENING,
    W5100_SOCK_STATE_ACCEPTED,
};

/******* function prototypes ********/

extern
void w5100_socket_init(void);

static
void w5100_command(int isocket, uint8_t cmd);

static
int w5100_sock_write(int fd, char *buf, int len);

static
int w5100_sock_read(int fd, char *buf, int len);

static
int w5100_sock_close(int fd);

/******* global variables ********/

static struct w5100_socket {
    int fd;
    //enum w5100_socket_state state; //TODO
    struct fd *fd_data;
    struct fd *connection_data;
} w5100_sockets[W5100_N_SOCKETS];

static uint8_t w5100_mac_addr[6] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85};

/******* function definitions ********/

static
int isocket_to_fd(int isocket)
{
    int fd;

    if (isocket < 0 || isocket >= W5100_N_SOCKETS)
    {
        fd = -1;
    }
    else
    {
        fd = w5100_sockets[isocket].fd;
    }
    return fd;
}

static
int fd_to_isocket(int fd)
{
    int isocket;

    for (isocket = 0; isocket < W5100_N_SOCKETS; isocket++)
    {
        if (fd == w5100_sockets[isocket].fd)
        {
            break;
        }
        if (w5100_sockets[isocket].connection_data != NULL)
        {
            if (w5100_sockets[isocket].connection_data->fd == fd)
            {
                break;
            }
        }
    }
    if (isocket >= W5100_N_SOCKETS)
    {
        isocket = -1;
    }
    return isocket;
}

static
struct fd *fill_fd_struct(int sockfd, int isocket)
{
    struct fd *fds;
    
    fds = file_struct_get(sockfd);
    
    fds->isatty = 0;
    fds->isopen = 1;
    fds->write = w5100_sock_write;
    fds->read = w5100_sock_read;
    fds->close = w5100_sock_close;
    fds->stat.st_mode = S_IFSOCK|S_IRWXU|S_IRWXG|S_IRWXO;
    fds->stat.st_blksize = 1024;
    fds->opaque = &w5100_sockets[isocket];

    return fds;
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
    w5100_sockets[isocket].fd_data = NULL;
    w5100_sockets[isocket].connection_data = NULL;
}

static
int w5100_sock_write(int fd, char *buf, int len)
{
    return send(fd, buf, len, 0);
}

static
int w5100_sock_read(int fd, char *buf, int len)
{
    return recv(fd, buf, len, 0);
}

static
int w5100_sock_close(int fd)
{
    int isocket;
    int ret;

    isocket = fd_to_isocket(fd);
    if (isocket == -1)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        struct fd *fds;
        uint8_t sr;
        
        fds = file_struct_get(fd);
        fds->isopen = 0;
        syscall_ffree(fd);
        w5100_command(isocket, W5100_CMD_CLOSE);
        do {
            sr = w5100_read_sock_reg(W5100_Sn_SR, isocket);
        } while (sr != W5100_SOCK_CLOSED);
        if (w5100_sockets[isocket].fd == fd)
        {
            socket_free(isocket);
        }
        else if (w5100_sockets[isocket].connection_data != NULL)
        {
            if (w5100_sockets[isocket].connection_data->stat.st_ino == fd)
            {
                w5100_command(isocket, W5100_CMD_OPEN);
                w5100_command(isocket, W5100_CMD_LISTEN);
            }
        }
        ret = 0;
    }
    return ret;
}

static
int tcp_create(void)
{
    int isocket;

    isocket = socket_alloc();
    if (isocket != -1)
    {
        int fd;
        
        fd = syscall_falloc();
        if (fd == -1)
        {
            isocket = -1;
            errno = ENFILE;
        }
        else
        {
            struct fd *fds;
            
            fds = fill_fd_struct(fd, isocket);

            w5100_sockets[isocket].fd = fd;
            w5100_sockets[isocket].fd_data = fds;
            w5100_sockets[isocket].connection_data = NULL;
            
            w5100_write_sock_reg(W5100_Sn_MR, isocket, W5100_SOCK_MODE_TCP);
        }
    }
    else
    {
        errno = ENOMEM;
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

    isocket = fd_to_isocket(sockfd);

    if (isocket == -1)
    {
        errno = EBADF;
        ret = -1;
    }
    else if ( addr->sa_family != AF_INET )
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
        /* TODO: check ENOTSOCK */
        /* TODO: check TCP */
        w5100_write_sock_regx(W5100_Sn_PORT, isocket, &server->sin_port);
        w5100_command(isocket, W5100_CMD_OPEN);
        do {
            sr = w5100_read_sock_reg(W5100_Sn_SR, isocket);
        } while (sr != W5100_SOCK_INIT);

        w5100_write_sock_regx(W5100_Sn_DIPR, isocket, &server->sin_addr.s_addr);
        w5100_write_sock_regx(W5100_Sn_DPORT, isocket, &server->sin_port);
        w5100_command(isocket, W5100_CMD_CONNECT);
        do {
            sr = w5100_read_sock_reg(W5100_Sn_SR, isocket);
        } while ((sr != W5100_SOCK_CLOSED) && (sr != W5100_SOCK_ESTABLISHED));
        if (sr == W5100_SOCK_ESTABLISHED)
        {
            ret = 0;
        }
        else if (sr == W5100_SOCK_CLOSED)
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

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int ret;
    int isocket;

    isocket = fd_to_isocket(sockfd);

    if (isocket == -1)
    {
        errno = EBADF;
        ret = -1;
    }
    else if ( addr->sa_family != AF_INET )
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
        /* TODO: check ENOTSOCK */
        /* TODO: check TCP */
        w5100_write_sock_regx(W5100_Sn_PORT, isocket, &server->sin_port);
        w5100_command(isocket, W5100_CMD_OPEN);
        do {
            sr = w5100_read_sock_reg(W5100_Sn_SR, isocket);
        } while (sr != W5100_SOCK_INIT);
        ret = 0;
    }
    return ret;
}

int listen(int sockfd, int backlog)
{
    int ret;
    int isocket;

    isocket = fd_to_isocket(sockfd);

    if (isocket == -1)
    {
        errno = EBADF;
        ret = -1;
    }
    else /* TODO: UPD and RAW */
    {
        uint8_t sr;
        /* TODO: check if already in use EADDRINUSE */
        /* TODO: check that backlog is 0 */
        (void)backlog;
        w5100_command(isocket, W5100_CMD_LISTEN);
        do {
            sr = w5100_read_sock_reg(W5100_Sn_SR, isocket);
        } while ((sr != W5100_SOCK_LISTEN) && (sr != W5100_SOCK_ESTABLISHED));
        ret = 0;
    }
    return ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret;
    int isocket;

    isocket = fd_to_isocket(sockfd);

    if (isocket == -1)
    {
        errno = EBADF;
        ret = -1;
    }
    else /* TODO: UPD and RAW */
    {
        uint8_t sr;
        int newsockfd;

        do {
            sr = w5100_read_sock_reg(W5100_Sn_SR, isocket);
        } while (sr != W5100_SOCK_ESTABLISHED);

        newsockfd = syscall_falloc();
        if (newsockfd == -1)
        {
            errno = ENFILE;
            /* go again into listen state */
            w5100_command(isocket, W5100_CMD_CLOSE);
            w5100_command(isocket, W5100_CMD_OPEN);
            w5100_command(isocket, W5100_CMD_LISTEN);
        }
        else
        {
            struct sockaddr_in *client;
            struct fd *fds;
            
            fds = fill_fd_struct(newsockfd, isocket);

            w5100_sockets[isocket].connection_data = fds;
            
            if (addr != NULL)
            {
                if (addrlen != NULL)
                {
                    *addrlen = sizeof(struct sockaddr_in);
                }
                client = (struct sockaddr_in *)addr;
                addr->sa_family = AF_INET;
                
                w5100_read_sock_regx(W5100_Sn_DIPR, isocket, &client->sin_addr.s_addr);
                w5100_read_sock_regx(W5100_Sn_DPORT, isocket, &client->sin_port);
            }

        }
        ret = newsockfd;
    }
    return ret;
}

static
uint16_t get_tx_size(int isocket)
{
    (void)isocket;
    return 0x800; /* 2KiB */
}

static
uint16_t get_tx_mask(int isocket)
{
    return get_tx_size(isocket) - 1; /* size is always power of 2 */
}

static
uint16_t get_tx_base(int isocket)
{
    return W5100_TX_MEM_BASE + get_tx_size(isocket) * isocket;
}

static
uint16_t get_rx_size(int isocket)
{
    (void)isocket;
    return 0x800; /* 2KiB */
}

static
uint16_t get_rx_mask(int isocket)
{
    return get_rx_size(isocket) - 1; /* size is always power of 2 */
}

static
uint16_t get_rx_base(int isocket)
{
    return W5100_RX_MEM_BASE + get_rx_size(isocket) * isocket;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    int isocket;
    ssize_t ret;

    (void)flags; /* TODO */

    isocket = fd_to_isocket(sockfd);
    if (isocket == -1)
    {
        errno = EBADF;
        ret = 1;
    }
    else if (len == 0)
    {
        ret = 0;
    }
    else
    {
        uint16_t toread;

        do
        {
            w5100_read_sock_regx(W5100_Sn_RX_RSR, isocket, &toread);
            toread = ntohs(toread);
            if (toread != 0)
            {
                uint16_t pread;
                uint16_t offset;
                uint16_t phys;
                uint16_t toread1;
                uint16_t toread2;
                uint16_t newrd;
                uint8_t *bytes = buf;

                if (len < toread)
                {
                    toread = len;
                }
                w5100_read_sock_regx(W5100_Sn_RX_RD, isocket, &pread);
                pread = ntohs(pread);
                offset = pread & get_rx_mask(isocket);
                phys = get_rx_base(isocket) + offset;
                if (offset + toread > get_rx_size(isocket))
                {
                    toread1 = get_rx_size(isocket) - offset;
                    toread2 = toread - toread1;
                }
                else
                {
                    toread1 = toread;
                    toread2 = 0;
                }
                if (toread1 > 0)
                {
                    w5100_read_mem(phys, &bytes[0], toread1);
                }
                if (toread2 > 0)
                {
                    w5100_read_mem(get_rx_base(isocket), &bytes[toread1], toread2);
                }
                newrd = htons(pread + toread);
                w5100_write_sock_regx(W5100_Sn_RX_RD, isocket, &newrd);
                w5100_command(isocket, W5100_CMD_RECV);
            }
        } while(toread == 0);
        ret = toread;
    }
    return ret;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    int isocket;
    ssize_t ret;

    (void)flags; /* TODO */

    isocket = fd_to_isocket(sockfd);
    if (isocket == -1)
    {
        errno = EBADF;
        ret = 1;
    }
    else
    {
        uint16_t nfree;
        uint16_t pwrite;
        uint16_t offset;
        uint16_t phys;
        uint16_t towrite;
        uint16_t towrite1;
        uint16_t towrite2;
        uint16_t newwr;
        const uint8_t *bytes = buf;

        w5100_read_sock_regx(W5100_Sn_TX_FSR, isocket, &nfree);
        nfree = ntohs(nfree);
        w5100_read_sock_regx(W5100_Sn_TX_WR, isocket, &pwrite);
        pwrite = ntohs(pwrite);
        offset = pwrite & get_tx_mask(isocket);
        phys = offset + get_tx_base(isocket);
        if (len > nfree)
        {
            towrite = nfree;
        }
        else
        {
            towrite = len;
        }
        if (towrite > (get_tx_size(isocket) - offset))
        {
            towrite1 = get_tx_size(isocket) - offset;
            towrite2 = towrite - towrite1;
        }
        else
        {
            towrite1 = towrite;
            towrite2 = 0;
        }
        if (towrite1 > 0)
        {
            w5100_write_mem(phys, &bytes[0], towrite1);
        }
        if (towrite2 > 0)
        {
            w5100_write_mem(get_tx_base(isocket), &bytes[towrite1], towrite2);
        }
        newwr = htons(pwrite + towrite);
        w5100_write_sock_regx(W5100_Sn_TX_WR, isocket, &newwr);
        w5100_command(isocket, W5100_CMD_SEND);
        ret = towrite;
    }
    return ret;
}

__attribute__((__constructor__))
void w5100_socket_init(void)
{
    int i;
    in_addr_t addr;

    w5100_init();

    w5100_write_reg(W5100_MR, W5100_MODE_RST); /* RST */
    while(w5100_read_reg(W5100_MR) & W5100_MODE_RST) /* RST bit clears by itself */
    {
        continue;
    }
    
    for (i = 0; i < W5100_N_SOCKETS; i++)
    {
        w5100_sockets[i].fd = W5100_SOCKET_FREE;
    }
    w5100_write_reg(W5100_RMSR, 0x55); /* 2KiB per socket */
    w5100_write_reg(W5100_TMSR, 0x55); /* 2KiB per socket */
    w5100_write_regx(W5100_SHAR, w5100_mac_addr);
    addr = inet_addr(W5100_IP_ADDR);
    w5100_write_regx(W5100_SIPR, &addr);
    addr = inet_addr(W5100_GATEWAY_ADDR);
    w5100_write_regx(W5100_GAR, &addr);
    addr = inet_addr(W5100_SUBNET);
    w5100_write_regx(W5100_SUBR, &addr);
}

