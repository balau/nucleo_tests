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
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include "fatfs.h"
#include "file.h"
#include "ff.h"

/* Macro definitions */

/* Function prototypes */

extern
void fatfs_init(void);

static
int fatfs_write (int fd, char *ptr, int len);

static
int fatfs_read (int fd, char *ptr, int len);

static
int fatfs_close (int fd);

static
BYTE flags2mode(int flags);

static
int fresult2errno(FRESULT result);

static
FIL *fatfs_fil_alloc(void);

static
void fatfs_fil_free(FIL *fp);

static
void fill_fd(int fildes, FIL *fp);

/* static variables */

static FATFS fs;

static struct {
    int allocated;
    FIL fil;
    } files[OPEN_MAX];

/* static functions */

static
int fresult2errno(FRESULT result)
{
    int err;

    switch (result)
    {
        case FR_OK:
            err = 0;
            break;
        case FR_EXIST:
            err = EEXIST;
            break;
        case FR_NO_FILE:
            err = ENOENT;
            break;
        case FR_NO_PATH:
        case FR_INVALID_NAME:
            err = ENOTDIR;
            break;
        case FR_WRITE_PROTECTED:
            err = EROFS;
            break;
        case FR_DENIED:
            err = EACCES;
            break;
        case FR_TOO_MANY_OPEN_FILES:
            err = ENFILE;
            break;
        case FR_NOT_ENOUGH_CORE:
            err = ENOMEM;
            break;
        case FR_DISK_ERR:
        case FR_INT_ERR:
        case FR_NOT_READY:
        case FR_INVALID_OBJECT:
        case FR_INVALID_DRIVE:
        case FR_NOT_ENABLED:
        case FR_NO_FILESYSTEM:
        case FR_MKFS_ABORTED:
        case FR_TIMEOUT:
        case FR_LOCKED:
        case FR_INVALID_PARAMETER:
        /* TODO */
        default:
            err = EINVAL;
            break;
    }

    return err;
}

static
BYTE flags2mode(int flags)
{
    BYTE mode;

    mode = 0;

    if ((flags & O_RDONLY) || (flags & O_RDWR))
    {
        mode |= FA_READ;
    }
    if ((flags & O_WRONLY) || (flags & O_RDWR))
    {
        mode |= FA_WRITE;

        if (!(flags & O_CREAT))
        {
            mode |= FA_OPEN_EXISTING;
        }
        else if (flags & O_EXCL)
        {
            mode |= FA_CREATE_NEW;
        }
        else if (flags & O_TRUNC)
        {
            mode |= FA_CREATE_ALWAYS;
        }
        else
        {
            mode |= FA_OPEN_ALWAYS;
        }
    }

    return mode;
}

static
FIL *fatfs_fil_alloc(void)
{
    int i_fil;
    FIL *f;

    f = NULL;
    for (i_fil = 0; i_fil < OPEN_MAX; i_fil++)
    {
        if (!files[i_fil].allocated)
        {
            files[i_fil].allocated = 1;
            f = &files[i_fil].fil;
            break;
        }
    }

    return f;
}

static
void fatfs_fil_free(FIL *fp)
{
    /* TODO */
    int i_fil;

    for (i_fil = 0; i_fil < OPEN_MAX; i_fil++)
    {
        if (files[i_fil].allocated && (fp == &files[i_fil].fil))
        {
            files[i_fil].allocated = 0;
            memset(&files[i_fil].fil, 0, sizeof(files[i_fil].fil));
            break;
        }
    }

}

static
int fatfs_write (int fd, char *ptr, int len)
{
    /* TODO */
    errno = EINVAL;
    return -1;
}

static
int fatfs_read (int fd, char *ptr, int len)
{
    /* TODO */
    errno = EINVAL;
    return -1;
}

static
int fatfs_close (int fd)
{
    int ret;
    struct fd *pfd;

    pfd = file_struct_get(fd);

    if (pfd == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        /* TODO: check also if file type is fatfs */

        FIL *filp;

        filp = pfd->opaque;

        fatfs_fil_free(filp);
        file_free(fd);

        ret = 0;
    }

    return ret;
}

static
void fill_fd(int fildes, FIL *fp)
{
    struct fd *pfd;
    struct stat s;

    pfd = file_struct_get(fildes);

    /* TODO: fill stat */
    pfd->isatty = 0;
    pfd->isopen = 1;
    pfd->write = fatfs_write;
    pfd->read = fatfs_read;
    pfd->close = fatfs_close;
    pfd->opaque = fp;
}

/* exported functions */

/* TODO: maybe put in diskio? use real time? */
DWORD get_fattime (void)
{
    return (
            ((2015-1980)<<25)
            |
            (1<<21)
            |
            (1<<16)
           );
}

int fatfs_open(const char *pathname, int flags)
{
    int ret;
    FRESULT result;
    BYTE mode;
    FIL *fp;
    int fildes;

    mode = flags2mode(flags);

    fp = fatfs_fil_alloc();
    fildes = file_alloc();
    if (fp == NULL)
    {
        errno = EMFILE;
        ret = -1;
    }
    else if (fildes < 0)
    {
        errno = ENFILE;
        ret = -1;
    }
    else
    {
        result = f_open(fp, pathname, mode);
        if (result == FR_OK)
        {
            fill_fd(fildes, fp);
            ret = fildes;
        }
        else
        {
            errno = fresult2errno(result);
            fatfs_fil_free(fp);
            file_free(fildes);
            ret = -1;
        }
    }

    if (ret == -1)
    {
        if (fp != NULL)
        {
            fatfs_fil_free(fp);
        }
        if (fildes >= 0)
        {
            file_free(fildes);
        }
    }

    return ret;
}

__attribute__((constructor))
void fatfs_init(void)
{
    FRESULT result;

    result = f_mount(&fs, "/", 1);
    (void)result; /* ignored */
}

