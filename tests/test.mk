
ROOT_DIR = ../../
OPENCM3_DIR = $(ROOT_DIR)/libopencm3
LDSCRIPT = $(ROOT_DIR)/scripts/stm32f103rb.ld
OOCD_INTERFACE = stlink-v2-1
OOCD_BOARD = st_nucleo_f103rb

include $(ROOT_DIR)/scripts/libopencm3.target.mk

