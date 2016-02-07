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
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "timespec.h"

/* Some public SNTP servers. */

/* http://tf.nist.gov/tf-cgi/servers.cgi
 * time-c.nist.gov (129.6.15.30)
 */
#define SNTP_SERVER_NIST_C 0x1e0f0681

/* http://tf.nist.gov/tf-cgi/servers.cgi
 * utcnist.colorado.edu (128.138.140.44)
 */
#define SNTP_SERVER_COLORADO_EDU 0x2c8c8a80

#ifndef DEFAULT_SNTP_SERVER
#  define DEFAULT_SNTP_SERVER SNTP_SERVER_NIST_C
#endif

/* NTP time starts at 1900/1/1
 * POSIX time starts at 1970/1/1
 * We need to convert one epoch to the other.
 * (70*365 + 17)*86400 http://stackoverflow.com/a/29138806/1415942
 */
#define SECS_TO_EPOCH 2208988800U

#define NTP_MESSAGE_WORDS (4 + (4*2))
#define NTP_TRANSMIT_TIMESTAMP_OFFSET (4+3*2)

#define NTP_MODE_BITPOS 24
#define NTP_MODE_BITMASK (0x7<<NTP_MODE_BITPOS)

#define NTP_VERSION_BITPOS 27
#define NTP_VERSION_BITMASK (0x7<<NTP_VERSION_BITPOS)

#define NTP_VERSION 4

#define NTP_MODE_CLIENT 3
#define NTP_MODE_SERVER 4

struct ntp_timestamp {
    uint32_t ntp_sec;
    uint32_t ntp_fract;
};

static in_addr_t sntp_server = DEFAULT_SNTP_SERVER;

/* Logic Shift Right Round */
static
uint32_t lsrr(uint32_t v, int shift)
{
    if (shift > 0)
    {
        uint32_t round;

        round = 1 << (shift-1);
        if (v < (UINT32_MAX - round))
        {
            v += round;
        }
        v >>= shift;
    }
    return v;
}

static
void ntp_to_timespec(const struct ntp_timestamp *nt, struct timespec *ts)
{
    uint32_t sec;
    uint32_t fract;
    uint32_t nsec;
    int fract_shift;

    sec = ntohl(nt->ntp_sec);
    fract = ntohl(nt->ntp_fract);

    sec -= SECS_TO_EPOCH;
    
    fract_shift = 18;
    fract = lsrr(fract, fract_shift);
    nsec = fract * lsrr(NSECS_IN_SEC, (32 - fract_shift));

    ts->tv_sec = sec;
    ts->tv_nsec = nsec;
}

static
int sntp_request(int sock, in_addr_t server)
{
    int res;
    struct sockaddr_in addr;
    ssize_t send_res;
    uint32_t ntp_message[NTP_MESSAGE_WORDS];
    uint32_t hdr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(123);
    addr.sin_addr.s_addr = server;

    memset(ntp_message, 0, sizeof(ntp_message));
    hdr = 0;
    hdr |= NTP_MODE_CLIENT<<NTP_MODE_BITPOS;
    hdr |= NTP_VERSION<<NTP_VERSION_BITPOS;
    ntp_message[0] = htonl(hdr);

    send_res = sendto(sock, ntp_message, sizeof(ntp_message), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (send_res == sizeof(ntp_message))
    {
        res = 0;
    }
    else
    {
        res = -1;
    }

    return res;
}

static
int sntp_reply(int sock, struct timespec *ts)
{
    int res;
    ssize_t recv_res;
    uint32_t ntp_message[NTP_MESSAGE_WORDS];

    recv_res = recv(sock, ntp_message, sizeof(ntp_message), 0);
    if (recv_res == sizeof(ntp_message))
    {
        uint32_t hdr;

        hdr = ntohl(ntp_message[0]);
        if (((hdr & NTP_MODE_BITMASK) >> NTP_MODE_BITPOS) != NTP_MODE_SERVER)
        {
            res = -1;
        }
        else if (((hdr & NTP_VERSION_BITMASK) >> NTP_VERSION_BITPOS) != NTP_VERSION)
        {
            res = -1;
        }
        else
        {
            struct ntp_timestamp nt;

            memcpy(&nt, &ntp_message[NTP_TRANSMIT_TIMESTAMP_OFFSET], sizeof(nt));
            ntp_to_timespec(&nt, ts);
            res = 0;
        }
    }
    else
    {
        res = -1;
    }

    return res;
}

int sntp_gettime(struct timespec *ts)
{
    int res;
    int sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock >= 0)
    {
        res = sntp_request(sock, sntp_server);
        if (res == 0)
        {
            res = sntp_reply(sock, ts);
        }
        close(sock);
    }
    else
    {
        res = -1;
    }

    return res;
}

void sntp_timeserver_set(in_addr_t server)
{
    sntp_server = server;
}

in_addr_t sntp_timeserver_get(void)
{
    return sntp_server;
}

