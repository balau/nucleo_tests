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
#include "ff.h"

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
    FRESULT result;
    FATFS fs;
    FIL fp;
    const char *filepath = "/rwtest.txt";

    printf(
            "fatfs_ro_test\n"
            "Press Enter to continue...\n");
    wait_enter();

    result = f_mount(&fs, "/", 1);
    printf("f_mount: %d\n", result);
    if (result == FR_OK)
    {
        result = f_open(&fp, filepath, FA_WRITE|FA_CREATE_ALWAYS);
        printf("f_open: %d\n", result);
    }
    if (result == FR_OK)
    {
        char message[80];
        UINT towrite;
        UINT bw;

        printf("Write a message:\n");
        scanf("%s", message);
        towrite = strlen(message) + 1;
        result = f_write(&fp, message, towrite, &bw);
        printf("f_write: %d\n", result);
        if (bw != towrite)
        {
            printf("f_write did write only %u/%u bytes.\n", bw, towrite);
        }
    }
    if (result == FR_OK)
    {
        result = f_close(&fp);
        printf("f_close: %d\n", result);
    }

    if (result == FR_OK)
    {
        result = f_open(&fp, filepath, FA_READ);
        printf("result: %d\n", result);
    }
    if (result == FR_OK)
    {
        uint8_t data[64];
        UINT nread;

        do {

            result = f_read(&fp, data, sizeof(data), &nread);
            fwrite(data, nread, 1, stdout);
        } while((result == FR_OK) && (nread == sizeof(data)));
        printf("\n");
        if (result != FR_OK)
        {
            printf("result: %d\n", result);
        }
    }
    if (result == FR_OK)
    {
        result = f_close(&fp);
        printf("f_close: %d\n", result);
    }

    return 0;
}

