/*
 * Copyright (c) 2016 Francesco Balducci
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
#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>
#include "timesync.h"

static
void print_timespec(const struct timespec *t)
{
    struct tm datetime;
    char datetime_str[80];

    printf("%lds %ldns\n", (long)t->tv_sec, (long)t->tv_nsec);
    datetime = *gmtime(&t->tv_sec);
    strftime(datetime_str, sizeof(datetime_str), "%A %d %B %Y, %H:%M:%S", &datetime);
    printf("%s\n", datetime_str);
}

int main(void)
{
    struct timespec t;
    struct in_addr server;
    int res;

    printf(
            "rfc868_time_test\n"
            "Press any key to continue...\n");
    getchar();

    clock_gettime(CLOCK_REALTIME, &t);
    print_timespec(&t);

    server.s_addr = rfc868_timeserver_get();
    printf("RFC868 server: %s\n", inet_ntoa(server));
    res = rfc868_gettime(&t);
    if (res == 0)
    {
        print_timespec(&t);
    }
    else
    {
        perror("rfc868_gettime");
    }

    return 0;
}

