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
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/scb.h>
#include "sigqueue_info.h"

struct signal_queue_item
{
    siginfo_t info;
    int order;
};

static struct {
    int last_order;
    struct signal_queue_item items[SIGQUEUE_MAX];
} signal_queue;

#define SIGNAL_MAX 32

struct signal_action
{
    struct sigaction act;
};

static
void pendsv_interrupt_raise(void);

struct signal_action signal_actions[SIGNAL_MAX + 1];

int sigqueue_info(const siginfo_t *info)
{
    int ret;
    int iqueue;
    int next_order;
    bool faults_already_disabled;

    faults_already_disabled = cm_is_masked_faults();
    if (!faults_already_disabled)
    {
        cm_disable_faults();
    }
    next_order = signal_queue.last_order + 1;
    for (iqueue = 0; iqueue < SIGQUEUE_MAX; iqueue++)
    {
        if (signal_queue.items[iqueue].info.si_signo == 0)
        {
            signal_queue.items[iqueue].info = *info;
            signal_queue.items[iqueue].order = next_order;
            signal_queue.last_order = next_order;
            pendsv_interrupt_raise();

            ret = 0;
            break;
        }
    }
    if (!faults_already_disabled)
    {
        cm_enable_faults();
    }

    if (iqueue == SIGQUEUE_MAX)
    {
        errno = EAGAIN;
        ret = -1;
    }

    return ret;
}

static
int signal_dequeue(siginfo_t *info)
{
    int ret;
    int iqueue;
    int order = -1;
    int iqueue_chosen = -1;
    bool faults_already_disabled;

    faults_already_disabled = cm_is_masked_faults();
    if (!faults_already_disabled)
    {
        cm_disable_faults();
    }
    for (iqueue = 0; iqueue < SIGQUEUE_MAX; iqueue++)
    {
        if ( (signal_queue.items[iqueue].info.si_signo != 0) && (
                    (iqueue_chosen == -1) ||
                    ((signal_queue.items[iqueue].order - order) < 0)))
        {
            iqueue_chosen = iqueue;
            order = signal_queue.items[iqueue].order;
        }
    }
    if (iqueue_chosen != -1)
    {
        *info = signal_queue.items[iqueue_chosen].info;
        signal_queue.items[iqueue_chosen].info.si_signo = 0;
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    if (!faults_already_disabled)
    {
        cm_enable_faults();
    }

    return ret;
}

int sigqueue (pid_t pid, int sig, union sigval value)
{
    int ret;
    pid_t this;

    this = getpid();

    if (pid != this)
    {
        errno = ESRCH;
        ret = -1;
    }
    else if (sig != 0)
    {
        siginfo_t info;

        memset(&info, 0, sizeof(siginfo_t));
        info.si_value = value;
        info.si_signo = sig;
        info.si_code = SI_QUEUE;
        ret = sigqueue_info(&info);
    }
    else
    {
        ret = 0;
    }

    return ret;
}

static
sigset_t sigmask(int sig)
{
    return (1<<(sig-1));
}

int sigismember(const sigset_t *set, int sig)
{
    int ret;
    
    ret = *set & sigmask(sig);

    return ret;
}

int sigaddset(sigset_t *set, int sig)
{
    *set |= sigmask(sig);

    return 0;
}

int sigdelset(sigset_t *set, int sig)
{
    *set &= ~sigmask(sig);

    return 0;
}

int sigemptyset(sigset_t *set)
{
    *set = 0;

    return 0;
}

int sigfillset(sigset_t *set)
{
    *set = ~0;

    return 0;
}

int sigaction(
        int sig,
        const struct sigaction *restrict act,
        struct sigaction *restrict oact)
{
    int ret;

    if ( (sig > SIGNAL_MAX) || (sig < 0) )
    {
        errno = EINVAL;
        ret = -1;
    }
    else
    {
        if (oact != NULL)
        {
            *oact = signal_actions[sig].act;
        }
        if (act != NULL)
        {
            signal_actions[sig].act = *act;
        }
        ret = 0;
    }

    return ret;
}

static
void signal_act(siginfo_t *info)
{
    int sig = info->si_signo;

    if (signal_actions[sig].act.sa_flags & SA_SIGINFO)
    {
        void (*sa_sigaction)(int, siginfo_t *, void *);
        sa_sigaction = (void *)signal_actions[sig].act.sa_sigaction;
        sa_sigaction(sig, info, NULL);
    }
    else
    {
        signal_actions[sig].act.sa_handler(sig);
    }
}

static
void pendsv_interrupt_raise(void)
{
    SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

void pend_sv_handler(void)
{
    siginfo_t info;

    if (signal_dequeue(&info) == 0)
    {
        signal_act(&info);
    }
}

