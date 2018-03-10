#include <stdio.h>
#include <math.h>

#include <i2c/i2c.h>

#include "blinkm.h"

void blinkm_update(blinkm_t *bkm);

static void blinkm_init_one(blinkm_t *bkm, uint8_t address)
{
    bkm->address = address;
    bkm->on = false;
    bkm->brightness = 100;
    bkm->hue = 0;
    bkm->saturation = 0;
    
    blinkm_update(bkm);
}

void blinkm_init()
{
    i2c_init(BLINKM_BUS, BLINKM_SCL_PIN, BLINKM_SDA_PIN, I2C_FREQ_100K);
    i2c_set_clock_stretch(BLINKM_BUS, 20);
    
    for (uint8_t i = 0 ; i < NUM_BLINKM ; i++)
    {
        blinkm_init_one(&blinkms[i], BLINKM_BASE_ADDR + i);
        //~ printf(">>> Initted BlinkM #%u: 0x%02x\n", i, blinkms[i].address);
    }
}

void blinkm_on_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context)
{
    blinkm_t *bkm = (blinkm_t *) context;
    
    if (value.format != homekit_format_bool)
    {
        printf("Invalid value for BlinkM ON characteristic: type=%d\n", value.format);
        return;
    }
    
    //~ printf(">>> BlinkM ON callback, value=%u\n", value.bool_value);
    
    bkm->on = value.bool_value;
    blinkm_update(bkm);
}

void blinkm_brightness_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context)
{
    blinkm_t *bkm = (blinkm_t *) context;
    
    if (value.format != homekit_format_int)
    {
        printf("Invalid value for BlinkM BRIGHTNESS characteristic: type=%d\n", value.format);
        return;
    }
    
    //~ printf(">>> BlinkM BRIGHTNESS callback, value=%u\n", value.int_value);
    
    bkm->brightness = value.int_value;
    blinkm_update(bkm);
}

void blinkm_hue_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context)
{
    blinkm_t *bkm = (blinkm_t *) context;
    
    if (value.format != homekit_format_float)
    {
        printf("Invalid value for BlinkM HUE characteristic: type=%d\n", value.format);
        return;
    }
    
    //~ printf(">>> BlinkM HUE callback, value=%f\n", value.float_value);
    
    bkm->hue = (uint8_t) ((value.float_value / 360.0f) * 255);
    blinkm_update(bkm);
}

void blinkm_saturation_cb(homekit_characteristic_t *ch, homekit_value_t value, void *context)
{
    blinkm_t *bkm = (blinkm_t *) context;
    
    if (value.format != homekit_format_float)
    {
        printf("Invalid value for BlinkM SATURATION characteristic: type=%d\n", value.format);
        return;
    }
    
    //~ printf(">>> BlinkM SATURATION callback, value=%f\n", value.float_value);
    
    bkm->saturation = (uint8_t) ((value.float_value / 100.0f) * 255);
    blinkm_update(bkm);
}

void blinkm_update(blinkm_t *bkm)
{
    uint8_t data[4] = {'h', 0, 0, 0};
    
    if (bkm->on)
    {
        data[1] = bkm->hue;
        data[2] = bkm->saturation;
        data[3] = round(255 * (bkm->brightness/100.0f));
        
        printf(">> Setting %u to: ON=%u, H=%.1f/%u, S=%.1f/%u, B=%.1f/%u\n",
               bkm->address, bkm->on,
               bkm->hue, data[1],
               bkm->saturation, data[2],
               bkm->brightness, data[3]);
    }
    
    int err = i2c_slave_write(BLINKM_BUS, bkm->address, NULL, data, 4);
    if (err != 0)
    {
        // do something with error
        printf("ERROR WHILE TRANSMITTING: %d\n", err);
    }
}
