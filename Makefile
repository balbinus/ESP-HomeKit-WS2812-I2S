export PATH := $(PATH):$(PWD)/esp-open-sdk/xtensa-lx106-elf/bin
export SDK_PATH := $(PWD)/esp-open-rtos

all: base par64

base: esp-open-sdk

esp-open-sdk:
	# Not using STANDALONE=n since it doesn't build with it...
	# AAAAND for some reason make -C doesn't work?!
	cd $@ && make toolchain esptool libhal

par64:
	$(MAKE) -C $@ clean all
	
.PHONY: base esp-open-sdk par64