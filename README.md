# ESP-HomeKit-WS2812-I2S
Started as a documentation of my build of ESP-HomeKit to control WS2812 LEDs via I2S, now does a little more. But mostly serves as a base of a « working state ».

## How to build the SDK

1. ***Always*** `git clone --recursive`. Dependencies have dependencies have dependencies.
2. You probably want to get the dependencies listed here: https://github.com/pfalcon/esp-open-sdk/blob/master/README.md#requirements-and-dependencies
3. As long as SourceForge will fuck up wget, download manually https://sourceforge.net/projects/expat/files/expat/2.1.0/expat-2.1.0.tar.gz/download and move it to `esp-open-sdk/crosstool-NG/.build/tarballs/expat-2.1.0.tar.gz`.
4. Run `make base` in the root directory of this repository, it will (try to) build `esp-open-sdk`.

## How to build individual projects

- To clean up (the Makefile isn't very good at picking up changes): `make -C <directory> clean`
- To build: `make -C <directory> all`
- To flash the ESP8266: `make -C <directory> flash`
- To flash and connect through serial: `make -C <directory> test`

`test` depends on `all` + `flash` + `monitor`.
