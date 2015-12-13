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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <file.h>
#include <limits.h>

#ifndef OPEN_MAX
/* We redefine OPEN_MAX here: its definition is what happens in this C source file. */
#  ifdef _POSIX_OPEN_MAX
#    warning "We needed OPEN_MAX from limits.h; using _POSIX_OPEN_MAX"
#    define OPEN_MAX _POSIX_OPEN_MAX
#  else
#    warning "We needed OPEN_MAX from limits.h;  assuming 20 which is _POSIX_OPEN_MAX in POSIX.1-2008"
#    define OPEN_MAX 20
#  endif
#endif

static
struct fd files[OPEN_MAX];

struct fd *file_struct_get(int fd)
{
    struct fd *f;

    if (fd >= OPEN_MAX)
    {
        f = NULL;
    }
    else
    {
        f = &files[fd];
    }
    return f;
}

int file_alloc(void)
{
    int fd;
    int ret = -1;

    /* starting from 3 because of STDIN, STDOUT, STDERR */
    for (fd = 3; fd < OPEN_MAX; fd++)
    {
        if (!files[fd].isallocated)
        {
            memset(&files[fd], 0, sizeof(files[fd]));
            
            files[fd].isallocated = 1;
            files[fd].fd = fd;
            ret = fd;
            break;
        }
    }
    return ret;
}

void file_free(int fd)
{
    if ((fd < OPEN_MAX) && (fd >= 0) && (files[fd].isallocated))
    {
        files[fd].isallocated = 0;
    }
}

