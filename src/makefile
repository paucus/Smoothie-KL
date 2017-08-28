PROJECT=main
BUILD_DIR=../build
DEVICES=lpc1768

__VERSION_NUMBER__=1.0.0

# BUILD_TYPE can be set to the following values:
#  Checked - Optimizations enabled with MRI debug monitor support. (Recommended Type)
#  Release - Optimizations enabled.
#  Debug - Optimization disabled with MRI debug monitor support.
BUILD_TYPE=Checked

# Set to 1 to tag each heap allocation with the caller's return address.
# NOTE: Can't be enabled with latest build as not compatible with newlib nano.
HEAP_TAGS=0

# Set to 1 configure MPU to disable write buffering and eliminate imprecise bus faults.
WRITE_BUFFER_DISABLE=0

# Set to non zero value if you want checks to be enabled which reserve a
# specific amount of space for the stack.  The heap's growth will be
# constrained to reserve this much space for the stack and the stack won't be
# able to grow larger than this amount.
STACK_SIZE=3072

# Set to 1 to allow MRI debug monitor to take full control of UART0 and use it
# as a dedicated debug channel.  If you are using the USB based serial port for
# the console then this should cause you no problems.  Set MRI_BREAK_ON_INIT to
# 0 if you don't want to break into GDB at startup.
ENABLE_DEBUG_MONITOR?=0

# BEGIN MODIF config_cache_preallocation
# This value should more or less reflect the number of elements the ConfigCache
# will have in memory. The purpose of it is to avoid fragmentation in memory,
# which is one of the reasons that cause memory errors during boot.
# When booting with debug enabled you can check the value printed with
# "ConfigCache size: xxx". This value is the number of configuration items in
# memory in the moment when most elements are loaded, so, that is a good hint
# of this value. Add a few elements more (20 items for examle) and that should
# be enough.
CONFIG_CACHE_PREALLOCATION=190
DEFINES += -DCONFIG_CACHE_PREALLOCATION=$(CONFIG_CACHE_PREALLOCATION)
# END MODIF config_cache_preallocation

# this is the default UART baud rate used if it is not set in config
# it is also the baud rate used to report any errors found while parsing the config file
DEFAULT_SERIAL_BAUD_RATE?=115200

# BEGIN MODIF external sd
# Set to 1 to use two sd cards: one for the configuration in SPI0 and one for files in SPI1.
SPLIT_CONFIG_AND_PUBLIC_SD?=1
ifeq "$(SPLIT_CONFIG_AND_PUBLIC_SD)" "1"
CONFIG_SD_MOUNT_DIR='"sd"'
PUBLIC_SD_MOUNT_DIR='"sd2"'
else
CONFIG_SD_MOUNT_DIR='"sd"'
PUBLIC_SD_MOUNT_DIR='"sd"'
endif
# END MODIF external sd
# BEGIN MODIF lcd
LCD_DISPLAY_SERIAL?=0
ifeq "$(LCD_DISPLAY_SERIAL)" "1"
LCD_TX_PIN=unset_pin
LCD_RX_PIN=unset_pin
LCD_RST_PIN=P2_12
LCD_BAUDRATE=unset_baudrate
LCD_INIT_BAUDRATE=unset_baudrate
else
LCD_TX_PIN=P4_28
LCD_RX_PIN=P4_29
LCD_RST_PIN=P2_12
LCD_INIT_BAUDRATE=9600
LCD_BAUDRATE=115200
endif
# END MODIF lcd

ifeq "$(ENABLE_DEBUG_MONITOR)" "1"
# Can add MRI_UART_BAUD=115200 to next line if GDB fails to connect to MRI.
# Tends to happen on some Linux distros but not Windows and OS X.
MRI_UART=MRI_UART_0
MRI_BREAK_ON_INIT=1
MRI_SEMIHOST_STDIO=1
else
MRI_UART=MRI_UART_0 MRI_UART_SHARE
MRI_BREAK_ON_INIT=0
MRI_SEMIHOST_STDIO=0
endif

# generate a git version string, sorry only on linux and mac osx
ifneq "$(OS)" "Windows_NT"
DEFINES += -D__GITVERSIONSTRING__=\"$(shell ./generate-version.sh)\"
else
DEFINES += -D__GITVERSIONSTRING__=\"$(shell generate-version.bat)\"
endif
# generate a git version string, sorry only on linux and mac osx
ifneq "$(OS)" "Windows_NT"
DEFINES += -D__TIMESTAMP_BUILD__=\"$(shell date '+%Y%m%d%H%M%S')\"
else
DEFINES += -D__TIMESTAMP_BUILD__=\"placeholder\"
endif

