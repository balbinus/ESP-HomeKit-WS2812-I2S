#ifdef WS2812

#include <stdio.h>
#include <string.h>
#include <math.h>  //requires LIBS ?= hal m to be added to Makefile

#include <esp8266.h>
#include <ws2812_i2s/ws2812_i2s.h>

#include <FreeRTOS.h>
#include <task.h>

#include "ws2812.h"

//http://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white
static void hsi2rgbw(float h, float s, float i, uint8_t* rgbw)
{
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

#endif /* WS2812 */