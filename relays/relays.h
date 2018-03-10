#pragma once

#include <homekit/types.h>

typedef struct {
    uint8_t pin;
    bool on;
} relay_t;

#ifndef NUM_RELAYS
#error You need to define NUM_RELAYS
#endif

relay_t relays[NUM_RELAYS];

void relay_init();

void relay_on_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context);

#define DEFINE_RELAY_SERVICE(idx) \
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){ \
            HOMEKIT_CHARACTERISTIC(NAME, "Relay #" #idx), \
            HOMEKIT_CHARACTERISTIC( \
                ON, false, \
                .callback=HOMEKIT_CHARACTERISTIC_CALLBACK_CONTEXT(relay_on_cb, &relays[idx]) \
            ), \
            NULL \
        })
