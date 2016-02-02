
ROOT_DIR = ../../
OPENCM3_DIR = $(ROOT_DIR)/libopencm3

CPPFLAGS += -I$(ROOT_DIR)/include
include $(ROOT_DIR)/scripts/posix.mk

#include $(ROOT_DIR)/scripts/libopencm3.target.mk
include $(ROOT_DIR)/scripts/libopencm3.stm32f103rb.mk
#include $(ROOT_DIR)/scripts/libopencm3.stm32f411re.mk


