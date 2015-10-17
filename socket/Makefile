#
# Copyright (c) 2015 Francesco Balducci
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

BINARY = socket_client_test
OBJS += w5100_socket.o
OBJS += w5100_spi.o
OBJS += inet.o
OBJS += ../stdio/stdio_usart.o
OBJS += ../syscalls/syscalls.o
CPPFLAGS += -I../syscalls
OPENCM3_DIR = ../libopencm3
LDSCRIPT = ../stm32f103rb.ld
OOCD_INTERFACE = stlink-v2-1
OOCD_BOARD = st_nucleo_f103rb
LDLIBS_SYS =

include ../libopencm3.target.mk

