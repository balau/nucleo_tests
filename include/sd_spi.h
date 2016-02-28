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
#ifndef SD_SPI_H
#define SD_SPI_H

#include <stdint.h>
#include <stdlib.h>

extern
uint8_t sd_send_command_r1(uint8_t cmd, uint32_t arg);

extern
void sd_send_command(uint8_t cmd, uint32_t arg, void *resp, size_t len);

extern
uint8_t sd_read_single_block(uint32_t address, void *dst);

extern
uint16_t sd_write_single_block(uint32_t address, const void *src);

extern
void sd_full_speed(void);

extern
void sd_init(void);

#endif /* SD_SPI_H */

