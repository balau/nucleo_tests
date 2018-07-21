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
#include "timespec.h"

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

static
sigset_t procmask;

static
struct signal_action signal_actions[SIGNAL_MAX + 1];

static
int critical_section_begin(void)
{
    int faults_already_disabled;

    faults_already_disabled = cm_is_masked_faults();
    if (!faults_already_disabled)
    {
        cm_disable_faults();
    }

    return faults_already_disabled;
}

static
void critical_section_end(int state)
{
    int faults_already_disabled = state;

    if (!faults_already_disabled)
    {
        cm_enable_faults();
    }
}

int sigqueue_info(const siginfo_t *info)
{
    int ret;
    int iqueue;
    int next_order;
    int cs_state;

    cs_state = critical_section_begin();
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
    critical_section_end(cs_state);

    if (iqueue == SIGQUEUE_MAX)
    {
        errno = EAGAIN;
        ret = -1;
    }

    return ret;
}

static
int signal_dequeue(const sigset_t *set, siginfo_t *info)
{
    int ret;
    int iqueue;
    int order = -1;
    int iqueue_chosen = -1;
    int cs_state;

    cs_state = critical_section_begin();

    for (iqueue = 0; iqueue < SIGQUEUE_MAX; iqueue++)
    {
        int candidate;
        int important;

        candidate =
            (signal_queue.items[iqueue].info.si_signo != 0) &&
            sigismember(set, signal_queue.items[iqueue].info.si_signo);

        if (!candidate)
        {
            important = 0;
        }
        else if (iqueue_chosen == -1)
        {
            important = 1;
        }
        else
        {
            important = ((signal_queue.items[iqueue].order - order) < 0);
        }

        if (candidate && important)
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
    critical_section_end(cs_state);

    return ret;
}

static
int signal_dequeue_procmask(siginfo_t *info)
{
    int ret;
    sigset_t set;

    set = ~procmask;
    ret = signal_dequeue(&set, info);

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

int siginterrupt(int sig, int flag)
{
    int ret;
    struct sigaction act;

    (void) sigaction(sig, NULL, &act);
    if (flag)
    {
        act.sa_flags &= ~SA_RESTART;
    }
    else
    {
        act.sa_flags |= SA_RESTART;
    }
    ret = sigaction(sig, &act, NULL);

    return ret;
}

static
int sigholdrel(int how, int sig)
{
    int ret;

    if (sig <= 0)
    {
        errno = EINVAL;
        ret = -1;
    }/*TODO: max?*/
    else
    {
        sigset_t set;

        sigemptyset(&set);
        sigaddset(&set, sig);
        sigprocmask(how, &set, NULL);

        ret = 0;
    }
    return ret;
}

int sighold(int sig)
{
    return sigholdrel(SIG_BLOCK, sig);
}

int sigrelse(int sig)
{
    return sigholdrel(SIG_UNBLOCK, sig);
}

int sigprocmask(
        int how,
        const sigset_t *restrict set,
        sigset_t *restrict oset)
{
    int ret;

    if (oset != NULL)
    {
        *oset = procmask;
    }
    if (set == NULL)
    {
        /* do nothing */
        ret = 0;
    }
    else if (how == SIG_BLOCK)
    {
        procmask |= *set;
        ret = 0;
    }
    else if (how == SIG_SETMASK)
    {
        procmask = *set;
        pendsv_interrupt_raise();
        ret = 0;
    }
    else if (how == SIG_UNBLOCK)
    {
        procmask &= ~(*set);
        pendsv_interrupt_raise();
        ret = 0;
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

int sigignore(int sig)
{
    int ret;
    struct sigaction act;

    (void) sigaction(sig, NULL, &act);
    act.sa_handler = SIG_IGN;
    ret = sigaction(sig, &act, NULL);

    return ret;
}

static
void signal_act(siginfo_t *info)
{
    int sig = info->si_signo;

    if (signal_actions[sig].act.sa_handler == SIG_IGN)
    {
        /* ignore */
    }
    else if (signal_actions[sig].act.sa_handler == SIG_DFL)
    {
        /* TODO */
    }
    else if (signal_actions[sig].act.sa_flags & SA_SIGINFO)
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

int sigpending (sigset_t *set)
{
    int iqueue;
    sigset_t blocked;
    int cs_state;
    
    sigemptyset(set);

    cs_state = critical_section_begin();

    sigprocmask(SIG_SETMASK, NULL, &blocked);

    for (iqueue = 0; iqueue < SIGQUEUE_MAX; iqueue++)
    {
        int sig;

        sig = signal_queue.items[iqueue].info.si_signo;
        if ( (sig != 0) && sigismember(&blocked, sig) )
        {
            sigaddset(set, sig);
        }

    }

    critical_section_end(cs_state);

    return 0;
}

int sigsuspend(const sigset_t *sigmask)
{
    sigset_t saved_mask;

    sigprocmask(SIG_SETMASK, sigmask, &saved_mask);
    
    /* TODO: wait */

    sigprocmask(SIG_SETMASK, &saved_mask, NULL);

    errno = EINTR;
    return -1;
}

int sigtimedwait(
        const sigset_t *restrict set,
        siginfo_t *restrict info,
        const struct timespec *restrict timeout)
{
    int ret;
    struct timespec start;

    clock_gettime(CLOCK_MONOTONIC, &start);

    while(signal_dequeue(set, info) != 0)
    {
        struct timespec now;
        struct timespec elapsed;

        /* TODO: wait event or timeout */

        clock_gettime(CLOCK_MONOTONIC, &now);
        timespec_diff(&now, &start, &elapsed);
        if (timespec_diff(timeout, &elapsed, NULL) <= 0)
        {
            errno = EAGAIN;
            ret = -1;
            break;
        }

    }

    return ret;
}

int sigwaitinfo(
        const sigset_t *restrict set,
        siginfo_t *restrict info)
{
    return sigtimedwait(set, info, &TIMESPEC_INFINITY);
}

int sigwait(const sigset_t *restrict set, int *restrict sig)
{
    int ret;

    siginfo_t info;

    ret = sigwaitinfo(set, &info);
    if (ret == 0)
    {
        *sig = info.si_signo;
    }

    return ret;
}

static
void pendsv_interrupt_raise(void)
{
    SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

void pend_sv_handler(void)
{
    siginfo_t info;

    while (signal_dequeue_procmask(&info) == 0)
    {
        signal_act(&info);
    }
}

