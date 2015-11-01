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
#include <file.h>
#include <unistd.h>
#include <errno.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>

extern
void stdio_init(void);

extern
void stdio_delete(void);

static
int stdio_usart_write(int fd, char *ptr, int len)
{
    int i;
    struct fd *f;

    f = file_struct_get(fd);
    /* TODO: non-blocking */
    (void)f;
    for(i = 0; i < len; i++)
    {
        usart_send_blocking(USART2, ptr[i]);
    }
    return len;
}

static
int stdio_usart_read(int fd, char *ptr, int len)
{
    int nread = 0;
    struct fd *f;

    f = file_struct_get(fd);
    (void)f;
    if (len > 0)
    {
        usart_wait_recv_ready(USART2);
        do {
            ptr[nread] = usart_recv(USART2);
            nread++;
        } while ((nread < len) && (USART_SR(USART2) & USART_SR_RXNE));
        if (nread == 0)
        {
            errno = EAGAIN;
            nread = -1;
        }
    }
    return nread;
}

static
void stdio_usart_init(void)
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
void fileno_out_init(int fd)
{
    struct fd *f;

    f = file_struct_get(fd);

    f->fd = fd;
    f->stat.st_mode = S_IFCHR|S_IWUSR|S_IWGRP|S_IWOTH;
    f->write = stdio_usart_write;
    f->isatty = 1;
    f->isopen = 1;
}

static
void fileno_in_init(int fd)
{
    struct fd *f;
    f = file_struct_get(fd);

    f->fd = fd;
    f->stat.st_mode = S_IFCHR|S_IRUSR|S_IRGRP|S_IROTH;
    f->read = stdio_usart_read;
    f->isatty = 1;
    f->isopen = 1;
}

__attribute__((__constructor__))
void stdio_init(void)
{
    stdio_usart_init();
    fileno_in_init(STDIN_FILENO);
    fileno_out_init(STDOUT_FILENO);
    fileno_out_init(STDERR_FILENO);
}

static
void fileno_delete(int fd)
{
    struct fd *f;
    f = file_struct_get(fd);
    f->isopen = 0;
}

__attribute__((__destructor__))
void stdio_delete(void)
{
    fileno_delete(STDIN_FILENO);
    fileno_delete(STDOUT_FILENO);
    fileno_delete(STDERR_FILENO);
}

