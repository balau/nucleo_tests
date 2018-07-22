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
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include "timespec.h"
#include "sigqueue_info.h"

struct sys_timer
{
    timer_t timerid;
    clockid_t clockid;
    struct sigevent sevp;
    int flags;
    struct itimerspec value;
    int overrun;
};

static
struct sys_timer sys_timers[TIMER_MAX];

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

static
int sys_timerid2td(timer_t timerid)
{
    return (timerid - 1);
}

static
timer_t sys_td2timerid(int td)
{
    return (td + 1);
}

static
int sys_timer_isallocated(int td)
{
    return (sys_timers[td].timerid != 0);
}

static
void sys_timer_time_to_expiration(int td, struct timespec *t)
{
    struct timespec now;

    /* Assuming timer is allocated and armed */
    clock_gettime(sys_timers[td].clockid, &now);
    timespec_diff(&sys_timers[td].value.it_value, &now, t);
}

static
int sys_timer_isarmed(int td)
{
    int ret;

    if (!sys_timer_isallocated(td))
    {
        ret = 0;
    }
    else if (timespec_diff(&TIMESPEC_ZERO, &sys_timers[td].value.it_value, NULL) == 0)
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    return ret;
}

static
int sys_timer_alloc(void)
{
    int ret = -1;
    int td;

    for(td = 0; td < TIMER_MAX; td++)
    {
        if (!sys_timer_isallocated(td))
        {
            struct sys_timer *tinfo;
            
            tinfo = &sys_timers[td];
            memset(tinfo, 0, sizeof(struct sys_timer));
            tinfo->timerid = sys_td2timerid(td);
            ret = td;
            break;
        }
    }
    
    return ret;
}

static
int sys_timer_iter(int td)
{
    int ret = -1;

    if (td < 0)
    {
        td = 0;
    }
    else
    {
        td++;
    }

    for ( ; td < TIMER_MAX; td++)
    {
        if (sys_timer_isarmed(td))
        {
            ret = td;
            break;
        }
    }

    return ret;
}

static
int sys_timer_expires_before(int t1, int t2)
{
    int ret;
    struct timespec to_expiration_t1;
    struct timespec to_expiration_t2;

    sys_timer_time_to_expiration(t1, &to_expiration_t1);
    sys_timer_time_to_expiration(t2, &to_expiration_t2);

    if (timespec_diff(&to_expiration_t1, &to_expiration_t2, NULL) >= 0)
    {
        /* t2 expires before t1 */
        ret = 0;
    }
    else
    {
        /* t1 expires before t2 */
        ret = 1;
    }

    return ret;
}

static
int sys_timer_next_expiring(void)
{
    int titer;
    int td_expiring = -1;

    titer = sys_timer_iter(-1);
    while(titer != -1)
    {
        if (td_expiring == -1)
        {
            td_expiring = titer;
        }
        else if (sys_timer_expires_before(titer, td_expiring))
        {
            td_expiring = titer;
        }
        titer = sys_timer_iter(titer);
    }

    return td_expiring;
}

static
void hw_timer_init(void)
{
}

static
void hw_timer_start(const struct timespec *interval)
{
}

static
void hw_timer_stop(void)
{
}

static
void timer_expired(int td)
{
    siginfo_t info;
    int ret;

    switch(sys_timers[td].sevp.sigev_notify)
    {
        case SIGEV_NONE:
            /* Ignore */
            break;

        case SIGEV_SIGNAL:

            memset(&info, 0, sizeof(siginfo_t));
            info.si_value = sys_timers[td].sevp.sigev_value;
            info.si_signo = sys_timers[td].sevp.sigev_signo;
            info.si_code = SI_TIMER;
            ret = sigqueue_info(&info);
            /* TODO: ret != 0? */
            (void)ret;
            break;

        case SIGEV_THREAD:
            sys_timers[td].sevp.sigev_notify_function(sys_timers[td].sevp.sigev_value);
            break;
    }
    if (timespec_diff(&sys_timers[td].value.it_interval, &TIMESPEC_ZERO, NULL) > 0)
    {
        struct timespec now;
        
        /* Re-arm timer */
        clock_gettime(sys_timers[td].clockid, &now);
        timespec_add(
                &now,
                &sys_timers[td].value.it_interval,
                &sys_timers[td].value.it_value);
    }
    else
    {
        /* Disarm timer */
        sys_timers[td].value.it_value = TIMESPEC_ZERO;
    }
}

