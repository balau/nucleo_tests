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
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>

static
void usart_init(void)
{
    uint32_t baud = 57600;
    uint32_t clock = 8000000;

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);
    
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_USART2_RX);
	USART2_BRR = ((2 * clock) + baud) / (2 * baud);
    
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX_RX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
	usart_enable(USART2);
}

static
void usart_puts(const char *s)
{
    while(*s != '\0')
    {
        usart_send_blocking(USART2, *s);
        s++;
    }
}

static
void usart_printhex(uint8_t v)
{
    char hex[3];
    
    snprintf(hex, 3, "%02x", v);
    usart_puts(hex);
}

static
void spi1_init(void)
{
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);

    /* CN5_6 D13 PA5 SPI1_SCK */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_SCK);
    /* CN5_4 D11 PA7 SPI1_MOSI */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI1_MOSI);
    /* CN5_5 D12 PA6 SPI1_MISO */
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_SPI1_MISO);
    /* CN5_3 D10 PB6 SPI1_CS */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO6);
    gpio_set(GPIOB, GPIO6);
    
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

static
uint8_t w5100_read_reg1(uint16_t reg)
{
    uint8_t rx;

    gpio_clear(GPIOB, GPIO6); /* lower chip select */
    (void)spi_xfer(SPI1, 0x0F);
    (void)spi_xfer(SPI1, reg >> 8);
    (void)spi_xfer(SPI1, reg & 0xFF);
    rx = spi_xfer(SPI1, 0x00);
    gpio_set(GPIOB, GPIO6); /* raise chip select */
    return rx;
}

int main(void)
{
    uint8_t rmsr;

    usart_init();
    spi1_init();
 
    usart_puts("Running.\r\n");
    rmsr = w5100_read_reg1(0x001a);
    usart_puts("RMSR = 0x");
    usart_printhex(rmsr);
    usart_puts("\r\n");
    while(1);
}

