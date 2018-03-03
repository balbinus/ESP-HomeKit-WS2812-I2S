/*
 * This is example of handling SK6812 RGBW (or equivalent) LEDs with HomeKit on an ESP8266.
 * 
 * Since it's using I2S to talk to the LEDs, the pin *must be* GPIO3, aka serial RX. On an
 * Adafruit HUZZAH board, you have to bypass the diode on this pin (connect directly to the
 * ESP8266).
 */
#include <stdio.h>
#include <string.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ota-tftp.h>

#include <homekit/homekit.h>
#include <homekit/types.h>
#include <homekit/characteristics.h>

#include <ws2812_i2s/ws2812_i2s.h>

#include <math.h>  //requires LIBS ?= hal m to be added to Makefile

#include "wifi.h"

static void wifi_init() {
    struct sdk_station_config wifi_config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASSWORD,
    };

    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&wifi_config);
    sdk_wifi_station_connect();
}

homekit_characteristic_t brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 100);
homekit_characteristic_t hue = HOMEKIT_CHARACTERISTIC_(HUE, 0);
homekit_characteristic_t saturation = HOMEKIT_CHARACTERISTIC_(SATURATION, 0);

/* Refresh rate. Higher makes for flickerier
   Recommend small values for small displays */
#define FPS 17
#define FPS_DELAY (1000 / FPS / portTICK_PERIOD_MS)

#define NUM_LEDS 128

static int min(int a, int b) {
    return (a > b) ? b : a;
}

uint8_t scale(uint8_t x, uint8_t s) {
    return (((uint16_t)x) * s) >> 8;
}


//http://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white
void hsi2rgbw(float h, float s, float i, uint8_t* rgbw) {
    uint8_t r, g, b, w;
    float cos_h, cos_1047_h;
    //h = fmod(h,360); // cycle h around to 0-360 degrees
    h = M_PI*h/(float)180; // Convert to radians.
    s /=(float)100; i/=(float)100; //from percentage to ratio
    s = s>0?(s<1?s:1):0; // clamp s and i to interval [0,1]
    i = i>0?(i<1?i:1):0;
    //~ i = i*sqrt(i); //shape intensity to have finer granularity near 0

    if(h < 2.09439) {
        cos_h = cos(h);
        cos_1047_h = cos(1.047196667-h);
        r = s*255*i/3*(1+cos_h/cos_1047_h);
        g = s*255*i/3*(1+(1-cos_h/cos_1047_h));
        b = 0;
        w = 255*(1-s)*i;
    } else if(h < 4.188787) {
        h = h - 2.09439;
        cos_h = cos(h);
        cos_1047_h = cos(1.047196667-h);
        g = s*255*i/3*(1+cos_h/cos_1047_h);
        b = s*255*i/3*(1+(1-cos_h/cos_1047_h));
        r = 0;
        w = 255*(1-s)*i;
    } else {
        h = h - 4.188787;
        cos_h = cos(h);
        cos_1047_h = cos(1.047196667-h);
        b = s*255*i/3*(1+cos_h/cos_1047_h);
        r = s*255*i/3*(1+(1-cos_h/cos_1047_h));
        g = 0;
        w = 255*(1-s)*i;
    }

    rgbw[0]=r;
    rgbw[1]=g;
    rgbw[2]=b;
    rgbw[3]=w;
}

ws2812_pixel_t pixels[NUM_LEDS];
bool par64_on = false;

void par64_update()
{
    uint8_t rgbw[4];
    hsi2rgbw(hue.value.float_value, saturation.value.float_value, (float) brightness.value.int_value, rgbw);
    ws2812_pixel_t color = (ws2812_pixel_t) {
        .red = rgbw[0],
        .green = rgbw[1],
        .blue = rgbw[2],
        .white = rgbw[3]
    };
    
    for (int i = 0; i < NUM_LEDS; i++)
    {
        pixels[i] = color;
    }

    ws2812_i2s_update(pixels, PIXEL_RGBW);
}

void par64_clear()
{
    memset(pixels, 0, sizeof(pixels));
    ws2812_i2s_update(pixels, PIXEL_RGBW);
}

void par64_task(void *_arg)
{
    while (par64_on) {
        par64_update();

        vTaskDelay(FPS_DELAY);
    }

    // TODO: replace with cute turn down animation.
    par64_clear();
    vTaskDelete(NULL);
}

void par64_init()
{
    ws2812_i2s_init(NUM_LEDS, PIXEL_RGBW);
    memset(pixels, 0, sizeof(pixels));
}

void par64_start()
{
    par64_on = true;
    xTaskCreate(par64_task, "PAR 64", 256, NULL, 2, NULL);
}

void par64_identify_task(void *_args)
{
    bool old_on = par64_on;
    par64_on = false;
    vTaskDelay(2*FPS_DELAY);

    ws2812_pixel_t black = { .color=0x00000000 };
    ws2812_pixel_t red   = { .color=0x33333333 };

    par64_clear();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    for (int x = 0; x < 2; x++) {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            pixels[i] = red;
            ws2812_i2s_update(pixels, PIXEL_RGBW);

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        for (int i = 0; i < NUM_LEDS; i++)
        {
            pixels[i] = black;
            ws2812_i2s_update(pixels, PIXEL_RGBW);

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

    ws2812_i2s_update(pixels, PIXEL_RGBW);

    if (old_on)
        par64_start();

    vTaskDelete(NULL);
}

void par64_identify(homekit_value_t _value)
{
    printf("PAR 64 identify\n");
    xTaskCreate(par64_identify_task, "PAR 64 identify", 256, NULL, 2, NULL);
}

homekit_value_t par64_on_get()
{
    return HOMEKIT_BOOL(par64_on);
}

void par64_on_set(homekit_value_t value)
{
    if (value.format != homekit_format_bool) {
        printf("Invalid value for PAR 64 ON characteristic: type=%d\n", value.format);
        return;
    }

    if (value.bool_value && !par64_on) {
        par64_start();
    }
    par64_on = value.bool_value;
}


homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "PAR 64"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "balbinus"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "001"),
            HOMEKIT_CHARACTERISTIC(MODEL, "NeoPAR64"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, par64_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "PAR 64"),
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
        NULL
    }),
    NULL
};


homekit_server_config_t config = {
    .accessories = accessories,
    .password = "123-45-678"
};

void user_init(void) {
    uart_set_baud(0, 115200);

    wifi_init();
    par64_init();
    par64_start();
    homekit_server_init(&config);
}
