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

#define DATA_CTRL_START 0xFE
#define DATA_RESP_MASK 0x1F
#define DATA_RESP_ACCEPTED 0x05
#define DATA_IDLE 0xFF
#define DATA_DUMMY 0xFF
#define BLOCK_SIZE 512

static
void sd_select(void)
{
    int tries;

    gpio_clear(GPIOB, GPIO5); /* lower chip select */

    tries = 125;
    do {
        if (spi_xfer(SPI1, DATA_DUMMY) == DATA_IDLE)
        {
            break;
        }
        tries--;
    } while(tries > 0);
}

static
void sd_deselect(void)
{
    gpio_set(GPIOB, GPIO5); /* raise chip select */
    (void)spi_xfer(SPI1, DATA_IDLE); /* SD card releases MISO */
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

static
int line_is_idle(uint8_t data)
{
    return ((data & 0x80) != 0);
}

static
uint8_t wait_resp(void)
{
    uint8_t r;

    do
    {
        r = spi_xfer(SPI1, DATA_DUMMY);
    } while (line_is_idle(r));

    return r;
}

static
void sd_send_command_inner(uint8_t cmd, uint32_t arg, void *resp, size_t len)
{
    uint8_t *resp_bytes;
    size_t i_byte;

    send_cmd(cmd, arg);
    resp_bytes = resp;

    resp_bytes[0] = wait_resp();
    for (i_byte = 1; i_byte < len; i_byte++)
    {
        resp_bytes[i_byte] = spi_xfer(SPI1, DATA_DUMMY);
    }
}

void sd_send_command(uint8_t cmd, uint32_t arg, void *resp, size_t len)
{
    sd_select();
    sd_send_command_inner(cmd, arg, resp, len);
    sd_deselect();
}

uint8_t sd_send_command_r1(uint8_t cmd, uint32_t arg)
{
    uint8_t r1;

    sd_send_command(cmd, arg, &r1, 1);

    return r1;
}

static
int send_rw_cmd(uint8_t cmd, uint32_t address)
{
    uint8_t r1;

    send_cmd(cmd, address);

    r1 = wait_resp();

    return (r1 == 0x00)?0:-1;
}

static
int read_block(void *dst)
{
    int res;
    uint8_t *dst_bytes;
    uint8_t data_ctrl;

    dst_bytes = dst;

    do
    {
        data_ctrl = spi_xfer(SPI1, DATA_DUMMY);
    } while (data_ctrl == DATA_IDLE); /* TODO: timeout */

    if (data_ctrl == DATA_CTRL_START)
    {
        int i_byte;
        uint8_t crc16_hi;
        uint8_t crc16_lo;

        data_ctrl = 0;

        for (i_byte = 0; i_byte < BLOCK_SIZE; i_byte++)
        {
            dst_bytes[i_byte] = spi_xfer(SPI1, DATA_DUMMY);
        }
        crc16_hi = spi_xfer(SPI1, DATA_DUMMY);
        crc16_lo = spi_xfer(SPI1, DATA_DUMMY);
        /* crc16: don't care. TODO: care. */
        (void)crc16_hi;
        (void)crc16_lo;

        res = 0;
    }
    else
    {
        res = -1;
    }

    return res;
}

int sd_read_single_block(uint32_t address, void *dst)
{
    int res;

    sd_select();
    res = send_rw_cmd(17, address);
    if (res == 0)
    {
        res = read_block(dst);
    }
    sd_deselect();

    return res;
}

static
int send_block(const void *src)
{
    const uint8_t *src_bytes;
    int i_byte;
    uint8_t data_resp;

    src_bytes = src;

    (void)spi_xfer(SPI1, DATA_CTRL_START);
    for (i_byte = 0; i_byte < BLOCK_SIZE; i_byte++)
    {
        (void)spi_xfer(SPI1, src_bytes[i_byte]);
    }
    /* crc16: don't care. TODO: care. */
    (void)spi_xfer(SPI1, DATA_DUMMY); 
    (void)spi_xfer(SPI1, DATA_DUMMY);

    data_resp = spi_xfer(SPI1, DATA_DUMMY);

    return ((data_resp & DATA_RESP_MASK) == DATA_RESP_ACCEPTED)?0:-1;
}

static
int wait_end_write(void)
{
    uint8_t busy;
    uint16_t r2;

    do
    {
        busy = spi_xfer(SPI1, DATA_DUMMY);
    } while (busy != DATA_IDLE); /* TODO: timeout */

    sd_send_command_inner(13, 0, &r2, 2);

    return (r2 == 0x0000)?0:-1;
}

int sd_write_single_block(uint32_t address, const void *src)
{
    int res;

    sd_select();
    res = send_rw_cmd(24, address);
    if (res == 0)
    {
        res = send_block(src);
    }
    if (res == 0)
    {
        res = wait_end_write();
    }
    sd_deselect();

    return res;
}

static
void sd_spi_init(uint32_t br)
{
    spi_init_master(
            SPI1,
            br,
            SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
            SPI_CR1_CPHA_CLK_TRANSITION_1,
            SPI_CR1_DFF_8BIT,
            SPI_CR1_MSBFIRST);
}

void sd_full_speed(void)
{
    sd_spi_init(SPI_CR1_BAUDRATE_FPCLK_DIV_2); /* 4MHz */
}

void sd_init(void)
{
    int i_dummy_clk;
    int spi_clk_khz;

    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);

    /* CN9_5 D4 PB5 SD_CS */
    /* CN5_3 D10 PB6 SPI1_CS */
    gpio_set(GPIOB, GPIO5);
    gpio_set(GPIOB, GPIO6);
#ifdef STM32F1
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO6);
    /* CN5_6 D13 PA5 SPI1_SCK */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_SCK);
    /* CN5_4 D11 PA7 SPI1_MOSI */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_MOSI);
    /* CN5_5 D12 PA6 SPI1_MISO */
#  if 0
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_SPI1_MISO);
    gpio_set(GPIOA, GPIO_SPI1_MISO);
#  else
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_SPI1_MISO);
#  endif
#elif defined(STM32F4)
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5|GPIO6);
    /* CN5_6 D13 PA5 SPI1_SCK */
    /* CN5_4 D11 PA7 SPI1_MOSI */
    /* CN5_5 D12 PA6 SPI1_MISO */
    gpio_set_af(GPIOA, GPIO_AF5, GPIO5|GPIO6|GPIO7);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5|GPIO6|GPIO7);
#endif
    spi_clk_khz = 250;
    /* Clock:
     * HSI 8MHz is the default
     * RCC_CFGR_SW = 0b00 -> HSI chosen as SYSCLK
     * RCC_CFGR_HPRE = 0b0000 -> no AHB prescaler
     * SPI1 is on APB2
     * RCC_CFGR_PRE2 = 0b0000 -> no APB2 prescaler
     * -> FPCLK = 8MHz
     * -> BR FPCLK/32 -> SCLK @ 250kHz < 400kHz
     */
    sd_spi_init(SPI_CR1_BAUDRATE_FPCLK_DIV_32);

    spi_enable_software_slave_management(SPI1);
    spi_set_nss_high(SPI1); /* Avoid Master mode fault MODF */
    spi_enable(SPI1);

    /* >74 clk cycles, >1ms -> >250clk */
    i_dummy_clk = 0;
    do
    {
        (void)spi_xfer(SPI1, 0xFF);
        i_dummy_clk += 8;
    } while (i_dummy_clk < 74 || i_dummy_clk < spi_clk_khz);
}

