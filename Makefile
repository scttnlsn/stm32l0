PROJECT = firmware
BUILD_DIR = bin

SRC_DIR = ./src
CFILES = $(wildcard src/*.c)

DEVICE=stm32l031k6

# You shouldn't have to edit anything below here.
VPATH += $(SRC_DIR)
INCLUDES += $(patsubst %,-I%, . $(SRC_DIR))
OPENCM3_DIR=./libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ./rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

flash:
	st-flash --reset write firmware.bin 0x08000000
