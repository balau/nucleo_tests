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
#ifndef SYS_SELECT_H
#define SYS_SELECT_H

/* TODO: don't include this, instead define struct timeval here and
 * include sys/select.h in sys/time.h
 */
#include <sys/time.h>

#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <limits.h>

#ifndef OPEN_MAX
#  ifdef _POSIX_OPEN_MAX
#    warning "We needed OPEN_MAX to set FD_SETSIZE; using _POSIX_OPEN_MAX"
#    define FD_SETSIZE _POSIX_OPEN_MAX
#  else
#    warning "We needed OPEN_MAX to set FD_SETSIZE; assuming 20 which is _POSIX_OPEN_MAX in POSIX.1-2008"
#    define FD_SETSIZE 20
#  endif
#else
#  define FD_SETSIZE OPEN_MAX
#endif

struct fd_set_s {
    unsigned long mask[(FD_SETSIZE + (8*sizeof(unsigned long)) - 1) / (8*sizeof(unsigned long))];
};

typedef struct fd_set_s fd_set;

void FD_CLR(int, fd_set *);

int  FD_ISSET(int, fd_set *);

void FD_SET(int, fd_set *);

void FD_ZERO(fd_set *);

int  pselect(int, fd_set *__restrict, fd_set *__restrict, fd_set *__restrict,
         const struct timespec *__restrict, const sigset_t *__restrict);

int  select(int, fd_set *__restrict, fd_set *__restrict, fd_set *__restrict,
         struct timeval *__restrict);

#endif /* SYS_SELECT_H */

