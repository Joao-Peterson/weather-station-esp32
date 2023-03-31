### Macros for the idf.py tool and other utils

.PHONY : build nvs/nvs.bin

# build and flash just the app and open monitor
default :
	idf.py app app-flash monitor

# just build the app to update dependencies and add components
build :
	idf.py app

# full build
all : nvs
	idf.py build flash monitor

# open up monitor
monitor :
	idf.py monitor

# open up menuconfig
menuconfig :
	idf.py menuconfig

# makes the nvs partition image and flashes it
nvs : nvs/nvs.bin
# reset esp before and after, writes at address 0x9000, #!change this if another partition table is used
	esptool.py --before default_reset --after hard_reset write_flash --flash_size detect 0x9000 $<

# read nvs from chip to a bin file, for cehcking purposes
read-nvs : 
	esptool.py read_flash 0x9000 0x6000 nvs/nvs.read.bin

# generate nvs partition image from script, #! includes sensitive/device specific data
nvs/nvs.bin :
	./genNvs.sh