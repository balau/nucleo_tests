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
#include <errno.h>
#include <limits.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/scb.h>

struct signal_queue_item
{
    int sig;
    union sigval value;
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

static
int signal_enqueue(int sig, union sigval value)
{
    int ret;
    int iqueue;
    int next_order;

    cm_disable_interrupts();
    next_order = signal_queue.last_order + 1;
    for (iqueue = 0; iqueue < SIGQUEUE_MAX; iqueue++)
    {
        if (signal_queue.items[iqueue].sig == 0)
        {
            signal_queue.items[iqueue].sig = sig;
            signal_queue.items[iqueue].value = value;
            signal_queue.items[iqueue].order = next_order;
            signal_queue.last_order = next_order;
            pendsv_interrupt_raise();

            ret = 0;
            break;
        }
    }
    cm_enable_interrupts();

    if (iqueue == SIGQUEUE_MAX)
    {
        errno = EAGAIN;
        ret = -1;
    }

    return ret;
}

static
int signal_dequeue(int *sig, union sigval *value)
{
    int ret;
    int iqueue;
    int order = -1;
    int iqueue_chosen = -1;

    cm_disable_interrupts();
    for (iqueue = 0; iqueue < SIGQUEUE_MAX; iqueue++)
    {
        if ( (signal_queue.items[iqueue].sig != 0) && (
                    (iqueue_chosen == -1) ||
                    ((signal_queue.items[iqueue].order - order) < 0)))
        {
            iqueue_chosen = iqueue;
            order = signal_queue.items[iqueue].order;
        }
    }
    if (iqueue_chosen != -1)
    {
        *sig = signal_queue.items[iqueue_chosen].sig;
        *value = signal_queue.items[iqueue_chosen].value;
        signal_queue.items[iqueue_chosen].sig = 0;
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    cm_enable_interrupts();

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
        ret = signal_enqueue(sig, value);
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
        signal_actions[sig].act = *act;
        pendsv_interrupt_raise();
        ret = 0;
    }

    return ret;
}

static
void signal_act(int sig, union sigval value)
{
    if (signal_actions[sig].act.sa_flags & SA_SIGINFO)
    {
        siginfo_t info;
        void (*sa_sigaction)(int, siginfo_t *, void *);
        sa_sigaction = (void *)signal_actions[sig].act.sa_sigaction;
        info.si_value = value;
        info.si_signo = sig;
        sa_sigaction(sig, &info, NULL);
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
    int sig;
    union sigval value;

    if (signal_dequeue(&sig, &value) == 0)
    {
        signal_act(sig, value);
    }
}

