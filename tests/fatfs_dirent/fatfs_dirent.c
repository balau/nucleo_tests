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

static
void wait_enter(void)
{
    int c;

    do {
        c = getchar();
    } while ((c != '\n') && (c != '\r'));
}

int main(void)
{
    const char *dirpath = "dirtest";
    int result;
    struct stat s;

    printf(
            "fatfs_dirent\n"
            "Press Enter to continue...\n");
    wait_enter();

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

    printf("Done.\n");

    return 0;
}

