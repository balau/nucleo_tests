/* Copyright (c) 2016 Francesco Balducci
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
#include "timesync.h"
#include <sys/socket.h>
#include <unistd.h>
#include "timespec.h"

#define RFC868_TYPE_TCP 1
#define RFC868_TYPE_UDP 2

#ifndef RFC868_TYPE
/* UDP is default because it's lighter on the network.
 * TCP might be too much to retrieve just 32bit of info.
 */
#  define RFC868_TYPE RFC868_TYPE_UDP
#endif

#if (RFC868_TYPE != RFC868_TYPE_TCP) && (RFC868_TYPE != RFC868_TYPE_UDP)
#  error "RFC868_TYPE must be one of: RFC868_TYPE_TCP, RFC868_TYPE_UDP"
#endif

/* Some public RFC868 servers. */
/*
 * http://www.inrim.it/ntp/services_i.shtml
 * time.inrim.it (193.204.114.105)
 * Note: only TCP seems to work.
 */
#define RFC868_SERVER_INRIM 0x6972ccc1

/* http://tf.nist.gov/tf-cgi/servers.cgi
 * time-c.nist.gov (129.6.15.30)
 */
#define RFC868_SERVER_NIST_C 0x1e0f0681

/* http://tf.nist.gov/tf-cgi/servers.cgi
 * utcnist.colorado.edu (128.138.140.44)
 */
#define RFC868_SERVER_COLORADO_EDU 0x2c8c8a80

#ifndef DEFAULT_RFC868_SERVER
#  define DEFAULT_RFC868_SERVER RFC868_SERVER_NIST_C
#endif

static in_addr_t rfc868_server = DEFAULT_RFC868_SERVER;

static
uint32_t recv_time32(int sock)
{
    uint32_t secs;
    ssize_t recv_res;
    uint32_t secs_net;

    recv_res = recv(sock, &secs_net, sizeof(secs_net), 0);
    if (recv_res == (ssize_t)sizeof(secs_net))
    {
        secs = ntohl(secs_net);
    }
    else
    {
        secs = 0;
    }
    return secs;
}

static
int rfc868_connect(int sock, in_addr_t server)
{
    int res;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(37); /* RFC 868 */
    addr.sin_addr.s_addr = server;

#if (RFC868_TYPE == RFC868_TYPE_TCP)
    res = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
#elif (RFC868_TYPE == RFC868_TYPE_UDP)
    ssize_t sendto_res;
    uint8_t dummy;
    size_t len;

    /* RFC868 says to send an empty datagram but for some interfaces
     * such as W5100 it's difficult to send empty datagrams, so we
     * send a byte. */
    dummy = 0;
    len = sizeof(dummy); 
    sendto_res = sendto(sock, &dummy, len, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (sendto_res == (ssize_t)len)
    {
        res = 0;
    }
    else
    {
        res = -1;
    }
#endif

    return res;
}

static
uint32_t rfc868_gettime32(in_addr_t server)
{
    int sock;
    uint32_t secs;
    int sock_type;

#if (RFC868_TYPE == RFC868_TYPE_TCP)
    sock_type = SOCK_STREAM;
#elif (RFC868_TYPE == RFC868_TYPE_UDP)
    sock_type = SOCK_DGRAM;
#endif

    sock = socket(AF_INET, sock_type, 0);
    if (sock >= 0)
    {
        int res;

        res = rfc868_connect(sock, server);
        if (res == 0)
        {
            secs = recv_time32(sock);
        }
        else
        {
            secs = 0;
        }
        close(sock);
    }
    else
    {
        secs = 0;
    }

    return secs;
}

static
void timespec_correct(struct timespec *ts)
{
    /* TODO: add the execution time of the rfc868_gettime function */
    ts->tv_nsec = NSECS_IN_SEC/2;
}

int rfc868_gettime(struct timespec *ts)
{
    int res;
    int32_t tp_secs;
    time_t secs_to_epoch;

    /* RFC 868 time starts at 1900/1/1
     * POSIX time starts at 1970/1/1
     * We need to convert one epoch to the other
     */
    secs_to_epoch = 2208988800U; /* (70*365 + 17)*86400 http://stackoverflow.com/a/29138806/1415942 */

    tp_secs = rfc868_gettime32(rfc868_server);
    if (tp_secs == 0)
    {
        res = -1;
    }
    else if (tp_secs < secs_to_epoch)
    {
        /* There was an error, since we are definitely not before 1970 */
        res = -1;
    }
    else
    {
        time_t posix_secs;

        posix_secs = tp_secs - secs_to_epoch;

        ts->tv_sec = posix_secs;
        ts->tv_nsec = 0;

        timespec_correct(ts);

        res = 0;
    }

    return res;
}

void rfc868_timeserver_set(in_addr_t server)
{
    rfc868_server = server;
}

in_addr_t rfc868_timeserver_get(void)
{
    return rfc868_server;
}

