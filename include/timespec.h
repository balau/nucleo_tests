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
#ifndef TIMESPEC_H
#define TIMESPEC_H

#include <time.h>
#include <sys/time.h>

#define NSECS_IN_SEC 1000000000

#define USECS_IN_SEC 1000000

#define MSECS_IN_SEC 1000

extern
void timespec_to_timeval(const struct timespec *src, struct timeval *dst);

extern
void timeval_to_timespec(const struct timeval *src, struct timespec *dst);

extern
void timespec_add(const struct timespec *t1, const struct timespec *t2, struct timespec *dst);

extern
void timespec_incr(struct timespec *x, const struct timespec *step);

extern
int timespec_diff(const struct timespec *to, const struct timespec *from, struct timespec *diff);

extern
const struct timespec TIMESPEC_ZERO;

extern
const struct timespec TIMESPEC_INFINITY;

#endif /* TIMESPEC_H */

