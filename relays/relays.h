#pragma once

#include <homekit/types.h>

#ifndef NUM_RELAYS
#error You need to define NUM_RELAYS
#endif

void relay_write(uint8_t idx, bool on);
void relay_init();

homekit_value_t relay_on_get(uint8_t idx);
void relay_on_set(uint8_t idx, homekit_value_t value);

#define RELAY_NAME(idx) "Relay #" #idx

#define DEFINE_CALLBACKS_RELAY(idx) \
    void relay_## idx ##_on_set(homekit_value_t v); \
    homekit_value_t relay_## idx ##_on_get();
#define IMPLEMENT_CALLBACKS_RELAY(idx) \
    void relay_## idx ##_on_set(homekit_value_t v) { relay_on_set(idx, v); } \
    homekit_value_t relay_## idx ##_on_get() { return relay_on_get(idx); }
#define DEFINE_RELAY_SERVICE(idx) \
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){ \
            HOMEKIT_CHARACTERISTIC(NAME, RELAY_NAME(idx)), \
            HOMEKIT_CHARACTERISTIC( \
                ON, false, \
                .getter=relay_## idx ##_on_get, \
                .setter=relay_## idx ##_on_set \
            ), \
            NULL \
        })

#if NUM_RELAYS > 0
    DEFINE_CALLBACKS_RELAY(0)
#if NUM_RELAYS > 1
    DEFINE_CALLBACKS_RELAY(1)
#if NUM_RELAYS > 2
    DEFINE_CALLBACKS_RELAY(2)
#if NUM_RELAYS > 3
    DEFINE_CALLBACKS_RELAY(3)
#endif /* > 3 */
#endif /* > 2 */
#endif /* > 1 */
#endif /* > 0 */