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
    printf("Alarm! %d %d %d %p\n", signo, info->si_signo, info->si_value.sival_int, context);
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

    if (raise(SIGALRM) != 0)
    {
        perror("raise");
        return 1;
    }
}

