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
#include <signal.h>

static
void alarm_handler(int signo, siginfo_t *info, void *context)
{
    (void)context;
    printf("Alarm! %d %d %d %d\n", signo, info->si_signo, info->si_value.sival_int, info->si_code);
}

int main(void)
{
    struct sigaction act;

    act.sa_sigaction = alarm_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    if (sigaction(
                SIGALRM,
                &act,
                NULL) != 0)
    {
        perror("sigaction");
        return 1;
    }

    if (sigaction(
                SIGUSR1,
                &act,
                NULL) != 0)
    {
        perror("sigaction");
        return 1;
    }

    if (sighold(SIGALRM) != 0)
    {
        perror("sighold");
        return 1;
    }
    if (sighold(SIGUSR1) != 0)
    {
        perror("sighold");
        return 1;
    }
    printf("holding...\n");

    if (raise(SIGALRM) != 0)
    {
        perror("raise");
        return 1;
    }
    printf("raised...\n");

    if (raise(SIGUSR1) != 0)
    {
        perror("raise");
        return 1;
    }
    printf("raised...\n");

    printf("unblocking...\n");
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGUSR1);

    if (sigprocmask(SIG_UNBLOCK, &set, NULL) != 0)
    {
        perror("sigprocmask");
        return 1;
    }
    printf("unblocked!\n");
}


