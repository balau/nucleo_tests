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
#ifndef DIRENT_H
#define DIRENT_H

typedef struct dirstream DIR;

#include <sys/types.h>

struct dirent {
    ino_t  d_ino; /* File serial number. */
    char   d_name[]; /* Filename string of entry. */
};

int alphasort(
        const struct dirent **,
        const struct dirent **);

int closedir(DIR *);

int dirfd(DIR *);

DIR *fdopendir(int);

DIR *opendir(const char *);

struct dirent *readdir(DIR *);

int  readdir_r(
        DIR *__restrict,
        struct dirent *__restrict,
        struct dirent **__restrict);

void rewinddir(DIR *);

int  scandir(
        const char *,
        struct dirent ***,
        int (*)(const struct dirent *),
        int (*)(const struct dirent **, const struct dirent **));

void seekdir(DIR *, long);

long telldir(DIR *);

#endif /* DIRENT_H */

