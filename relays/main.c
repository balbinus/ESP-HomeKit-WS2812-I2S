#include <stdio.h>
#include <string.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <homekit/homekit.h>
#include <homekit/types.h>
#include <homekit/characteristics.h>

#include <wifi_config.h>

#ifdef WS2812
#include "ws2812.h"

#define HOMEKIT_CHARACTERISTIC_CALLBACK_CONTEXT(f, c) &(homekit_characteristic_change_callback_t) { .function = f, .context = c }

homekit_characteristic_t brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 100);
homekit_characteristic_t hue = HOMEKIT_CHARACTERISTIC_(HUE, 0);
homekit_characteristic_t saturation = HOMEKIT_CHARACTERISTIC_(SATURATION, 0);
#endif

#ifdef RELAYS
#include "relays.h"
#endif

#ifdef BLINKM
#include "blinkm.h"
#endif

#ifndef ONBOARD_LED_GPIO
#define ONBOARD_LED_GPIO 2
#endif

#define ACCESSORY_NAME "BLBNS ESP8266 HomeKit"

void onboard_led_init()
{
    gpio_enable(ONBOARD_LED_GPIO, GPIO_OUTPUT);
    gpio_write(ONBOARD_LED_GPIO, 1);
}

void accessory_identify_task(void *_args)
{
    // Do NOT attempt that on the relays. Waaaay too fast. For them or their loads.
    for (int i=0; i<3; i++)
    {
        for (int j=0; j<2; j++)
        {
            gpio_write(ONBOARD_LED_GPIO, 0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            gpio_write(ONBOARD_LED_GPIO, 1);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    gpio_write(ONBOARD_LED_GPIO, 1);
    vTaskDelete(NULL);
}

void accessory_identify(homekit_value_t _value)
{
    printf("Accessory identify\n");
    xTaskCreate(accessory_identify_task, "Accessory identify", 128, NULL, 2, NULL);
}

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, ACCESSORY_NAME);
homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            &name,
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "balbinus"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "F61F7B271F90"), // FIXME FIXME
            HOMEKIT_CHARACTERISTIC(MODEL, ACCESSORY_NAME),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
            NULL
        }),
#ifdef RELAYS
#if NUM_RELAYS > 0
        DEFINE_RELAY_SERVICE(0),
#if NUM_RELAYS > 1
        DEFINE_RELAY_SERVICE(1),
#if NUM_RELAYS > 2
        DEFINE_RELAY_SERVICE(2),
#if NUM_RELAYS > 3
        DEFINE_RELAY_SERVICE(3),
#endif /* > 3 */
#endif /* > 2 */
#endif /* > 1 */
#endif /* > 0 */
#endif /* RELAYS */
#ifdef WS2812
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "WS2812"),
            HOMEKIT_CHARACTERISTIC(
                ON, false,
                .getter=par64_on_get,
                .setter=par64_on_set
            ),
            &brightness,
            &hue,
            &saturation,
            NULL
        }),
#endif
#ifdef BLINKM
#if NUM_BLINKM > 0
        DEFINE_BLINKM_SERVICE(0),
#if NUM_BLINKM > 1
        DEFINE_BLINKM_SERVICE(1),
#if NUM_BLINKM > 2
        DEFINE_BLINKM_SERVICE(2),
#if NUM_BLINKM > 3
        DEFINE_BLINKM_SERVICE(3),
#endif /* > 3 */
#endif /* > 2 */
#endif /* > 1 */
#endif /* > 0 */
#endif /* BLINKM */
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-123" // FIXME FIXME
};

void on_wifi_ready()
{
    homekit_server_init(&config);
}

void create_accessory_name()
{
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);
    
    int name_len = snprintf(NULL, 0, ACCESSORY_NAME " %02X:%02X:%02X",
                            macaddr[3], macaddr[4], macaddr[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, ACCESSORY_NAME " %02X:%02X:%02X",
             macaddr[3], macaddr[4], macaddr[5]);
    
    name.value = HOMEKIT_STRING(name_value);
}

void user_init(void)
{
    uart_set_baud(0, 115200);
    
    //~ homekit_server_reset();
    create_accessory_name();
    
    wifi_config_init(ACCESSORY_NAME, NULL, on_wifi_ready);

    onboard_led_init();
#ifdef RELAYS
    relay_init();
#endif
#ifdef WS2812
    par64_init();
    par64_start();
#endif
#ifdef BLINKM
    blinkm_init();
#endif
}
