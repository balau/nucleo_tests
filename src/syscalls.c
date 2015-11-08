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

int _fstat(int fd, struct stat *buf);
int _write (int fd, char *ptr, int len);
int _read (int fd, char *ptr, int len);
void * _sbrk (ptrdiff_t incr);
int _close(int fd);
int _isatty(int fd);
_off_t _lseek(int fd, _off_t offset, int whence );
pid_t _getpid(void);
int _kill(pid_t pid, int sig);

int _write (int fd, char *ptr, int len)
{
    int ret;
    struct fd *f;
    
    f = file_struct_get(fd);
    if (f == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!f->isopen)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (f->write == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        ret = f->write(fd, ptr, len);
    }
    return ret;
}

int _read (int fd, char *ptr, int len)
{
    int ret;
    struct fd *f;
    
    f = file_struct_get(fd);
    if (f == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!f->isopen)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (f->read == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        ret = f->read(fd, ptr, len);
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
    struct fd *f;
    
    f = file_struct_get(fd);
    if (f == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!f->isopen)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (f->close != NULL)
    {
        ret = f->close(fd);
    }
    else
    {
        /* TODO */
        f->isopen = 0;
        ret = 0;
    }
    return ret;
}

int _fstat(int fd, struct stat *buf)
{
    int ret;
    struct fd *f;
    
    f = file_struct_get(fd);
    if (f == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!f->isopen)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        *buf = f->stat;
        ret = 0;
    }
    return ret;
}

int _isatty(int fd)
{
    int ret;
    struct fd *f;
    
    f = file_struct_get(fd);
    if (f == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!f->isopen)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (f->isatty)
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
    struct fd *f;
    
    f = file_struct_get(fd);
    if (f == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (!f->isopen)
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

void _exit(int code)
{
    (void)code;
    while(1)
    {
        continue; /* infinite loop */
    }
}

pid_t _getpid(void)
{
    return 1;
}

int _kill(pid_t pid, int sig)
{
    /* do nothing */
    (void)pid;
    (void)sig;
    errno = EPERM;
    return -1;
}

