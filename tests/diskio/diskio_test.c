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
#include "diskio.h"

static
void wait_enter(void)
{
    int c;

    do {
        c = getchar();
    } while ((c != '\n') && (c != '\r'));
}

static
void print_data(const void *data, size_t n)
{
    const uint8_t *bytes;
    size_t i;

    bytes = data;
    for (i = 0; i < n; i++)
    {
        printf("%02X", bytes[i]);
        if (((i + 1) % 16) == 0)
        {
            printf("\n");
        }
    }
}

int main(void)
{
    DSTATUS status;
    DRESULT result;
    BYTE pdrv;
    uint8_t data[512*2];

    printf(
            "diskio_test\n"
            "Press Enter to continue...\n");
    wait_enter();

    pdrv = 0;

    status = disk_status(pdrv);
    printf("status: 0x%02X\n", status);
    status = disk_initialize(pdrv);
    printf("status: 0x%02X\n", status);
    status = disk_status(pdrv);
    printf("status: 0x%02X\n", status);

    result = disk_read (pdrv, data, 0, 2);
    printf("result: 0x%02X\n", result);
    if (result == RES_OK)
    {
        print_data(data, sizeof(data));
    }
    return 0;
}

