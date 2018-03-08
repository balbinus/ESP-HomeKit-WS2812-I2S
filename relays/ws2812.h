#pragma once

#include <homekit/types.h>
#include <homekit/characteristics.h>

extern homekit_characteristic_t brightness;
extern homekit_characteristic_t hue;
extern homekit_characteristic_t saturation;

#ifndef NUM_LEDS
#error You need to define NUM_LEDS
#endif

/* Refresh rate. Higher makes for flickerier
   Recommend small values for small displays */
#define FPS 17
#define FPS_DELAY (1000 / FPS / portTICK_PERIOD_MS)

void par64_init();
void par64_start();

homekit_value_t par64_on_get();
void par64_on_set(homekit_value_t value);