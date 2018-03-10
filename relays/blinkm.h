#pragma once

#include <homekit/types.h>

#define BLINKM_BASE_ADDR (0x20)
#define BLINKM_BUS       (0)
#define BLINKM_SDA_PIN   (4)
#define BLINKM_SCL_PIN   (5)

typedef struct {
    uint8_t address;
    bool    on;
    float   brightness;
    float   hue;
    float   saturation;
} blinkm_t;

#ifndef NUM_BLINKM
#error You need to define NUM_BLINKM
#endif

blinkm_t blinkms[NUM_BLINKM];

void blinkm_init();

void blinkm_on_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context);
void blinkm_brightness_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context);
void blinkm_hue_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context);
void blinkm_saturation_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context);

#define DEFINE_BLINKM_SERVICE(idx) \
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){ \
            HOMEKIT_CHARACTERISTIC(NAME, "BlinkM #" #idx), \
            HOMEKIT_CHARACTERISTIC( \
                ON, false, \
                .callback=HOMEKIT_CHARACTERISTIC_CALLBACK_CONTEXT(blinkm_on_cb, &blinkms[idx]) \
            ), \
            HOMEKIT_CHARACTERISTIC( \
                BRIGHTNESS, 100, \
                .callback=HOMEKIT_CHARACTERISTIC_CALLBACK_CONTEXT(blinkm_brightness_cb, &blinkms[idx]) \
            ), \
            HOMEKIT_CHARACTERISTIC( \
                HUE, 0, \
                .callback=HOMEKIT_CHARACTERISTIC_CALLBACK_CONTEXT(blinkm_hue_cb, &blinkms[idx]) \
            ), \
            HOMEKIT_CHARACTERISTIC( \
                SATURATION, 0, \
                .callback=HOMEKIT_CHARACTERISTIC_CALLBACK_CONTEXT(blinkm_saturation_cb, &blinkms[idx]) \
            ), \
            NULL \
        })
