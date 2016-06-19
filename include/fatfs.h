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
#ifndef FATFS_H
#define FATFS_H

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

extern
int fatfs_open(const char *pathname, int flags);

extern
off_t fatfs_lseek(int fd, off_t offset, int whence );

extern
int fatfs_unlink(const char *path);

extern
int fatfs_link(const char *path1, const char *path2);

extern
int fatfs_rename(const char *old, const char *new);

extern
int fatfs_fsync(int fd);

extern
int fatfs_stat(const char *path, struct stat *buf);

extern
int fatfs_mkdir(const char *path, mode_t mode);

extern
int fatfs_rmdir(const char *path);

extern
int fatfs_chdir(const char *path);

extern
char *fatfs_getcwd(char *buf, size_t size);

extern
DIR *fatfs_opendir(const char *path);

extern
int fatfs_closedir(DIR *dirp);

extern
struct dirent *fatfs_readdir(DIR *);

extern
int  fatfs_readdir_r(
        DIR *__restrict,
        struct dirent *__restrict,
        struct dirent **__restrict);

extern
void fatfs_rewinddir(DIR *);

extern
long fatfs_telldir(DIR *);

extern
int fatfs_dirfd(DIR *);

extern
DIR *fatfs_fdopendir(int fd);

extern
void fatfs_seekdir(DIR *, long);

#endif /* FATFS_H */

