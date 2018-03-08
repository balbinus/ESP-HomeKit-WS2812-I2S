#include <stdio.h>
#include <string.h>
#include <esp8266.h>

#include "relays.h"

const int relay_gpio[NUM_RELAYS] = {12, 16};
bool relay_on[NUM_RELAYS] = {false, false};


void relay_write(uint8_t idx, bool on)
{
    gpio_write(relay_gpio[idx], on ? 1 : 0);
}

void relay_init()
{
    for (uint8_t i = 0 ; i < NUM_RELAYS ; i++)
    {
        gpio_enable(relay_gpio[i], GPIO_OUTPUT);
        relay_write(i, relay_on[i]);
    }
}

homekit_value_t relay_on_get(uint8_t idx)
{
    if (idx >= NUM_RELAYS)
    {
        printf("Invalid relay index: %u\n", idx);
        return HOMEKIT_BOOL(false);
    }
    
    return HOMEKIT_BOOL(relay_on[idx]);
}

void relay_on_set(uint8_t idx, homekit_value_t value)
{
    if (value.format != homekit_format_bool)
    {
        printf("Invalid value format: %d\n", value.format);
        return;
    }
    if (idx >= NUM_RELAYS)
    {
        printf("Invalid relay index: %u\n", idx);
        return;
    }

    relay_on[idx] = value.bool_value;
    relay_write(idx, relay_on[idx]);
}

#if NUM_RELAYS > 0
    IMPLEMENT_CALLBACKS_RELAY(0)
#if NUM_RELAYS > 1
    IMPLEMENT_CALLBACKS_RELAY(1)
#if NUM_RELAYS > 2
    IMPLEMENT_CALLBACKS_RELAY(2)
#if NUM_RELAYS > 3
    IMPLEMENT_CALLBACKS_RELAY(3)
#endif /* > 3 */
#endif /* > 2 */
#endif /* > 1 */
#endif /* > 0 */