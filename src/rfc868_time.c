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

#ifndef DEFAULT_RFC868_SERVER
/*
 * http://www.inrim.it/ntp/services_i.shtml
 * time.inrim.it (193.204.114.105)
 */
#  define DEFAULT_RFC868_SERVER 0x6972ccc1
#endif

static in_addr_t rfc868_server = DEFAULT_RFC868_SERVER;

static
uint32_t rfc868_gettime32(in_addr_t server)
{
    int sock;
    uint32_t secs;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock >= 0)
    {
        int res;
        struct sockaddr_in addr;

        addr.sin_family = AF_INET;
        addr.sin_port = htons(37); /* RFC 868 */
        addr.sin_addr.s_addr = server;

        res = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        if (res == 0)
        {
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

