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
#include "sd_spi.h"
#include <stdint.h>
#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>

static
void sd_select(void)
{
    gpio_clear(GPIOB, GPIO5); /* lower chip select */
}

static
void sd_deselect(void)
{
    gpio_set(GPIOB, GPIO5); /* raise chip select */
}

static
uint8_t crc7_get(uint8_t cmd, uint32_t arg)
{
    uint8_t crc7;

    (void)arg;
    cmd &= 0x3F;
    if (cmd == 0)
    {
        /* arg should be 0 */
        crc7 = 0x94;
    }
    else if (cmd == 8)
    {
        /* arg should be 0x1AA */
        crc7 = 0x86;
    }
    else
    {
        /* don't care */
        crc7 = 0xFF;
    }
    crc7 |= 0x01; /* end bit */
    return crc7;
}

static
void send_cmd(uint8_t cmd, uint32_t arg)
{
    uint8_t crc7;

    crc7 = crc7_get(cmd, arg);

    cmd |= 0x40;
    (void)spi_xfer(SPI1, cmd);
    (void)spi_xfer(SPI1, (arg>>24) & 0xFF);
    (void)spi_xfer(SPI1, (arg>>16) & 0xFF);
    (void)spi_xfer(SPI1, (arg>> 8) & 0xFF);
    (void)spi_xfer(SPI1, (arg>> 0) & 0xFF);
    (void)spi_xfer(SPI1, crc7);
}

void sd_send_command(uint8_t cmd, uint32_t arg, void *resp, size_t len)
{
    uint8_t *resp_bytes;
    size_t i_byte;

    sd_select();
    send_cmd(cmd, arg);
    resp_bytes = resp;

    for (i_byte = 0; i_byte < len; i_byte++)
    {
        uint8_t r;
        do
        {
            r = spi_xfer(SPI1, 0xFF);
        } while (((r & 0x80) == 0x80) && (i_byte == 0));

        resp_bytes[i_byte] = r;
    }

    sd_deselect();
    (void)spi_xfer(SPI1, 0xFF);
}

uint8_t sd_send_command_r1(uint8_t cmd, uint32_t arg)
{
    uint8_t r1;

    sd_send_command(cmd, arg, &r1, 1);

    return r1;
}

extern
uint8_t sd_read_single_block(uint32_t address, void *dst)
{
    uint8_t *dst_bytes;
    uint8_t r1;
    uint8_t data_ctrl;

    sd_select();
    send_cmd(17, address);
    dst_bytes = dst;

    do
    {
        r1 = spi_xfer(SPI1, 0xFF);
    } while (r1 & 0x80);
    
    do
    {
        data_ctrl = spi_xfer(SPI1, 0xFF);
    } while (data_ctrl == 0xFF);
    if (data_ctrl == 0xFE)
    {
        int i_byte;
        uint8_t crc16_hi;
        uint8_t crc16_lo;

        data_ctrl = 0;

        for (i_byte = 0; i_byte < 512; i_byte++)
        {
            dst_bytes[i_byte] = spi_xfer(SPI1, 0xFF);
        }
        crc16_hi = spi_xfer(SPI1, 0xFF);
        crc16_lo = spi_xfer(SPI1, 0xFF);
        (void)crc16_hi;
        (void)crc16_lo;
    }

    sd_deselect();
    (void)spi_xfer(SPI1, 0xFF);

    return data_ctrl;
}


void sd_init(void)
{
    int i_dummy;

    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);

    /* CN9_5 D4 PB5 SD_CS */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
    gpio_set(GPIOB, GPIO5);
    /* CN5_3 D10 PB6 SPI1_CS */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO6);
    gpio_set(GPIOB, GPIO6);
    /* CN5_6 D13 PA5 SPI1_SCK */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_SCK);
    /* CN5_4 D11 PA7 SPI1_MOSI */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_MOSI);
    /* CN5_5 D12 PA6 SPI1_MISO */
#if 0
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_SPI1_MISO);
    gpio_set(GPIOA, GPIO_SPI1_MISO);
#else
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_SPI1_MISO);
#endif
    
    /* Clock:
     * HSI 8MHz is the default
     * RCC_CFGR_SW = 0b00 -> HSI chosen as SYSCLK
     * RCC_CFGR_HPRE = 0b0000 -> no AHB prescaler
     * SPI1 is on APB2
     * RCC_CFGR_PRE2 = 0b0000 -> no APB2 prescaler
     * -> FPCLK = 8MHz
     * -> BR FPCLK/2 -> SCLK @ 4MHz
     */
    spi_init_master(
            SPI1,
            SPI_CR1_BAUDRATE_FPCLK_DIV_64,
            SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
            SPI_CR1_CPHA_CLK_TRANSITION_1,
            SPI_CR1_DFF_8BIT,
            SPI_CR1_MSBFIRST);

    spi_enable_software_slave_management(SPI1);
    spi_set_nss_high(SPI1); /* Avoid Master mode fault MODF */
    spi_enable(SPI1);

    /* >74 clk cycles */
    for (i_dummy = 0; i_dummy < 10; i_dummy++)
    {
        (void)spi_xfer(SPI1, 0xFF);
    }
}

