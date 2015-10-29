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

#ifndef NFILES_MAX
#  define NFILES_MAX 16
#endif

static
struct fd files[NFILES_MAX];

struct fd *syscall_get_file_struct(int fd)
{
    struct fd *f;

    if (fd >= NFILES_MAX)
    {
        f = NULL;
    }
    else
    {
        f = &files[fd];
    }
    return f;
}

int syscall_falloc(void)
{
    int fd;
    int ret = -1;

    /* starting from 3 because of STDIN, STDOUT, STDERR */
    for (fd = 3; fd < NFILES_MAX; fd++)
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

void syscall_ffree(int fd)
{
    if ((fd < NFILES_MAX) && (fd >= 0) && (files[fd].isallocated))
    {
        files[fd].isallocated = 0;
    }
}

