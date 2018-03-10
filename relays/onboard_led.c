#include <stdio.h>

#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <homekit/types.h>

#include "onboard_led.h"

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
