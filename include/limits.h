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
#ifndef LIMITS_H
#define LIMITS_H

#include_next <limits.h>

#ifndef OPEN_MAX
#  ifdef _POSIX_OPEN_MAX
#    define OPEN_MAX _POSIX_OPEN_MAX
#  else
#    define OPEN_MAX 20
#  endif
#endif

#ifndef NAME_MAX
#  ifdef _POSIX_NAME_MAX
#    define NAME_MAX _POSIX_NAME_MAX
#  else
#    define NAME_MAX 12 /* 8+1+3 like FILENAME.EXT */
#  endif
#endif

#ifndef TIMER_MAX
#  ifdef _POSIX_TIMER_MAX
#    define TIMER_MAX _POSIX_TIMER_MAX
#  else
#    define TIMER_MAX 32
#  endif
#endif

#ifndef SIGQUEUE_MAX
#  ifdef _POSIX_SIGQUEUE_MAX
#    define SIGQUEUE_MAX _POSIX_SIGQUEUE_MAX
#  else
#    define SIGQUEUE_MAX 32
#  endif
#endif

#endif