static
void sys_timers_manage(void)
{
    int td_expiring;
    int expired;

    do {
        td_expiring = sys_timer_next_expiring();
        if (td_expiring != -1)
        {
            struct timespec to_expiration;

            sys_timer_time_to_expiration(td_expiring, &to_expiration);
            if (timespec_diff(&to_expiration, &TIMESPEC_ZERO, NULL) <= 0)
            {
                timer_expired(td_expiring);
                expired = 1;
            }
            else
            {
                hw_timer_start(&to_expiration);
                expired = 0;
            }
        }
        else
        {
            hw_timer_stop();
            expired = 0;
        }
    } while(expired);
}

static
void sys_timer_free(int td)
{
    if ( (td >= 0) && (td < TIMER_MAX) )
    {
        sys_timers[td].timerid = 0;
    }
}

int timer_create(
        clockid_t clockid,
        struct sigevent *__restrict sevp,
        timer_t *__restrict timerid)
{
    int ret;
    int td;

    if ( (clockid == CLOCK_MONOTONIC) || (clockid == CLOCK_REALTIME) )
    {
        td = sys_timer_alloc();
        if (td != -1)
        {
            struct sys_timer *tinfo;
            
            tinfo = &sys_timers[td];
            tinfo->clockid = clockid;
            if (sevp != NULL)
            {
                tinfo->sevp = *sevp;
            }
            else
            {
                tinfo->sevp.sigev_notify = SIGEV_SIGNAL;
                tinfo->sevp.sigev_signo = SIGALRM;
                tinfo->sevp.sigev_value.sival_int = tinfo->timerid;
            }
            *timerid = tinfo->timerid;
            ret = 0;
        }
        else
        {
            errno = EAGAIN;
            ret = -1;
        }
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }
    
    return ret;
}

int timer_delete(timer_t timerid)
{
    int ret;
    int td;

    td = sys_timerid2td(timerid);
    if (sys_timer_isallocated(td))
    {
        /* TODO: stop timer? */
        sys_timer_free(td);
        ret = 0;
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

int timer_settime(
        timer_t timerid,
        int flags,
        const struct itimerspec *new_value,
        struct itimerspec *old_value)
{
    int ret;
    int td;

    td = sys_timerid2td(timerid);
    if (sys_timer_isallocated(td))
    {
        if (old_value != NULL)
        {
            if (sys_timer_isarmed(td))
            {
                sys_timer_time_to_expiration(td, &old_value->it_value);
                old_value->it_interval = sys_timers[td].value.it_interval;
            }
            else
            {
                *old_value = sys_timers[td].value;
            }
        }
        if (
                (timespec_diff(&new_value->it_value, &TIMESPEC_ZERO, NULL) != 0) &&
                (!(flags & TIMER_ABSTIME)) )
        {
            struct timespec now;

            clock_gettime(sys_timers[td].clockid, &now);
            timespec_add(
                    &now,
                    &new_value->it_value,
                    &sys_timers[td].value.it_value);
            sys_timers[td].value.it_interval = new_value->it_interval;
        }
        else
        {
            sys_timers[td].value = *new_value;
        }

        sys_timers_manage();
        ret = 0;
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

int timer_gettime(
        timer_t timerid,
        struct itimerspec *curr_value)
{
    int ret;
    int td;

    td = sys_timerid2td(timerid);
    if (sys_timer_isallocated(td))
    {
        if (sys_timer_isarmed(td))
        {
            sys_timer_time_to_expiration(td, &curr_value->it_value);
            curr_value->it_interval = sys_timers[td].value.it_interval;
        }
        else
        {
            *curr_value = sys_timers[td].value;
        }
        ret = 0;
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

int timer_getoverrun(timer_t timerid)
{
    int ret;
    int td;

    td = sys_timerid2td(timerid);
    if (sys_timer_isallocated(td))
    {
        ret = sys_timers[td].overrun;
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

