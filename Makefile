# Heavily influenced by the makefile used in the interrupt blog's zero to main series
# see: https://github.com/memfault/zero-to-main

ifdef DEBUG
	NO_ECHO :=
else
	NO_ECHO := @
endif

PROJECT := unmanaged_flash
BUILD_DIR ?= build

SRCS += \
	src/st/ll/stm32l4xx_ll_gpio.c \
	src/st/ll/stm32l4xx_ll_pwr.c \
	src/st/ll/stm32l4xx_ll_rcc.c \
	src/st/ll/stm32l4xx_ll_spi.c \
	src/st/ll/stm32l4xx_ll_utils.c \
	src/st/system_stm32l4xx.c \
	src/modules/led.c \
	src/modules/shell.c \
	src/modules/shell_command.c \
	src/modules/spi.c \
	src/modules/sys_time.c \
	src/modules/uart.c \
	src/syscalls.c \
	src/startup_stm32l432kc.c \
	src/stm32l432kc_it.c \
	src/main.c

INCLUDES += \
	src/st \
	src/st/ll \
	src/cmsis \


CC=arm-none-eabi-gcc
LD=arm-none-eabi-ld
OCPY=arm-none-eabi-objcopy
ODUMP=arm-none-eabi-objdump
SZ=arm-none-eabi-size
MKDIR=mkdir
STFLASH=st-flash

ASMFLAGS += \
	-mcpu=cortex-m4 \
	-g3 \
	-x assembler \
	--specs=nano.specs \
	-mfpu=fpv4-sp-d16 \
	-mfloat-abi=hard \
	-mthumb

CFLAGS += \
	-mcpu=cortex-m4 \
	-std=gnu11 \
	-g3 \
	-Og \
	-ffunction-sections \
	-fdata-sections \
	-Wall \
	-fno-signed-char \
	-Wno-pointer-sign \
	-fstack-usage \
	--specs=nano.specs \
	-mfpu=fpv4-sp-d16 \
	-mfloat-abi=hard \
	-mthumb

LDFLAGS += \
	-T src/stm32l432kc.ld \
	-static \
	-u \
	-lc \
	-lm \
	-Wl,--start-group \
	-Wl,--end-group \
	-Wl,-Map=$(BUILD_DIR)/$(PROJECT).map \
	-Wl,--gc-sections \
	-Wl,--print-memory-usage

DEFINES += \
	DEBUG \
	USE_FULL_ASSERT \
	STM32L432xx

CFLAGS += $(foreach i,$(INCLUDES),-I$(i))
CFLAGS += $(foreach d,$(DEFINES),-D$(d))

OBJ_DIR = $(BUILD_DIR)/objs
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all
all: $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR):
	$(NO_ECHO)$(MKDIR) -p $(BUILD_DIR)

$(OBJ_DIR):
	$(NO_ECHO)$(MKDIR) -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.s $(OBJ_DIR)
	@echo "Assembling $<"
	$(NO_ECHO)$(MKDIR) -p $(dir $@)
	$(NO_ECHO)$(CC) -c -o $@ $< $(ASMFLAGS)

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)
	@echo "Compiling $<"
	$(NO_ECHO)$(MKDIR) -p $(dir $@)
	$(NO_ECHO)$(CC) -c -o $@ $< $(CFLAGS)

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).lst
	$(OCPY) $< $@ -O binary
	$(SZ) $<

$(BUILD_DIR)/$(PROJECT).lst: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)
	$(ODUMP) -D $< > $@

$(BUILD_DIR)/$(PROJECT).elf: $(OBJS)
	@echo "Linking $@"
	$(NO_ECHO)$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: flash
flash: $(BUILD_DIR)/$(PROJECT).bin
	$(STFLASH) write $(BUILD_DIR)/$(PROJECT).bin 0x08000000
