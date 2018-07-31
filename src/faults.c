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
#include <libopencm3/cm3/nvic.h>
#include "sigqueue_info.h"

void mem_manage_handler(void)
{
    siginfo_t info;

    info.si_signo = SIGSEGV;
    info.si_code = SEGV_ACCERR;
    /* TODO: addr */

    sigqueue_info(&info);
}

void bus_fault_handler(void)
{
    siginfo_t info;

    info.si_signo = SIGBUS;
    info.si_code = BUS_ADRERR;
    /* TODO: addr */

    sigqueue_info(&info);
}

void usage_fault_handler(void)
{
    siginfo_t info;

    info.si_signo = SIGILL;
    info.si_code = ILL_ILLOPC;
    /* TODO: addr */

    sigqueue_info(&info);
}

void hard_fault_handler(void)
{
    siginfo_t info;

    info.si_signo = SIGABRT;

    sigqueue_info(&info);
}

void nmi_handler(void)
{
    siginfo_t info;

    info.si_signo = SIGABRT;

    sigqueue_info(&info);
}

