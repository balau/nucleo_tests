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
#include <dirent.h>
#include "fatfs.h"
#include "file.h"

#define DIR FFDIR
#include "ff.h"
#undef DIR

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

struct dirent_storage {
    ino_t d_ino;
    char d_name[NAME_MAX+1];
};

struct dirstream {
    int fd;
    long loc;
    struct dirent_storage cur_entry;
    FFDIR ffdir;
};

/* typedef struct dirstream DIR in dirent.h */

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
    else if (S_ISREG(pfd->stat.st_mode))
    {
        FIL *filp;
        FRESULT result;
        UINT written;

        filp = pfd->opaque;

        if (pfd->status_flags & O_APPEND)
        {
            DWORD size;

            size = f_size(filp);
            result = f_lseek(filp, size);
        }
        else
        {
            result = FR_OK;
        }

        if (result == FR_OK)
        {
            result = f_write(filp, ptr, len, &written);
        }
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
    else if (S_ISDIR(pfd->stat.st_mode))
    {
        errno = EISDIR;
        ret = -1;
    }
    else
    {
        errno = EBADF;
        ret = -1;
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
    else if (S_ISREG(pfd->stat.st_mode))
    {
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
    else if (S_ISDIR(pfd->stat.st_mode))
    {
        errno = EISDIR;
        ret = -1;
    }
    else
    {
        errno = EBADF;
        ret = -1;
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

        result = f_closedir(&dp->ffdir);
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
void fill_stat(const FILINFO *fno, struct stat *out)
{
    mode_t mode;

    memset(out, 0, sizeof(struct stat));

    out->st_size = fno->fsize;
    if ((fno->fattrib & AM_MASK) & AM_DIR)
    {
        mode = S_IFDIR;
    }
    else
    {
        mode = S_IFREG;
    }
    mode |= (S_IRUSR|S_IRGRP|S_IROTH);
    mode |= (S_IXUSR|S_IXGRP|S_IXOTH);
    if (!((fno->fattrib & AM_MASK) & AM_RDO))
    {
        /* rwxrwxrwx */
        mode |= (S_IWUSR|S_IWGRP|S_IWOTH);
    }
    else
    {
        /* r-xr-xr-x */
    }
    out->st_mode = mode;
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
void fill_fd(struct fd *pfd, int flags, const FILINFO *fno)
{
    pfd->isatty = 0;
    pfd->isopen = 1;
    pfd->close = fatfs_close;
    pfd->status_flags = flags;
    pfd->descriptor_flags = 0;
    if (!((fno->fattrib & AM_MASK) & AM_DIR))
    {
        pfd->write = fatfs_write;
        pfd->read = fatfs_read;
    }

    fill_stat(fno, &pfd->stat);
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
        FILINFO fno_after;

        mode = flags2mode(flags);
        result = f_open(fp, pathname, mode);
        if ((result == FR_OK) && (fno->fname[0] == '\0'))
        {
            result = f_stat(pathname, &fno_after);
            fno = &fno_after;
        }
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
    DIR *dp;

    dp = fatfs_dir_alloc();

    if (dp == NULL)
    {
        result = FR_TOO_MANY_OPEN_FILES;
    }
    else
    {
        result = f_opendir(&dp->ffdir, pathname);
        if (result == FR_OK)
        {
            dp->fd = fildes;
            dp->loc = 0;
            fill_fd_dir(fildes, dp, flags, fno);
        }
        else
        {
            fatfs_dir_free(dp);
        }
    }

    return result;
}

static
FRESULT fatfs_open_file_or_dir(const char *pathname, int flags, int fildes)
{
    FRESULT result;
    FILINFO fno;

    fno.fname[0] = '\0'; /* initialize as invalid */
    result = f_stat(pathname, &fno);
    if ((result != FR_OK) && (result != FR_NO_FILE))
    {
        /* just return */
    }
    else if ((result == FR_OK) && ((fno.fattrib & AM_MASK) & AM_DIR))
    {
        result = fatfs_open_dir(pathname, flags, fildes, &fno);
    }
    else
    {
        result = fatfs_open_file(pathname, flags, fildes, &fno);
    }

    return result;
}

static
int is_dir(DIR *dirp)
{
    int ret;
    struct fd *pfd;

    pfd = file_struct_get(dirp->fd);
    if (pfd == NULL)
    {
        errno = EBADF;
        ret = 0;
    }
    else if (S_ISDIR(pfd->stat.st_mode))
    {
        ret = 1;
    }
    else
    {
        errno = ENOTDIR;
        ret = 0;
    }

    return ret;
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

off_t fatfs_lseek(int fd, off_t offset, int whence )
{
    off_t ret;
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
        DWORD pos;

        filp = pfd->opaque;
        if (whence == SEEK_CUR)
        {
            pos = f_tell(filp);
        }
        else if (whence == SEEK_END)
        {
            pos = f_size(filp);
        }
        else if (whence == SEEK_CUR)
        {
            pos = 0;
        }
        else
        {
            /* TODO: error */
            pos = 0;
        }
        pos += offset;

        result = f_lseek(filp, pos);
        if (result == FR_OK)
        {
            ret = pos;
        }
        else
        {
            errno = fresult2errno(result);
            ret = -1;
        }
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

int fatfs_unlink(const char *path)
{
    int ret;
    FRESULT result;

    result = f_unlink(path);
    if (result == FR_OK)
    {
        ret = 0;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

int fatfs_link(const char *path1, const char *path2)
{
    int ret;

    errno = ENOSYS;
    ret = -1;

    return ret;
}

int fatfs_fsync(int fd)
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

        result = f_sync(filp);
        if (result == FR_OK)
        {
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
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

int fatfs_stat(const char *path, struct stat *buf)
{
    int ret;
    FRESULT result;
    FILINFO fno;

    result = f_stat(path, &fno);
    if (result == FR_OK)
    {
        fill_stat(&fno, buf);
        ret = 0;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

int fatfs_mkdir(const char *path, mode_t mode)
{
    int ret;
    FRESULT result;

    (void)mode; /* ignored */
    result = f_mkdir(path);
    if (result == FR_OK)
    {
        ret = 0;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

int fatfs_rmdir(const char *path)
{
    int ret;
    FRESULT result;

    result = f_unlink(path);
    if (result == FR_OK)
    {
        ret = 0;
    }
    else if (result == FR_DENIED)
    {
        FILINFO fno;

        /* the dir was readonly or not empty: check */
        result = f_stat(path, &fno);
        if (result == FR_OK)
        {
            if ((fno.fattrib & AM_MASK) & AM_RDO)
            {
                errno = EACCES;
            }
            else
            {
                errno = ENOTEMPTY;
            }
        }
        else
        {
            errno = fresult2errno(result);
        }
        ret = -1;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

int fatfs_chdir(const char *path)
{
    int ret;
    FRESULT result;

    result = f_chdir(path);
    if (result == FR_OK)
    {
        ret = 0;
    }
    else
    {
        errno = fresult2errno(result);
        ret = -1;
    }

    return ret;
}

char *fatfs_getcwd(char *buf, size_t size)
{
    char *ret;
    FRESULT result;

    result = f_getcwd(buf, size);
    if (result == FR_OK)
    {
        ret = buf;
    }
    else
    {
        errno = fresult2errno(result);
        ret = NULL;
    }

    return ret;
}

DIR *fatfs_opendir(const char *path)
{
    DIR *ret;
    int result;

    result = fatfs_open(path, O_RDONLY);
    if (result == -1)
    {
        ret = NULL;
    }
    else
    {
        struct fd *pfd;

        pfd = file_struct_get(result);
        if (pfd == NULL)
        {
            errno = EBADF;
            ret = NULL;
        }
        else
        {
            ret = pfd->opaque;
        }
    }

    return ret;
}

int fatfs_closedir(DIR *dirp)
{
    return fatfs_close(dirp->fd);
}

int fatfs_dirfd(DIR *dirp)
{
    int ret;

    if (is_dir(dirp))
    {
        ret = dirp->fd;
    }
    else
    {
        errno = EINVAL;
        ret = -1;
    }

    return ret;
}

DIR *fatfs_fdopendir(int fd)
{
    DIR *ret;
    struct fd *pfd;

    pfd = file_struct_get(fd);
    if (pfd == NULL)
    {
        errno = EBADF;
        ret = NULL;
    }
    else if (S_ISDIR(pfd->stat.st_mode))
    {
        ret = pfd->opaque;
    }
    else
    {
        errno = ENOTDIR;
        ret = NULL;
    }

    return ret;
}

struct dirent *fatfs_readdir(DIR *dirp)
{
    struct dirent *ret;

    if (is_dir(dirp))
    {
        (void)fatfs_readdir_r(dirp, (struct dirent *)&dirp->cur_entry, &ret);
        /* ignore return value */
    }
    else
    {
        errno = EBADF;
        ret = NULL;
    }

    return ret;
}

int fatfs_readdir_r(
        DIR *dirp,
        struct dirent *entry,
        struct dirent **result)
{
    int ret;

    if (!is_dir(dirp))
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        FRESULT fresult;
        FILINFO fno;

        fresult = f_readdir(&dirp->ffdir, &fno);
        if (fresult != FR_OK)
        {
            errno = fresult2errno(fresult);
            ret = -1;
            *result = NULL;
        }
        else if (fno.fname[0] == '\0')
        {
            /* end of entries */
            ret = 0;
            *result = NULL;
        }
        else
        {
            ret = 0;
            dirp->loc++;
            entry->d_ino = dirp->loc; /* TODO: meaningful number */
            strncpy(entry->d_name, fno.fname, NAME_MAX+1);
            *result = entry;
        }
    }

    return ret;
}

void fatfs_rewinddir(DIR *dirp)
{
    if (!is_dir(dirp))
    {
        /* POSIX says no errors are defined */
    }
    else
    {
        int result;

        result = f_readdir(&dirp->ffdir, NULL);
        if (result == FR_OK)
        {
            dirp->loc = 0;
        }
        else
        {
            /* POSIX says no errors are defined */
        }
    }
}

long fatfs_telldir(DIR *dirp)
{
    long ret;

    if (is_dir(dirp))
    {
       ret = dirp->loc;
    }
    else
    {
        /* POSIX says no errors are defined */
        ret = -1;
    }

    return ret;
}

void fatfs_seekdir(DIR *dirp, long loc)
{
    if (is_dir(dirp))
    {
        long cur_loc;

        cur_loc = fatfs_telldir(dirp);
        if (loc < cur_loc)
        {
            fatfs_rewinddir(dirp);
        }
        while(loc > cur_loc)
        {
            int ret;
            struct dirent entry;
            struct dirent *result;

            ret = fatfs_readdir_r(dirp, &entry, &result);
            if ((result == NULL) || (ret != 0))
            {
                /* POSIX says no errors are defined */
                break;
            }
            cur_loc = fatfs_telldir(dirp);
        }
    }
    else
    {
        /* POSIX says no errors are defined */
    }
}

__attribute__((constructor))
void fatfs_init(void)
{
    FRESULT result;

    result = f_mount(&fs, "/", 1);
    (void)result; /* ignored */
}

