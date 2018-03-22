TARGET = firmware

ARCH = stm32
ARCH_FLAGS = -mthumb -mcpu=cortex-m0plus -msoft-float
DEFS += -DSTM32L0
LDSCRIPT = ./stm32l0xx4.ld
LIBNAME = opencm3_stm32l0

# Executables

PREFIX ?= arm-none-eabi

CC := $(PREFIX)-gcc
CXX := $(PREFIX)-g++
LD := $(PREFIX)-gcc
AR := $(PREFIX)-ar
AS := $(PREFIX)-as
OBJCOPY := $(PREFIX)-objcopy
OBJDUMP := $(PREFIX)-objdump
OPT := -Os
CSTD ?= -std=c99

# Sources

SRC = $(wildcard src/*.c)
OBJS = $(SRC:.c=.o)
DEFS += -I./src

OPENCM3_DIR = ./libopencm3
DEFS += -I./libopencm3/include
DEFS += -I./libopencm3/include/libopencm3/$(ARCH)
DEFS += -I./libopencm3/include/libopencm3/cm3

LDFLAGS += -L./libopencm3/lib
LDLIBS += -l$(LIBNAME)

# C flags

TGT_CFLAGS += $(OPT) $(CSTD) -g
TGT_CFLAGS += $(ARCH_FLAGS)
TGT_CFLAGS += -Wextra -Wshadow -Wimplicit-function-declaration
TGT_CFLAGS += -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
TGT_CFLAGS += -fno-common -ffunction-sections -fdata-sections

# C++ flags

TGT_CXXFLAGS += $(OPT) $(CXXSTD) -g
TGT_CXXFLAGS += $(ARCH_FLAGS)
TGT_CXXFLAGS += -Wextra -Wshadow -Wredundant-decls
#TGT_CXXFLAGS += -Wextra -Wshadow -Wredundant-decls  -Weffc++
TGT_CXXFLAGS += -fno-common -ffunction-sections -fdata-sections

# C & C++ preprocessor common flags

TGT_CPPFLAGS += -MD
TGT_CPPFLAGS += -Wall -Wundef
TGT_CPPFLAGS += $(DEFS)

# Linker flags

TGT_LDFLAGS += --static -nostartfiles
TGT_LDFLAGS += -T$(LDSCRIPT)
TGT_LDFLAGS += $(ARCH_FLAGS)
TGT_LDFLAGS += -Wl,-Map=$(*).map
TGT_LDFLAGS += -Wl,--gc-sections
TGT_LDFLAGS += -specs=nosys.specs -specs=nano.specs # printf

ifeq ($(V),99)
TGT_LDFLAGS += -Wl,--print-gc-sections
endif

# Libraries

LDLIBS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

##################################################

.SUFFIXES: .elf .bin .hex .srec .list .map
.SECONDEXPANSION:
.SECONDARY:

all: bin

elf: $(TARGET).elf
bin: $(TARGET).bin
hex: $(TARGET).hex
srec: $(TARGET).srec
list: $(TARGET).list

%.bin: %.elf
	$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.hex: %.elf
	$(OBJCOPY) -Oihex $(*).elf $(*).hex

%.srec: %.elf
	$(OBJCOPY) -Osrec $(*).elf $(*).srec

%.list: %.elf
	$(OBJDUMP) -S $(*).elf > $(*).list

%.elf %.map: $(OBJS) $(LDSCRIPT)
	$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(*).elf

%.o: %.c
	$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).c

%.o: %.cxx
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cxx

%.o: %.cpp
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cpp

clean:
	$(RM) *.o *.d *.elf *.bin *.hex *.srec *.list *.map generated.* ${OBJS} ${SRC:.c=.d}

flash:
	st-flash --reset write firmware.bin 0x08000000

reset:
	st-flash reset

console:
	screen /dev/ttyUSB0 115200

-include $(OBJS:.o=.d)

.PHONY: clean elf bin hex srec list flash reset console
