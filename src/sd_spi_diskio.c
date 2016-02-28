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
#include <stdint.h>
#include "diskio.h"
#include "sd_spi.h"

#define N_PDRV 1

#define SD_STATE_IDLE 0x01
#define SD_SECTOR_SIZE 512

struct pdrv {
    int initialized:1;
    int present:1;
    int write_protected:1;
    int byte_addressable:1;
};

static struct pdrv pdrv_data[N_PDRV];

DSTATUS disk_initialize (BYTE pdrv)
{
    DSTATUS status;

    status = 0;

    if (pdrv < N_PDRV)
    {
        uint8_t r1;
        uint32_t arg_hcs;
        int tries;

        sd_init();
        tries = 4;
        do {
            r1 = sd_send_command_r1(0, 0);
            tries--;
        } while ((r1 != SD_STATE_IDLE) && (tries > 0));
        if (r1 != SD_STATE_IDLE)
        {
            status = STA_NODISK;
        }
        if (status == 0)
        {
            uint8_t r7[5];

            sd_send_command(8, 0x1AA, r7, sizeof(r7));
            if (
                    (r7[0] != SD_STATE_IDLE) /* state not idle */
                    ||
                    ((r7[3]&0x0F) != 0x01) /* not supported voltage range */
                    ||
                    (r7[4] != 0xAA) /* check pattern error */
               )
            {
                /* TODO: SD V1 */
                status = STA_NODISK;
            }
        }
        if (status == 0)
        {
            arg_hcs = 0x40000000;
            /* timeout should be around 1s.
             * SPI clk is 125kHz.
             * a command is around 8 bytes -> 64 SPI clk cycles.
             * a loop is around 128 SPI clk cycles -> ~1ms
             * tries = 1000 so it's ~1000ms.
             */
            tries = 1000;
            do
            {
                r1 = sd_send_command_r1(55, 0);
                r1 = sd_send_command_r1(41, arg_hcs);
                tries--;
            } while ((r1 & SD_STATE_IDLE) && (tries > 0));
            if (r1 & SD_STATE_IDLE)
            {
                status = STA_NODISK;
            }
        }
        if (status == 0)
        {
            uint8_t r3[5];
            
            sd_send_command(58, 0, r3, sizeof(r3));
            if (r3[1] & 0x40)
            {
                /* high capacity */
                pdrv_data[pdrv].byte_addressable = 0;
            }
            else
            {
                /* std capacity */
                pdrv_data[pdrv].byte_addressable = 1;
                r1 = sd_send_command_r1(16, SD_SECTOR_SIZE);
                if (r1 != 0)
                {
                    status = STA_NODISK;
                }
            }
        }
        if (status == 0)
        {
            sd_full_speed();
            pdrv_data[pdrv].initialized = 1;
            pdrv_data[pdrv].present = 1;
            /* temporarily protect from write */
            pdrv_data[pdrv].write_protected = 1;
            status = STA_PROTECT;
        }
    }
    else
    {
        status = STA_NODISK;
    }

    return status;
}

DSTATUS disk_status (BYTE pdrv)
{
    DSTATUS status;

    if (pdrv < N_PDRV)
    {
        if (!pdrv_data[pdrv].initialized)
        {
            status = STA_NOINIT;
        }
        else if (!pdrv_data[pdrv].present)
        {
            status = STA_NODISK;
        }
        else if (pdrv_data[pdrv].write_protected)
        {
            status = STA_PROTECT;
        }
        else
        {
            status = 0;
        }
    }
    else
    {
        status = STA_NODISK;
    }

    return status;
}

static
uint32_t get_addr(DWORD sector, int byte_addressable)
{
    uint32_t addr;

    if (byte_addressable)
    {
        addr = sector * SD_SECTOR_SIZE;
    }
    else
    {
        addr = sector;
    }

    return addr;
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    DRESULT result;

    if (pdrv >= N_PDRV)
    {
        result = RES_ERROR;
    }
    else if (!pdrv_data[pdrv].initialized)
    {
        result = RES_NOTRDY;
    }
    else if (!pdrv_data[pdrv].present)
    {
        result = RES_ERROR;
    }
    else
    {
        while (count > 0)
        {
            uint32_t addr;
            int read_res;

            addr = get_addr(sector, pdrv_data[pdrv].byte_addressable);
            read_res = sd_read_single_block(addr, buff);
            if (read_res != 0)
            {
                break;
            }
            buff += SD_SECTOR_SIZE;
            sector++;
            count--;
        }
        if (count > 0)
        {
            result = RES_ERROR;
        }
        else
        {
            result = RES_OK;
        }
    }

    return result;
}

DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    DRESULT result;

    result = RES_ERROR;

    return result;
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff)
{
    DRESULT result;

    if (pdrv >= N_PDRV)
    {
        result = RES_ERROR;
    }
    else if (!pdrv_data[pdrv].initialized)
    {
        result = RES_NOTRDY;
    }
    else if (!pdrv_data[pdrv].present)
    {
        result = RES_ERROR;
    }
    else
    {
        DWORD *buff_dword = (DWORD *)buff;

        switch(cmd)
        {
            case CTRL_SYNC:
                result = RES_OK;
                break;
            case GET_SECTOR_COUNT:
                *buff_dword = 2*1024*(1024/SD_SECTOR_SIZE); /* TODO, temporarily 2GiB */
                result = RES_OK;
                break;
            case GET_SECTOR_SIZE:
                *buff_dword = SD_SECTOR_SIZE;
                result = RES_OK;
                break;
            case GET_BLOCK_SIZE:
                *buff_dword = SD_SECTOR_SIZE;
                result = RES_OK;
                break;
            case CTRL_TRIM:
                result = RES_OK;
                break;
            default:
                result = RES_PARERR;
                break;
        }
    }

    return result;
}



