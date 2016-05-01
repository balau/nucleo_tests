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

    fout = fopen(filepath, "w");
    if (fout == NULL)
    {
        perror(filepath);
        return 1;
    }
    else
    {
        printf("Write a message:\n");
        scanf("%s", message);
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

    fin = fopen(filepath, "r");
    if (fout == NULL)
    {
        perror(filepath);
        return 1;
    }
    else
    {
        char *presult;

        presult = fgets(message, sizeof(message) - 1, fin);
        if (presult == NULL)
        {
            perror(filepath);
            return 1;
        }
        puts(message);
    }
    result = fclose(fin);
    if (result != 0)
    {
        perror(filepath);
        return 1;
    }

    return 0;
}

