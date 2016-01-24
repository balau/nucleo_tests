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
#include "diskio.h"

void wait_enter(void)
{
    int c;

    do {
        c = getchar();
    } while ((c != '\n') && (c != '\r'));
}

int main(void)
{
    DSTATUS status;
    DRESULT result;
    BYTE pdrv = 0;

    printf(
            "diskio_test\n"
            "Press Enter to continue...\n");
    wait_enter();

    status = disk_status(pdrv);
    printf("status: 0x%02X\n", status);
    status = disk_initialize(pdrv);
    printf("status: 0x%02X\n", status);
    status = disk_status(pdrv);
    printf("status: 0x%02X\n", status);

    return 0;
}

