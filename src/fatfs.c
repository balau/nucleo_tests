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
DIR *fatfs_dir_alloc(void);

static
void fatfs_dir_free(DIR *fp);

static
int fatfs_fildir_alloc(void);

static
int fatfs_fildir_free(void *p);

static
void fill_fd_fil(int fildes, FIL *fp, int flags, const FILINFO *fno);

static
void fill_fd_dir(int fildes, DIR *fp, int flags, const FILINFO *fno);

static
void fill_fd(struct fd *pfd, int flags, const FILINFO *fno);

/* static variables */

static FATFS fs;

static struct {
    int allocated;
    union
    {
        FIL fil;
        DIR dir;
    };
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
    int accmode;

    mode = 0;
    accmode = flags & O_ACCMODE;

    if ((accmode == O_RDONLY) || (accmode == O_RDWR))
    {
        mode |= FA_READ;
    }
    if ((accmode == O_WRONLY) || (accmode == O_RDWR))
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
int fatfs_fildir_alloc(void)
{
    int i_fil;

    for (i_fil = 0; i_fil < OPEN_MAX; i_fil++)
    {
        if (!files[i_fil].allocated)
        {
            files[i_fil].allocated = 1;
            break;
        }
    }
    if (i_fil == OPEN_MAX)
    {
        i_fil = -1;
    }
    return i_fil;
}

static
FIL *fatfs_fil_alloc(void)
{
    int i_fil;
    FIL *f;

    i_fil = fatfs_fildir_alloc();
    if (i_fil == -1)
    {
        f = NULL;
    }
    else
    {
        f = &files[i_fil].fil;
    }

    return f;
}

static
DIR *fatfs_dir_alloc(void)
{
    int i_fil;
    DIR *d;

    i_fil = fatfs_fildir_alloc();
    if (i_fil == -1)
    {
        d = NULL;
    }
    else
    {
        d = &files[i_fil].dir;
    }

    return d;
}

static
int fatfs_fildir_free(void *fp)
{
    int i_fil;

    for (i_fil = 0; i_fil < OPEN_MAX; i_fil++)
    {
        if (
                files[i_fil].allocated
                &&
                (
                 (fp == &files[i_fil].fil)
                 ||
                 (fp == &files[i_fil].dir)
                )
           )

        {
            files[i_fil].allocated = 0;
            break;
        }
    }
    if (i_fil == OPEN_MAX)
    {
        i_fil = -1;
    }

    return i_fil;
}

static
void fatfs_dir_free(DIR *fp)
{
    int i_fil;

    i_fil = fatfs_fildir_free(fp);
    if (i_fil != -1)
    {
        memset(&files[i_fil].dir, 0, sizeof(files[i_fil].dir));
    }
}

static
void fatfs_fil_free(FIL *fp)
{
    int i_fil;

    i_fil = fatfs_fildir_free(fp);
    if (i_fil != -1)
    {
        memset(&files[i_fil].fil, 0, sizeof(files[i_fil].fil));
    }
}

static
int fatfs_write (int fd, char *ptr, int len)
{
    int ret;
    struct fd *pfd;

    pfd = file_struct_get(fd);

    if (pfd == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (pfd->opaque == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        /* TODO: check also if file type is fatfs */

        FIL *filp;
        FRESULT result;
        UINT written;

        filp = pfd->opaque;

        /* TODO: manage append */

        result = f_write(filp, ptr, len, &written);
        if (result == FR_OK)
        {
            ret = written;
        }
        else
        {
            errno = fresult2errno(result);
            ret = -1;
        }
    }

    return ret;
}

static
int fatfs_read (int fd, char *ptr, int len)
{
    int ret;
    struct fd *pfd;

    pfd = file_struct_get(fd);

    if (pfd == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (pfd->opaque == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        /* TODO: check also if file type is fatfs */

        FIL *filp;
        FRESULT result;
        UINT nbytes_read;

        filp = pfd->opaque;

        result = f_read(filp, ptr, len, &nbytes_read);
        if (result == FR_OK)
        {
            ret = nbytes_read;
        }
        else
        {
            errno = fresult2errno(result);
            ret = -1;
        }
    }

    return ret;
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
    else if (pfd->opaque == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else if (S_ISREG(pfd->stat.st_mode))
    {
        FIL *filp;
        FRESULT result;

        filp = pfd->opaque;

        result = f_close(filp);
        if (result == FR_OK)
        {
            fatfs_fil_free(filp);
            file_free(fd);
            ret = 0;
        }
        else
        {
            errno = fresult2errno(result);
            ret = -1;
        }
    }
    else if (S_ISDIR(pfd->stat.st_mode))
    {
        DIR *dp;
        FRESULT result;

        dp = pfd->opaque;

        result = f_closedir(dp);
        if (result == FR_OK)
        {
            fatfs_dir_free(dp);
            file_free(fd);
            ret = 0;
        }
        else
        {
            errno = fresult2errno(result);
            ret = -1;
        }
    }
    else
    {
        errno = EBADF;
        ret = -1;
    }

    return ret;
}

static
void fill_fd(struct fd *pfd, int flags, const FILINFO *fno)
{
    mode_t mode;

    pfd->isatty = 0;
    pfd->isopen = 1;
    pfd->close = fatfs_close;
    pfd->status_flags = flags;
    pfd->descriptor_flags = 0;
    memset(&pfd->stat, 0, sizeof(struct stat));
    pfd->stat.st_size = fno->fsize;
    if ((fno->fattrib & AM_MASK) & AM_DIR)
    {
        mode = S_IFDIR;
    }
    else
    {
        mode = S_IFREG;
        pfd->write = fatfs_write;
        pfd->read = fatfs_read;
    }
    /* rwxrwxrwx or r-xr-xr-x */
    mode |= (S_IRUSR|S_IRGRP|S_IROTH);
    mode |= (S_IXUSR|S_IXGRP|S_IXOTH);
    if (!((fno->fattrib & AM_MASK) & AM_RDO))
    {
        mode |= (S_IWUSR|S_IWGRP|S_IWOTH);
    }
    pfd->stat.st_mode = mode;
#if 0
    /* not present in newlib struct stat */
    struct timespec ts;
    fattime_to_timespec(fno->fdate, fno->ftime, &ts);
    pfd->stat.st_atim = ts;
    pfd->stat.st_mtim = ts;
    pfd->stat.st_ctim = ts;
#endif
}

static
void fill_fd_fil(int fildes, FIL *fp, int flags, const FILINFO *fno)
{
    struct fd *pfd;

    pfd = file_struct_get(fildes);

    fill_fd(pfd, flags, fno);
    pfd->opaque = fp;
}

static
void fill_fd_dir(int fildes, DIR *fp, int flags, const FILINFO *fno)
{
    struct fd *pfd;

    pfd = file_struct_get(fildes);

    fill_fd(pfd, flags, fno);
    pfd->opaque = fp;
}

static
FRESULT fatfs_open_file(const char *pathname, int flags, int fildes, const FILINFO *fno)
{
    FRESULT result;
    BYTE mode;
    FIL *fp;

    fp = fatfs_fil_alloc();

    if (fp == NULL)
    {
        result = FR_TOO_MANY_OPEN_FILES;
    }
    else
    {
        mode = flags2mode(flags);
        result = f_open(fp, pathname, mode);
        if (result == FR_OK)
        {
            fill_fd_fil(fildes, fp, flags, fno);
        }
        else
        {
            fatfs_fil_free(fp);
        }
    }

    return result;
}

static
FRESULT fatfs_open_dir(const char *pathname, int flags, int fildes, const FILINFO *fno)
{
    FRESULT result;
    DIR *fp;

    fp = fatfs_dir_alloc();

    if (fp == NULL)
    {
        result = FR_TOO_MANY_OPEN_FILES;
    }
    else
    {
        result = f_opendir(fp, pathname);
        if (result == FR_OK)
        {
            fill_fd_dir(fildes, fp, flags, fno);
        }
        else
        {
            fatfs_dir_free(fp);
        }
    }

    return result;
}

static
FRESULT fatfs_open_file_or_dir(const char *pathname, int flags, int fildes)
{
    FRESULT result;
    FILINFO fno;

    result = f_stat(pathname, &fno);
    if (result != FR_OK)
    {
        /* just return */
    }
    else if ((fno.fattrib & AM_MASK) & AM_DIR)
    {
        result = fatfs_open_dir(pathname, flags, fildes, &fno);
    }
    else
    {
        result = fatfs_open_file(pathname, flags, fildes, &fno);
    }

    return result;
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
    int fildes;
    FRESULT result;

    fildes = file_alloc();
    if (fildes < 0)
    {
        errno = ENFILE;
        ret = -1;
    }
    else
    {
        result = fatfs_open_file_or_dir(pathname, flags, fildes);
        if (result == FR_OK)
        {
            ret = fildes;
        }
        else
        {
            errno = fresult2errno(result);
            file_free(fildes);
            ret = -1;
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

