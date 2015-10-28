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

#include "syscalls.h"

int _fstat(int fd, struct stat *buf);
int _write (int fd, char *ptr, int len);
int _read (int fd, char *ptr, int len);
void * _sbrk (ptrdiff_t incr);
int _close(int fd);
int _isatty(int fd);
_off_t _lseek(int fd, _off_t offset, int whence );


#define NFILES_MAX 8

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

int _write (int fd, char *ptr, int len)
{
    int ret;

    if (fd >= NFILES_MAX)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!files[fd].isopen) /* TODO: check write permission */
    {
        errno = EBADF;
        ret = -1;
    }
    else if (files[fd].write == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        ret = files[fd].write(fd, ptr, len);
    }
    return ret;
}

int _read (int fd, char *ptr, int len)
{
    int ret;

    if (fd >= NFILES_MAX)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!files[fd].isopen) /* TODO: check read permission */
    {
        errno = EBADF;
        ret = -1;
    }
    else if (files[fd].read == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        ret = files[fd].read(fd, ptr, len);
    }
    return ret;
}

void * _sbrk (ptrdiff_t incr)
{
    extern uint8_t end;
    extern uint8_t _stack;
    static uint8_t *program_break = &end;
    const uint8_t *max_break = &_stack;

    void *ret;
    uint8_t *current_break;

    current_break = program_break;

    if (current_break + incr >= max_break)
    {
        errno = ENOMEM;
        ret = (void *)-1;
    }
    else
    {
        program_break += incr;
        ret = current_break;
    }
    return ret;
}

int _close(int fd)
{
    int ret;

    if (fd >= NFILES_MAX)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!files[fd].isopen)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (files[fd].close != NULL)
    {
        ret = files[fd].close(fd);
    }
    else
    {
        /* TODO */
        files[fd].isopen = 0;
        ret = 0;
    }
    return ret;
}

int _fstat(int fd, struct stat *buf)
{
    int ret;
    
    if (fd >= NFILES_MAX)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!files[fd].isopen)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        *buf = files[fd].stat;
        ret = 0;
    }
    return ret;
}

int _isatty(int fd)
{
    int ret;

    if (fd >= NFILES_MAX)
    {
        errno = EBADF;
        ret = 0;
    }
    else if (!files[fd].isopen)
    {
        errno = EBADF;
        ret = 0;
    }
    else if (files[fd].isatty)
    {
        ret = 1;
    }
    else
    {
        errno = ENOTTY;
        ret = 0;
    }
    return ret;
}

_off_t _lseek(int fd, _off_t offset, int whence )
{
    int ret;
    
    if (fd >= NFILES_MAX)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!files[fd].isopen)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        /* TODO */
        (void)offset;
        (void)whence;
        errno = ESPIPE;
        ret = -1;
    }
    return ret;
}

