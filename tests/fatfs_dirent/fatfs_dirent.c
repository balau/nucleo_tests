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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

static
void wait_enter(void)
{
    int c;

    do {
        c = getchar();
    } while ((c != '\n') && (c != '\r'));
}

static
const char dirpath[] = "dirtest";

static
const char dir2path[] = "dirtest/subdir";

static
int test_create(void)
{
    int result;
    struct stat s;

    result = mkdir(dirpath, S_IRWXU | S_IRWXG | S_IRWXO);
    if (result != 0)
    {
        if (errno == EEXIST)
        {
            printf("Directory already exists. Continuing...\n");
        }
        else
        {
            perror(dirpath);
            return 1;
        }
    }

    result = stat(dirpath, &s);
    if (result != 0)
    {
        perror(dirpath);
        return 1;
    }
    if (!S_ISDIR(s.st_mode))
    {
        fprintf(stderr, "%s: not a directory.\n", dirpath);
        return 1;
    }
    result = stat(dirpath, &s);
    if (result != 0)
    {
        perror(dirpath);
        return 1;
    }
    if (!S_ISDIR(s.st_mode))
    {
        fprintf(stderr, "%s: not a directory.\n", dirpath);
        return 1;
    }

    result = mkdir(dir2path, S_IRWXU | S_IRWXG | S_IRWXO);
    if (result != 0)
    {
        if (errno == EEXIST)
        {
            printf("Directory already exists. Continuing...\n");
        }
        else
        {
            perror(dir2path);
            return 1;
        }
    }

    return 0;
}

static
int test_chdir(void)
{
    int result;
    char cwd[256];

    printf("cwd = %s\n", getcwd(cwd, sizeof(cwd)));
    result = chdir(dirpath);
    if (result != 0)
    {
        perror(dirpath);
        return 1;
    }
    printf(
            "cd %s\n"
            "cwd = %s\n",
            dirpath,
            getcwd(cwd, sizeof(cwd)));
    result = chdir("..");
    if (result != 0)
    {
        perror(dirpath);
        return 1;
    }
    printf(
            "cd ..\n"
            "cwd = %s\n", getcwd(cwd, sizeof(cwd)));

    return 0;
}

static
int test_ls(DIR *d)
{
    long loc;
    struct dirent *entry;

    rewinddir(d);
    loc = telldir(d);
    printf("loc: %ld\n", loc);
    do
    {
        errno = 0; /* to recognize end of entries from errors */
        entry = readdir(d);
        if (entry != NULL)
        {
            printf("entry: %s (%u)\n", entry->d_name, entry->d_ino);
            loc = telldir(d);
            printf("loc: %ld\n", loc);
        }
        else
        {
            if (errno != 0)
            {
                perror(dirpath);
                return 1;
            }
        }
    } while (entry != NULL);

    return 0;
}

static
int test_ls_r(DIR *d)
{
    int result;
    long loc;
    struct dirent *entry;

    rewinddir(d);
    loc = telldir(d);
    printf("loc: %ld\n", loc);
    do
    {
        struct {
            ino_t d_ino;
            char d_name[NAME_MAX+1];
        } entry_storage;

        result = readdir_r(d, (struct dirent *)&entry_storage, &entry);
        if (result != 0)
        {
            perror(dirpath);
            return 1;
        }
        else if (entry != NULL)
        {
            printf("entry: %s (%u)\n", entry->d_name, entry->d_ino);
            loc = telldir(d);
            printf("loc: %ld\n", loc);
        }
    } while (entry != NULL);

    return 0;
}

static
int test_list(void)
{
    int result;
    DIR *d;

    d = opendir(dirpath);
    if (d == NULL)
    {
        perror(dirpath);
        return 1;
    }
    result = test_ls(d);
    if (result != 0)
    {
        return result;
    }
    result = test_ls_r(d);
    if (result != 0)
    {
        return result;
    }
    result = closedir(d);
    if (result != 0)
    {
        perror(dirpath);
        return 1;
    }

    return 0;
}

static
int test_remove(void)
{
    int result;
    struct stat s;

    result = rmdir(dirpath);
    if (result == 0)
    {
        fprintf(stderr, "%s: removed not empty directory.\n", dirpath);
        return 1;
    }
    else if ((errno != EEXIST) && (errno != ENOTEMPTY))
    {
        perror(dirpath);
        return 1;
    }
    result = rmdir(dir2path);
    if (result != 0)
    {
        perror(dir2path);
        return 1;
    }
    result = rmdir(dirpath);
    if (result != 0)
    {
        perror(dirpath);
        return 1;
    }

    result = stat(dirpath, &s);
    if (result == 0)
    {
        fprintf(stderr, "%s: still exists.\n", dirpath);
        return 1;
    }
    else if (errno != ENOENT)
    {
        perror(dirpath);
        return 1;
    }

    return 0;
}

int main(void)
{
    int result;

    printf(
            "fatfs_dirent\n"
            "Press Enter to continue...\n");
    wait_enter();

    result = test_create();
    if (result != 0)
    {
        return result;
    }
    result = test_chdir();
    if (result != 0)
    {
        return result;
    }
    result = test_list();
    if (result != 0)
    {
        return result;
    }
    result = test_remove();
    if (result != 0)
    {
        return result;
    }

    printf("Done.\n");

    return 0;
}

