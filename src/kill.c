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
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "sigqueue_info.h"

int kill (pid_t pid, int sig)
{
    int ret;
    pid_t this;

    this = getpid();

    if ( (pid != this) && (pid > 0) )
    {
        errno = ESRCH;
        ret = -1;
    }
    else if (sig != 0)
    {
        siginfo_t info;

        memset(&info, 0, sizeof(info));
        info.si_signo = sig;
        info.si_code = SI_USER;
        info.si_pid = this;

        ret = sigqueue_info(&info);
    }
    else
    {
        ret = 0;
    }

    return ret;
}

