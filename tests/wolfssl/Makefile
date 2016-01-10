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

BINARY = wolfssl
OBJS += $(ROOT_DIR)/src/w5100_socket.o
OBJS += $(ROOT_DIR)/src/w5100_spi.o
OBJS += $(ROOT_DIR)/src/inet.o
OBJS += $(ROOT_DIR)/src/stdio_usart.o
OBJS += $(ROOT_DIR)/src/syscalls.o
OBJS += $(ROOT_DIR)/src/file.o
OBJS += $(ROOT_DIR)/src/timespec.o
OBJS += $(ROOT_DIR)/src/clock_gettime_systick.o
OBJS += $(ROOT_DIR)/src/gettimeofday.o
OBJS += $(ROOT_DIR)/src/getaddrinfo.o
LDLIBS_SYS =

WOLFSSL_DIR = $(ROOT_DIR)/wolfssl-3.7.0
CPPFLAGS += -I$(WOLFSSL_DIR)
LDFLAGS += -L$(WOLFSSL_DIR)/src/.libs
LDLIBS += -lwolfssl -lm

include ../test.mk

clean: libwolfssl_clean

libwolfssl_clean:
	cd $(WOLFSSL_DIR); \
		make -s distclean || true;

wolfssl.o: $(WOLFSSL_DIR)/src/.libs/libwolfssl.a

$(WOLFSSL_DIR)/src/.libs/libwolfssl.a:
	cd $(WOLFSSL_DIR); \
		export CPPFLAGS="-I/home/francesco/Projects/Nucleo/nucleo_tests/include -D_POSIX_SOURCE=200809L -D_POSIX_C_SOURCE=200809L -D_POSIX_TIMERS=200809L -D_POSIX_MONOTONIC_CLOCK=200809L -D_POSIX_CLOCK_SELECTION=200809L -D_XOPEN_SOURCE=700 -DCUSTOM_RAND_GENERATE=rand -DWOLFSSL_STATIC_RSA -DNO_WOLFSSL_SERVER -DNO_SESSION_CACHE"; \
		export CFLAGS="-mthumb -march=armv7-m -g"; \
		export LDFLAGS="-mthumb -march=armv7-m -g --specs=nosys.specs -L/home/francesco/Projects/Nucleo/nucleo_tests/src/ -L/home/francesco/Projects/Nucleo/nucleo_tests/libopencm3/lib"; \
		export LIBS="-lposix -lopencm3_stm32f1"; \
		./configure -q --host=arm-none-eabi --disable-filesystem --disable-des3 --disable-chacha --disable-poly1305 --disable-memory --disable-oldtls; \
		make -s

