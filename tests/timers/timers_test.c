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
#include <stdio.h>
#include <time.h>
#include <signal.h>

volatile int alarm_cnt;

static
void alarm_handler(int signo)
{
    printf("Alarm! %d\n", signo);
    alarm_cnt++;
}

int main(void)
{
    struct sigevent ev;
    timer_t timerid;
    struct itimerspec timspec;
    struct sigaction act;

    ev.sigev_notify = SIGEV_SIGNAL;
    ev.sigev_signo = SIGALRM;
    ev.sigev_value.sival_int = 0;

    if (timer_create(
            CLOCK_REALTIME,
            &ev,
            &timerid) != 0)
    {
        perror("timer_create");
        return 1;
    }

    act.sa_handler = alarm_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(
                SIGALRM,
                &act,
                NULL)!= 0)
    {
        perror("timer_create");
        return 1;
    }

    timspec.it_interval.tv_sec = 1;
    timspec.it_interval.tv_nsec = 0;
    timspec.it_value.tv_sec = 2;
    timspec.it_value.tv_nsec = 0;

    if (timer_settime(
                timerid,
                0,
                &timspec,
                NULL) != 0)
    {
        perror("timer_settime");
        return 1;
    }

    while(1)
    {
        struct timespec t;

        clock_gettime(CLOCK_MONOTONIC, &t);
        printf("%lds %ldns\n", (long)t.tv_sec, (long)t.tv_nsec);

        t.tv_sec = 0;
        t.tv_nsec = 500*1000*1000;
        nanosleep(&t, NULL);

        if (timer_gettime(
                    timerid,
                    &timspec) != 0)
        {
            perror("timer_gettime");
            return 1;
        }
        printf("%lds %ldns\n", (long)timspec.it_value.tv_sec, (long)timspec.it_value.tv_nsec);
        if (alarm_cnt > 3)
        {
            break;
        }
    }

    if (timer_delete(timerid) != 0)
    {
        perror("timer_delete");
    }
}


