#include <stdio.h>
#include <string.h>
#include <esp8266.h>

#include "relays.h"

#define MAX_RELAYS 4
#if NUM_RELAYS > MAX_RELAYS
#error You want more than 4 relays? Cool, then go define some pins in relays.c.
#endif
const int relay_gpio[MAX_RELAYS] = {12, 16, 13, 14};

void relay_write(relay_t *rl)
{
    gpio_write(rl->pin, rl->on ? 1 : 0);
}

void relay_init_one(relay_t *rl, uint8_t pin)
{
    rl->pin = pin;
    rl->on = false;
    
    gpio_enable(rl->pin, GPIO_OUTPUT);
    relay_write(rl);
}

void relay_init()
{
    for (uint8_t i = 0 ; i < NUM_RELAYS ; i++)
    {
        relay_init_one(&relays[i], relay_gpio[i]);
    }
}

void relay_on_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context)
{
    relay_t *rl = (relay_t *) context;
    
    if (value.format != homekit_format_bool)
    {
        printf("Invalid value format: %d\n", value.format);
        return;
    }
    
    rl->on = value.bool_value;
    relay_write(rl);
}
