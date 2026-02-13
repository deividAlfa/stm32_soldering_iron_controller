##############################################################################
# STM32 Soldering Iron Controller - Makefile
# Target: KSGER v3 STM32F101CBTx with SSD1306 display
##############################################################################

######################################
# Target
######################################
TARGET = STM32SolderingStation

######################################
# Building variables
######################################
# Optimization
OPT = -Os

# Debug (1=yes, 0=no)
DEBUG = 1

######################################
# Paths
######################################
# Build output directory
BUILD_DIR = build

# Toolchain path
TOOLCHAIN_PATH = /usr/local/bin

######################################
# Toolchain
######################################
PREFIX = arm-none-eabi-
ifdef TOOLCHAIN_PATH
CC = $(TOOLCHAIN_PATH)/$(PREFIX)gcc
AS = $(TOOLCHAIN_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(TOOLCHAIN_PATH)/$(PREFIX)objcopy
SZ = $(TOOLCHAIN_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

######################################
# Source files
######################################

# ASM sources (startup)
ASM_SOURCES = \
Core/Startup/startup_stm32f101c8tx.s

# C sources - Core/Src
C_SOURCES = \
Core/Src/display_test.c \
Core/Src/iron.c \
Core/Src/main.c \
Core/Src/pid.c \
Core/Src/settings.c \
Core/Src/stm32f1xx_hal_msp.c \
Core/Src/stm32f1xx_it.c \
Core/Src/syscalls.c \
Core/Src/sysmem.c \
Core/Src/system_stm32f1xx.c \
Core/Src/user_main.c

# C sources - Drivers/generalIO
C_SOURCES += \
Drivers/generalIO/adc_global.c \
Drivers/generalIO/buzzer.c \
Drivers/generalIO/rotary_encoder.c \
Drivers/generalIO/tempsensors.c \
Drivers/generalIO/voltagesensors.c

# C sources - Drivers/graphics
C_SOURCES += \
Drivers/graphics/display.c \
Drivers/graphics/gui/gui.c \
Drivers/graphics/gui/oled.c \
Drivers/graphics/gui/screen.c \
Drivers/graphics/gui/widgets.c \
Drivers/graphics/gui/screens/addon_fume_extractor_settings_screen.c \
Drivers/graphics/gui/screens/addon_switch_off_reminder_settings_screen.c \
Drivers/graphics/gui/screens/addons_screen.c \
Drivers/graphics/gui/screens/boot_screen.c \
Drivers/graphics/gui/screens/calibration_screen.c \
Drivers/graphics/gui/screens/debug_screen.c \
Drivers/graphics/gui/screens/display_screen.c \
Drivers/graphics/gui/screens/iron_screen.c \
Drivers/graphics/gui/screens/main_screen.c \
Drivers/graphics/gui/screens/reset_screen.c \
Drivers/graphics/gui/screens/screen_common.c \
Drivers/graphics/gui/screens/settings_screen.c \
Drivers/graphics/gui/screens/tip_list_screen.c \
Drivers/graphics/gui/screens/tip_settings_screen.c \
Drivers/graphics/gui/screens/gui_strings.c \
Drivers/graphics/gui/screens/system_screen.c

# C sources - Drivers/graphics/u8g2 (all 103 .c files in the u8g2 directory)
C_SOURCES += \
Drivers/graphics/u8g2/u8g2_bitmap.c \
Drivers/graphics/u8g2/u8g2_box.c \
Drivers/graphics/u8g2/u8g2_buffer.c \
Drivers/graphics/u8g2/u8g2_circle.c \
Drivers/graphics/u8g2/u8g2_cleardisplay.c \
Drivers/graphics/u8g2/u8g2_d_memory.c \
Drivers/graphics/u8g2/u8g2_d_setup.c \
Drivers/graphics/u8g2/u8g2_font.c \
Drivers/graphics/u8g2/u8g2_fonts.c \
Drivers/graphics/u8g2/u8g2_hvline.c \
Drivers/graphics/u8g2/u8g2_input_value.c \
Drivers/graphics/u8g2/u8g2_intersection.c \
Drivers/graphics/u8g2/u8g2_kerning.c \
Drivers/graphics/u8g2/u8g2_line.c \
Drivers/graphics/u8g2/u8g2_ll_hvline.c \
Drivers/graphics/u8g2/u8g2_message.c \
Drivers/graphics/u8g2/u8g2_polygon.c \
Drivers/graphics/u8g2/u8g2_selection_list.c \
Drivers/graphics/u8g2/u8g2_setup.c \
Drivers/graphics/u8g2/u8log.c \
Drivers/graphics/u8g2/u8log_u8g2.c \
Drivers/graphics/u8g2/u8log_u8x8.c \
Drivers/graphics/u8g2/u8x8_8x8.c \
Drivers/graphics/u8g2/u8x8_byte.c \
Drivers/graphics/u8g2/u8x8_cad.c \
Drivers/graphics/u8g2/u8x8_capture.c \
Drivers/graphics/u8g2/u8x8_d_a2printer.c \
Drivers/graphics/u8g2/u8x8_d_hd44102.c \
Drivers/graphics/u8g2/u8x8_d_il3820_296x128.c \
Drivers/graphics/u8g2/u8x8_d_ist3020.c \
Drivers/graphics/u8g2/u8x8_d_ist7920.c \
Drivers/graphics/u8g2/u8x8_d_ks0108.c \
Drivers/graphics/u8g2/u8x8_d_lc7981.c \
Drivers/graphics/u8g2/u8x8_d_ld7032_60x32.c \
Drivers/graphics/u8g2/u8x8_d_ls013b7dh03.c \
Drivers/graphics/u8g2/u8x8_d_max7219.c \
Drivers/graphics/u8g2/u8x8_d_pcd8544_84x48.c \
Drivers/graphics/u8g2/u8x8_d_pcf8812.c \
Drivers/graphics/u8g2/u8x8_d_pcf8814_hx1230.c \
Drivers/graphics/u8g2/u8x8_d_s1d15721.c \
Drivers/graphics/u8g2/u8x8_d_s1d15e06.c \
Drivers/graphics/u8g2/u8x8_d_sbn1661.c \
Drivers/graphics/u8g2/u8x8_d_sed1330.c \
Drivers/graphics/u8g2/u8x8_d_sh1106_64x32.c \
Drivers/graphics/u8g2/u8x8_d_sh1106_72x40.c \
Drivers/graphics/u8g2/u8x8_d_sh1107.c \
Drivers/graphics/u8g2/u8x8_d_sh1108.c \
Drivers/graphics/u8g2/u8x8_d_sh1122.c \
Drivers/graphics/u8g2/u8x8_d_ssd1305.c \
Drivers/graphics/u8g2/u8x8_d_ssd1306_128x32.c \
Drivers/graphics/u8g2/u8x8_d_ssd1306_128x64_noname.c \
Drivers/graphics/u8g2/u8x8_d_ssd1306_2040x16.c \
Drivers/graphics/u8g2/u8x8_d_ssd1306_48x64.c \
Drivers/graphics/u8g2/u8x8_d_ssd1306_64x32.c \
Drivers/graphics/u8g2/u8x8_d_ssd1306_64x48.c \
Drivers/graphics/u8g2/u8x8_d_ssd1306_72x40.c \
Drivers/graphics/u8g2/u8x8_d_ssd1306_96x16.c \
Drivers/graphics/u8g2/u8x8_d_ssd1309.c \
Drivers/graphics/u8g2/u8x8_d_ssd1316.c \
Drivers/graphics/u8g2/u8x8_d_ssd1317.c \
Drivers/graphics/u8g2/u8x8_d_ssd1318.c \
Drivers/graphics/u8g2/u8x8_d_ssd1320.c \
Drivers/graphics/u8g2/u8x8_d_ssd1322.c \
Drivers/graphics/u8g2/u8x8_d_ssd1325.c \
Drivers/graphics/u8g2/u8x8_d_ssd1326.c \
Drivers/graphics/u8g2/u8x8_d_ssd1327.c \
Drivers/graphics/u8g2/u8x8_d_ssd1329.c \
Drivers/graphics/u8g2/u8x8_d_ssd1606_172x72.c \
Drivers/graphics/u8g2/u8x8_d_ssd1607_200x200.c \
Drivers/graphics/u8g2/u8x8_d_st7511.c \
Drivers/graphics/u8g2/u8x8_d_st75256.c \
Drivers/graphics/u8g2/u8x8_d_st7528.c \
Drivers/graphics/u8g2/u8x8_d_st75320.c \
Drivers/graphics/u8g2/u8x8_d_st7565.c \
Drivers/graphics/u8g2/u8x8_d_st7567.c \
Drivers/graphics/u8g2/u8x8_d_st7571.c \
Drivers/graphics/u8g2/u8x8_d_st7586s_erc240160.c \
Drivers/graphics/u8g2/u8x8_d_st7586s_s028hn118a.c \
Drivers/graphics/u8g2/u8x8_d_st7586s_ymc240160.c \
Drivers/graphics/u8g2/u8x8_d_st7588.c \
Drivers/graphics/u8g2/u8x8_d_st7920.c \
Drivers/graphics/u8g2/u8x8_d_stdio.c \
Drivers/graphics/u8g2/u8x8_d_t6963.c \
Drivers/graphics/u8g2/u8x8_d_uc1601.c \
Drivers/graphics/u8g2/u8x8_d_uc1604.c \
Drivers/graphics/u8g2/u8x8_d_uc1608.c \
Drivers/graphics/u8g2/u8x8_d_uc1610.c \
Drivers/graphics/u8g2/u8x8_d_uc1611.c \
Drivers/graphics/u8g2/u8x8_d_uc1617.c \
Drivers/graphics/u8g2/u8x8_d_uc1638.c \
Drivers/graphics/u8g2/u8x8_d_uc1701_dogs102.c \
Drivers/graphics/u8g2/u8x8_d_uc1701_mini12864.c \
Drivers/graphics/u8g2/u8x8_debounce.c \
Drivers/graphics/u8g2/u8x8_display.c \
Drivers/graphics/u8g2/u8x8_fonts.c \
Drivers/graphics/u8g2/u8x8_gpio.c \
Drivers/graphics/u8g2/u8x8_input_value.c \
Drivers/graphics/u8g2/u8x8_message.c \
Drivers/graphics/u8g2/u8x8_selection_list.c \
Drivers/graphics/u8g2/u8x8_setup.c \
Drivers/graphics/u8g2/u8x8_string.c \
Drivers/graphics/u8g2/u8x8_u16toa.c \
Drivers/graphics/u8g2/u8x8_u8toa.c

# C sources - Drivers/addons
C_SOURCES += \
Drivers/addons/addon_fume_extractor.c \
Drivers/addons/addon_switch_off_reminder.c

# C sources - STM32F1xx HAL Driver (selected modules only)
C_SOURCES += \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_crc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_iwdg.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c

######################################
# MCU configuration
######################################

# CPU
CPU = -mcpu=cortex-m3

# FPU (none for Cortex-M3)

# Float ABI (none for Cortex-M3)

# MCU flags
MCU = $(CPU) -mthumb

######################################
# C defines
######################################
C_DEFS = \
-DUSE_HAL_DRIVER \
-DSTM32F101xB \
-DSSD1306 \
-DENABLE_ADDON_SWITCH_OFF_REMINDER

######################################
# Include paths
######################################
C_INCLUDES = \
-ICore/Inc \
-IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
-IDrivers/CMSIS/Include \
-IDrivers/generalIO \
-IDrivers/graphics \
-IDrivers/graphics/gui \
-IDrivers/graphics/gui/screens \
-IDrivers/graphics/u8g2 \
-IDrivers/STM32F1xx_HAL_Driver/Inc \
-IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy \
-IDrivers/addons

######################################
# Compiler flags
######################################

# ASM flags
ASFLAGS = $(MCU) $(OPT) -Wall -fdata-sections -ffunction-sections

# C flags
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT)
CFLAGS += -Wno-builtin-macro-redefined
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall
CFLAGS += -fstack-usage
CFLAGS += --specs=nano.specs

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency files
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

######################################
# Linker flags
######################################

# Linker script
LDSCRIPT = _STM32F101CBTX_FLASH.ld

# Libraries
LIBS = -lc -lm -lnosys
LIBDIR =

LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += --specs=nano.specs --specs=nosys.specs
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/output.map

######################################
# Build rules
######################################

# Default target
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

# List of C objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# List of ASM objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir -p $@

######################################
# Clean
######################################
clean:
	rm -rf $(BUILD_DIR)

######################################
# Flash (using st-flash or openocd)
######################################
flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $< 0x08000000

flash-ocd: $(BUILD_DIR)/$(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c "program $< verify reset exit"

######################################
# Dependencies
######################################
-include $(wildcard $(BUILD_DIR)/*.d)

######################################
# Phony targets
######################################
.PHONY: all clean flash flash-ocd
