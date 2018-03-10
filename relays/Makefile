PROGRAM = main

ESP_HK_DEMO = ../esp-homekit-demo

EXTRA_COMPONENTS = \
	extras/http-parser \
	extras/dhcpserver \
	extras/i2c \
	$(abspath $(ESP_HK_DEMO)/components/wifi_config) \
	$(abspath $(ESP_HK_DEMO)/components/wolfssl) \
	$(abspath $(ESP_HK_DEMO)/components/cJSON) \
	$(abspath $(ESP_HK_DEMO)/components/homekit)

FLASH_SIZE ?= 8
HOMEKIT_SPI_FLASH_BASE_ADDR ?= 0x7a000

EXTRA_CFLAGS += -I.. -DHOMEKIT_SHORT_APPLE_UUIDS -DRELAYS -DNUM_RELAYS=4 -DWS2812 -DNUM_LEDS=124 -DBLINKM -DNUM_BLINKM=2

LIBS ?= hal m

EXTRA_COMPONENTS += extras/i2s_dma extras/ws2812_i2s

include $(SDK_PATH)/common.mk

monitor:
	$(FILTEROUTPUT) --port $(ESPPORT) --baud 115200 --elf $(PROGRAM_OUT)
