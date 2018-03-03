export PATH := $(PATH):$(PWD)/esp-open-sdk/xtensa-lx106-elf/bin
export SDK_PATH := $(PWD)/esp-open-rtos

all: base par64

base: esp-open-sdk

esp-open-sdk:
	cd $@ && make toolchain esptool libhal
	# Not using STANDALONE=n since it doesn't build with it...

par64:
	$(MAKE) -C $@ clean all
	
.PHONY: base esp-open-sdk par64