# use c++11 features for the checksums and set default baud rate for serial uart
DEFINES += -DCHECKSUM_USE_CPP -DDEFAULT_SERIAL_BAUD_RATE=$(DEFAULT_SERIAL_BAUD_RATE) -DCONFIG_SD_MOUNT_DIR=${CONFIG_SD_MOUNT_DIR} -DPUBLIC_SD_MOUNT_DIR=${PUBLIC_SD_MOUNT_DIR} -DSPLIT_CONFIG_AND_PUBLIC_SD=${SPLIT_CONFIG_AND_PUBLIC_SD} -DLCD_DISPLAY_SERIAL=${LCD_DISPLAY_SERIAL} -DLCD_TX_PIN=${LCD_TX_PIN} -DLCD_RX_PIN=${LCD_RX_PIN} -DLCD_RST_PIN=${LCD_RST_PIN} -DLCD_BAUDRATE=${LCD_BAUDRATE} -DLCD_INIT_BAUDRATE=${LCD_INIT_BAUDRATE} -D__VERSION_NUMBER__=\"${__VERSION_NUMBER__}\"

ifneq "$(STEPTICKER_DEBUG_PIN)" ""
# Set a Pin here that toggles on end of move
DEFINES += -DSTEPTICKER_DEBUG_PIN=$(STEPTICKER_DEBUG_PIN)
endif

# add any modules that you do not want included in the build
# BEGIN MODIF excluded_modules
# We need to disable some modules to make the firmware fit in memory
# For some reason EXCLUDED_MODULES doesn't seem to work. Use the DEFINES below instead appending for each xxxx/yyyy a -DNO_XXXX_YYYY (in uppercase).
# Nevertheless, keep adding discarded modules here just in case we find a way to make it work again.
EXCLUDED_MODULES = tools/touchprobe tools/scaracal tools/statusreport utils/panel tools/laser utils/pausebutton
DEFINES += -DNO_NETWORK_WEBSERVER -DNO_NETWORK_PLAN9 -DNO_NETWORK_SFTPD -DNO_TOOLS_STATUSREPORT -DNO_UTILS_PANEL -DNO_TOOLS_LASER -DNO_UTILS_PAUSEBUTTON -DNO_TOOLS_TOUCHPROBE -DNO_TOOLS_SCARACAL
# e.g for a CNC machine
#export EXCLUDED_MODULES = tools/touchprobe tools/laser tools/temperaturecontrol tools/extruder
# END MODIF excluded_modules

ifneq "$(INCLUDE_MODULE)" ""
export EXCLUDED_MODULES = $(filter-out $(INCLUDE_MODULE),$(EXCLUDE_MODULES))
else
export EXCLUDED_MODULES = $(EXCLUDE_MODULES)
endif


# set to not compile in any network support
#export NONETWORK = 1

include $(BUILD_DIR)/build.mk

#CONSOLE?=/dev/ttyUSB0
ifneq "$(OS)" "Windows_NT"
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
		CONSOLE?=$(shell echo /dev/tty.usbserial*)
    else
		CONSOLE?=$(shell echo /dev/ttyUSB*)
    endif
endif

BAUD?=115200

.PHONY: debug-store flash upload debug console dfu

debug-store: ../LPC1768/$(PROJECT).elf
	cp ../LPC1768/$(PROJECT).elf ../LPC1768/$(PROJECT)_lastupload.elf

flash: ../LPC1768/$(PROJECT).hex debug-store
	lpc21isp $< $(CONSOLE) 115200 12000

dfu: ../LPC1768/$(PROJECT).bin
	dfu-util -D $< -R

upload: ../LPC1768/$(PROJECT).bin debug-store
	sudo /usr/bin/dfu-util -R -d 1d50:6015 -D $<

debug: ../LPC1768/$(PROJECT)_lastupload.elf
	arm-none-eabi-gdbtui $< --baud 115200 -ex  "set target-charset ASCII" -ex "set remotelogfile /tmp/mri.log" -ex "target remote $(CONSOLE)"

console:
	stty raw ignbrk -echo $(BAUD) < $(CONSOLE)
	( cat <&3 & cat >&3 ; kill %% ) 3<>$(CONSOLE)
