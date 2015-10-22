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
#include <ctype.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>

static void delay_loop(int32_t loops)
{
    while(loops > 0)
    {
        asm("nop");
        loops--;
    }
}

static void usart_puts(uint32_t usart, const char *s)
{
    while(*s != '\0')
    {
        usart_send_blocking(usart, *s);
        s++;
    }
}

int main(void)
{
    int32_t i;

    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);

    flash_set_ws(FLASH_ACR_LATENCY_2WS);
	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV4); /* 14MHz */
	rcc_set_hpre(RCC_CFGR_HPRE_SYSCLK_NODIV); /* 56MHz */
	rcc_set_ppre1(RCC_CFGR_PPRE1_HCLK_DIV2); /* 28MHz */
	rcc_set_ppre2(RCC_CFGR_PPRE2_HCLK_NODIV); /* 56MHz */
    rcc_set_pll_multiplication_factor(RCC_CFGR_PLLMUL_PLL_CLK_MUL14); /* 8MHz/2 x14 = 56MHz */
	rcc_set_pll_source(RCC_CFGR_PLLSRC_HSI_CLK_DIV2);
	rcc_osc_on(PLL);
	rcc_wait_for_osc_ready(PLL);
	rcc_set_sysclk_source(RCC_CFGR_SW_SYSCLKSEL_PLLCLK);

    rcc_periph_clock_enable(RCC_USART2);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_USART2_RX);

    uint32_t baud = 57600;
    uint32_t clock = 28000000;
	USART2_BRR = ((2 * clock) + baud) / (2 * baud);
    
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX_RX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
	usart_enable(USART2);

    usart_puts(USART2, "ready to receive.\r\n");
    gpio_set(GPIOA, GPIO5);
    while(1)
    {
        uint16_t c;
        c = usart_recv_blocking(USART2);
        usart_send_blocking(USART2, toupper(c));
        gpio_toggle(GPIOA, GPIO5);
    }
}

