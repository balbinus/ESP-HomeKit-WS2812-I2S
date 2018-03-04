#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "wifi.h"

#define NUM_RELAYS 2

static void wifi_init() {
    struct sdk_station_config wifi_config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASSWORD
    };

    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&wifi_config);
    sdk_wifi_station_connect();
}

const int led_gpio = 2;

const int relay_gpio[NUM_RELAYS] = {12, 16};
bool relay_on[NUM_RELAYS] = {false, false};

void relay_write(uint8_t idx, bool on)
{
    gpio_write(relay_gpio[idx], on ? 1 : 0);
}

void relay_init()
{
    gpio_enable(led_gpio, GPIO_OUTPUT);
    gpio_write(led_gpio, 1);
    
    for (uint8_t i = 0 ; i < NUM_RELAYS ; i++)
    {
        gpio_enable(relay_gpio[i], GPIO_OUTPUT);
        relay_write(i, relay_on[i]);
    }
}

void relay_identify_task(void *_args)
{
    // Do NOT attempt that on the relays. Waaaay too fast. For them or their loads.
    for (int i=0; i<3; i++)
    {
        for (int j=0; j<2; j++)
        {
            gpio_write(led_gpio, 0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            gpio_write(led_gpio, 1);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    gpio_write(led_gpio, 1);
    vTaskDelete(NULL);
}

void relay_identify(homekit_value_t _value)
{
    printf("Relay identify\n");
    xTaskCreate(relay_identify_task, "Relay identify", 128, NULL, 2, NULL);
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
// Partial applications in C?
static void relay_1_on_set(homekit_value_t v) { relay_on_set(0, v); }
static homekit_value_t relay_1_on_get() { return relay_on_get(0); }
static void relay_2_on_set(homekit_value_t v) { relay_on_set(1, v); }
static homekit_value_t relay_2_on_get() { return relay_on_get(1); }

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Cubes Salon"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "balbinus"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "F61F7B271F90"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Relay Board"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, relay_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Cube Bas"),
            HOMEKIT_CHARACTERISTIC(
                ON, false,
                .getter=relay_1_on_get,
                .setter=relay_1_on_set
            ),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Cube Haut"),
            HOMEKIT_CHARACTERISTIC(
                ON, false,
                .getter=relay_2_on_get,
                .setter=relay_2_on_set
            ),
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-123"
};

void user_init(void) {
    uart_set_baud(0, 115200);

    wifi_init();
    relay_init();
    //~ homekit_server_reset();
    homekit_server_init(&config);
}
