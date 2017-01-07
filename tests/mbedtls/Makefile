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

BINARY = mbedtls_test
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
OBJS += $(ROOT_DIR)/src/select.o
OBJS += $(ROOT_DIR)/src/poll.o
OBJS += digicert_der.o
LDLIBS_SYS =

MBEDTLS_DIR = $(ROOT_DIR)/mbedtls-2.2.0
CPPFLAGS += -I$(MBEDTLS_DIR)/include
LDFLAGS += -L$(MBEDTLS_DIR)/library
LDLIBS += -lmbedtls -lmbedx509 -lmbedcrypto
CPPFLAGSG += -I$(PWD) -DMBEDTLS_CONFIG_FILE="<mbedtls_nucleo_config.h>"

include ../test.mk

%_der.c: %.der
	xxd -i $< |sed 's/\<unsigned\>/const unsigned/g' >$@

clean: libmbedtls_clean

libmbedtls_clean:
	cd $(MBEDTLS_DIR); \
		make -s clean;

mbedtls_test.o: $(MBEDTLS_DIR)/library/libmbedtls.a

$(MBEDTLS_DIR)/library/libmbedtls.a:
	root_dir=`readlink -f $(ROOT_DIR)`; \
		cd $(MBEDTLS_DIR); \
		export CPPFLAGS="-I$$root_dir/include -D_POSIX_SOURCE=200809L -D_POSIX_C_SOURCE=200809L -D_POSIX_TIMERS=200809L -D_POSIX_MONOTONIC_CLOCK=200809L -D_POSIX_CLOCK_SELECTION=200809L -D_XOPEN_SOURCE=700 -I$(PWD) -DMBEDTLS_CONFIG_FILE=\"<mbedtls_nucleo_config.h>\""; \
		export CFLAGS="$(ARCH_FLAGS) -g -Os -ffunction-sections $$CPPFLAGS"; \
		export LDFLAGS="$(ARCH_FLAGS) -g --specs=nosys.specs -L$$root_dir/src/ -L$$root_dir/libopencm3/lib"; \
		export CC=arm-none-eabi-gcc; \
		export AR=arm-none-eabi-ar; \
		export LIBS="-lposix -lopencm3_stm32f1"; \
		make lib

