#pragma once

#ifndef ONBOARD_LED_GPIO
#define ONBOARD_LED_GPIO 2
#endif

void onboard_led_init();
void accessory_identify_task(void *_args);
void accessory_identify(homekit_value_t _value);