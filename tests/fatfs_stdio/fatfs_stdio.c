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
    FILE *fout;
    FILE *fin;
    char message[80];
    int result;

    printf(
            "fatfs_stdio\n"
            "Press Enter to continue...\n");
    wait_enter();

    printf("Write a message:\n");
    scanf("%s", message);
    printf("Writing message to %s...\n", filepath);

    fout = fopen(filepath, "w");
    if (fout == NULL)
    {
        perror(filepath);
        return 1;
    }
    else
    {
        result = fputs(message, fout);
        if (result < 0)
        {
            perror(filepath);
            return 1;
        }
    }
    result = fclose(fout);
    if (result != 0)
    {
        perror(filepath);
        return 1;
    }
    printf("Done.\n"
           "Reading message from %s...\n", filepath);

    fin = fopen(filepath, "r");
    if (fout == NULL)
    {
        perror(filepath);
        return 1;
    }
    else
    {
        while (!feof(fin))
        {
            size_t nbytes;
            char *p;

            nbytes = fread(message, 1, sizeof(message), fin);
            if(ferror(fin))
            {
                perror(filepath);
                fclose(fin);
                return 1;
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
        }
    }
    printf("\nftell = %ld.\n", ftell(fin));
    result = fclose(fin);
    if (result != 0)
    {
        perror(filepath);
        return 1;
    }
    printf("Done.\n");

    return 0;
}

