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
#include "w5100.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>

#define OP_WRITE 0xF0
#define OP_READ 0x0F

static
void w5100_select(void)
{
    gpio_clear(GPIOB, GPIO6); /* lower chip select */
}

static
void w5100_deselect(void)
{
    gpio_set(GPIOB, GPIO6); /* raise chip select */
}

static
void w5100_xfer_addr(uint16_t reg)
{
    (void)spi_xfer(SPI1, reg >> 8);
    (void)spi_xfer(SPI1, reg & 0xFF);
}

static
void w5100_write_byte(uint16_t reg, uint8_t val)
{
    w5100_select();
    (void)spi_xfer(SPI1, OP_WRITE);
    w5100_xfer_addr(reg);
    (void)spi_xfer(SPI1, val);
    w5100_deselect();
}

static
uint8_t w5100_read_byte(uint16_t reg)
{
    uint8_t rx;

    w5100_select();
    (void)spi_xfer(SPI1, OP_READ);
    w5100_xfer_addr(reg);
    rx = spi_xfer(SPI1, 0x00);
    w5100_deselect();

    return rx;
}

uint8_t w5100_read_reg(uint16_t reg)
{
    uint8_t rx;

    rx = w5100_read_byte(reg);
    
    return rx;
}

uint16_t w5100_read_reg2(uint16_t reg)
{
    uint16_t rx;
    int16_t i_byte;

    rx = 0;
    /* MSB first. */
    for (i_byte = 0; i_byte < 2; i_byte++)
    {
        rx |= w5100_read_byte(reg + i_byte);
        rx <<= 8;
    }
    
    return rx;
}

void w5100_write_reg(uint16_t reg, uint8_t val)
{
    w5100_write_byte(reg, val);
}

void w5100_write_reg2(uint16_t reg, uint16_t val)
{
    int16_t i_byte;

    /* MSB first. */
    for (i_byte = 0; i_byte < 2; i_byte++)
    {
        uint8_t b;
        int32_t shiftr;

        shiftr = (3 - i_byte) * 8;
        b = (val >> shiftr) & 0xFF;
        w5100_write_byte(reg + i_byte, b);
    }
}

void w5100_read_mem(uint16_t addr, void *buf, size_t n)
{
    uint8_t *pbytes = buf;
    size_t i_byte;

    for (i_byte = 0; i_byte < n; i_byte++)
    {
        pbytes[i_byte] = w5100_read_byte(addr + i_byte);
    }
}

void w5100_write_mem(uint16_t addr, const void *buf, size_t n)
{
    const uint8_t *pbytes = buf;
    size_t i_byte;

    for (i_byte = 0; i_byte < n; i_byte++)
    {
        w5100_write_byte(addr + i_byte, pbytes[i_byte]);
    }
}

static
void w5100_spi_init(void)
{
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);

#ifdef STM32F1
    /* CN5_6 D13 PA5 SPI1_SCK */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_SCK);
    /* CN5_4 D11 PA7 SPI1_MOSI */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_MOSI);
    /* CN5_5 D12 PA6 SPI1_MISO */
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_SPI1_MISO);
    /* CN5_3 D10 PB6 SPI1_CS */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO6);
#elif defined(STM32F4)
    /* CN5_6 D13 PA5 SPI1_SCK */
    /* CN5_4 D11 PA7 SPI1_MOSI */
    /* CN5_5 D12 PA6 SPI1_MISO */
    gpio_set_af(GPIOA, GPIO_AF5, GPIO5|GPIO6|GPIO7);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5|GPIO6|GPIO7);
    /* CN5_3 D10 PB6 SPI1_CS */
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6);
#endif
    w5100_deselect();
    
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
            SPI_CR1_BAUDRATE_FPCLK_DIV_2,
            SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
            SPI_CR1_CPHA_CLK_TRANSITION_1,
            SPI_CR1_DFF_8BIT,
            SPI_CR1_MSBFIRST);

    spi_enable_software_slave_management(SPI1);
    spi_set_nss_high(SPI1); /* Avoid Master mode fault MODF */
    spi_enable(SPI1);
}

void w5100_init(void)
{
    w5100_spi_init();

}

