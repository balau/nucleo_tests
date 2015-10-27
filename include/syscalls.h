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
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <sys/types.h>
#include <sys/stat.h>

struct fd {
    struct stat stat;
    int isatty;
    int isopen;
    int (*write)(int, char*, int);
    int (*read)(int, char*, int);
    int (*close)(int);
    int isallocated;
    void *opaque;
};

extern
struct fd *syscall_get_file_struct(int fd);

extern
int syscall_falloc(void);

extern
void syscall_ffree(int fd);

#endif

