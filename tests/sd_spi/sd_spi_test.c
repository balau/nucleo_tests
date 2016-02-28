/*
 * Copyright (c) 2015 Francesco Balducci
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
#include <stdint.h>
#include <stdio.h>
#include "sd_spi.h"

static
void print_resp(uint8_t cmd, void *resp, size_t len)
{
    size_t i;
    uint8_t *resp_bytes;

    printf("CMD%d: ", cmd);
    resp_bytes = resp;
    for (i = 0; i < len; i++)
    {
        printf("%02X", (unsigned)resp_bytes[i]);
    }
    printf("\n");
}

int main(void)
{
    uint8_t r1;
    uint8_t r7[5];
    uint8_t r3[5];
    uint8_t r2[2];
    uint32_t arg_hcs;
    uint8_t block[512];
    int tries;
    int read_res;

    
    printf("SD card SPI initialization...");
    getchar();
    printf("\n");

    sd_init();

    tries = 4;
    do {
        r1 = sd_send_command_r1(0, 0);
        print_resp(0, &r1, 1);
        if (r1 != 0x01)
        {
            fprintf(stderr, "state not idle\n");
        }
        tries--;
        if (tries <= 0)
        {
            fprintf(stderr, "giving up\n");
            return 1;
        }
    } while (r1 != 0x01);

    sd_send_command(8, 0x1AA, r7, sizeof(r7));
    print_resp(8, r7, sizeof(r7));

    if (r7[0] != 0x01)
    {
        fprintf(stderr, "state not idle\n");
        return 1;
    }
    else if ((r7[3]&0x0F) != 0x01)
    {
        fprintf(stderr, "non supported voltage range\n");
        return 1;
    }
    else if (r7[4] != 0xAA)
    {
        fprintf(stderr, "check pattern error\n");
        return 1;
    }

    sd_send_command(58, 0, r3, sizeof(r3));
    print_resp(58, r3, sizeof(r3));

    arg_hcs = 0x40000000;
    do 
    {
        r1 = sd_send_command_r1(55, 0);
        r1 = sd_send_command_r1(41, arg_hcs);
    } while(r1 & 0x01);
    print_resp(41, &r1, 1);

    sd_send_command(58, 0, r3, sizeof(r3));
    print_resp(58, r3, sizeof(r3));
    if (r3[1] & 0x40)
    {
        printf("High capacity\n");
    }
    else
    {
        printf("Standard capacity\n");
    }

    //sd_send_command(13, 0, r2, sizeof(r2));
    //print_resp(13, r2, sizeof(r2));

    read_res = sd_read_single_block(0, block);
    if (read_res == 0)
    {
        print_resp(17, block, sizeof(block));
    }
    else
    {
        print_resp(17, &read_res, sizeof(read_res));
    }

    sd_send_command(13, 0, r2, sizeof(r2));
    print_resp(13, r2, sizeof(r2));

    sd_send_command(13, 0, r2, sizeof(r2));
    print_resp(13, r2, sizeof(r2));

    return 0; 
}

