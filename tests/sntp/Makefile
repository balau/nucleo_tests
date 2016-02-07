#
# Copyright (c) 2016 Francesco Balducci
#
# This file is part of nucleo_tests.
#
#    nucleo_tests is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    nucleo_tests is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with nucleo_tests.  If not, see <http://www.gnu.org/licenses/>.
#

BINARY = sntp_test
OBJS += $(ROOT_DIR)/src/stdio_usart.o
OBJS += $(ROOT_DIR)/src/syscalls.o
OBJS += $(ROOT_DIR)/src/file.o
OBJS += $(ROOT_DIR)/src/clock_gettime_systick.o
OBJS += $(ROOT_DIR)/src/timespec.o
OBJS += $(ROOT_DIR)/src/sntp.o
OBJS += $(ROOT_DIR)/src/w5100_socket.o
OBJS += $(ROOT_DIR)/src/w5100_spi.o
OBJS += $(ROOT_DIR)/src/inet.o

include ../test.mk

