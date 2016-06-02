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

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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
    const char *filepath = "rwtest.txt";
    int fdout;
    int fdin;
    char message[80];
    int result;

    printf(
            "fatfs_unistd\n"
            "Press Enter to continue...\n");
    wait_enter();

    printf("Write a message:\n");
    scanf("%s", message);
    printf("Writing message to %s...\n", filepath);

    fdout = open(filepath, O_WRONLY|O_TRUNC|O_CREAT);
    if (fdout == -1)
    {
        perror(filepath);
        return 1;
    }
    else
    {
        result = write(fdout, message, strlen(message));
        if (result < 0)
        {
            perror(filepath);
            return 1;
        }
    }
    result = fsync(fdout);
    if (result != 0)
    {
        perror(filepath);
        return 1;
    }
    printf("\npos = %ld.\n", lseek(fdout, 0, SEEK_CUR));
    result = close(fdout);
    if (result != 0)
    {
        perror(filepath);
        return 1;
    }
    printf("Done.\n"
           "Reading message from %s...\n", filepath);

    fdin = open(filepath, O_RDONLY);
    if (fdout == -1)
    {
        perror(filepath);
        return 1;
    }
    else
    {
        ssize_t nbytes;

        do {
            char *p;

            nbytes = read(fdin, message, sizeof(message));
            if(nbytes == -1)
            {
                perror(filepath);
                close(fdin);
                return 1;
            }
            if (nbytes == 0)
            {
                /* EOF */
                break;
            }
            p = message;
            while (nbytes > 0)
            {
                size_t nwritten;

                nwritten = fwrite(p, 1, nbytes, stdout);
                if (nwritten > 0)
                {
                    nbytes -= nwritten;
                    p += nwritten;
                }
            }
        } while(1);
    }
    printf("\npos = %ld.\n", lseek(fdin, 0, SEEK_CUR));
    result = close(fdin);
    if (result != 0)
    {
        perror(filepath);
        return 1;
    }
    printf(
            "Done.\n"
            "Removing file...\n");

    result = unlink(filepath);
    if (result != 0)
    {
        perror(filepath);
        return 1;
    }
    printf("Done.\n");

    return 0;
}

