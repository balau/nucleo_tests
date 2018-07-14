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
#include "timespec.h"

/* TODO: ISR */
static void isr(void)
{
    // read clock
    // see if some timer has fired
    //   manage fired timer
    // check next timer
    // schedule next isr
}

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
    return 1;
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
            ret = tinfo->timerid;
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

    td = sys_td2timerid(timerid);
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

    td = sys_td2timerid(timerid);
    if (sys_timer_isallocated(td))
    {
        /* TODO: convert value ABS */
        if (old_value != NULL)
        {
            *old_value = sys_timers[td].value;
        }
        sys_timers[td].value = *new_value;
        /* TODO: arm ISR */
        ret = 0;
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return -1;
}

int timer_gettime(
        timer_t timerid,
        struct itimerspec *curr_value)
{
    int ret;
    int td;

    td = sys_td2timerid(timerid);
    if (sys_timer_isallocated(td))
    {
        /* TODO: convert value ABS */
        *curr_value = sys_timers[td].value;
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

    td = sys_td2timerid(timerid);
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

