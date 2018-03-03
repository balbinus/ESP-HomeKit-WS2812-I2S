# ESP-HomeKit-WS2812-I2S
Build of ESP-HomeKit to control WS2812 LEDs via I2S

# How to build

1. ***Always*** `git clone --recursive`. Dependencies have dependencies have dependencies.
2. You probably want to get the dependencies listed here: https://github.com/pfalcon/esp-open-sdk/blob/master/README.md#requirements-and-dependencies
3. As long as SourceForge will fuck up wget, download manually https://sourceforge.net/projects/expat/files/expat/2.1.0/expat-2.1.0.tar.gz/download and move it to `esp-open-sdk/crosstool-NG/.build/tarballs/expat-2.1.0.tar.gz`.
4. Run `make base` in the root directory of this repository, it will (try to) build `esp-open-sdk`.
