#ifndef LED_EFFECT_LIGHTNING_H
#define LED_EFFECT_LIGHTNING_H

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "utils/integer_math/lib8simplex.h"
#include "utils/integer_math/lib8perlin.h"
#include "utils/integer_math/lib8tion.h"
#include "utils/synced_timer.h"
#include "utils/color.h"


typedef struct {
    int16_t* pixels;
    int16_t pixel_count;
    int32_t bolt_timer;
    uint16_t x;
    uint32_t delay_factor;
    uint32_t reset_delay_factor;
    float lightning_fade;
} lightning_params_t;


void lightning_before_render(
    const synced_timer_t* timer,
    void* params
) {
    lightning_params_t* lightning_params = (lightning_params_t*)params;
    uint16_t pixel_count = lightning_params->pixel_count;
    for (uint16_t i = 0; i < pixel_count; i++) {
        lightning_params->pixels[i] *= lightning_params->lightning_fade;
    }

    lightning_params->bolt_timer -= timer->delta_time;
    uint32_t bolt_min = floor(pixel_count / 15);
    uint32_t bolt_max = ceil(pixel_count / 6);

    if (lightning_params->bolt_timer <= 0) {
        uint16_t bolt_size = rand() % (bolt_max - bolt_min + 1) + bolt_min;
        while (bolt_size-- > 0 && lightning_params->x < pixel_count) {
            lightning_params->pixels[lightning_params->x++] = 250;
        }

        int min_delay = lightning_params->delay_factor / 5;
        lightning_params->bolt_timer = rand() % (lightning_params->delay_factor - min_delay + 1) + min_delay;

        if (lightning_params->x >= pixel_count) {
            lightning_params->x = 0;
            min_delay = lightning_params->reset_delay_factor / 3;
            lightning_params->bolt_timer = rand() % (lightning_params->reset_delay_factor - min_delay + 1) + min_delay;
        }
    }
}

uint8_t lightning_render(    
    const uint16_t position,
    const void* params
) {
    lightning_params_t* lightning_params = (lightning_params_t*)params;
    return lightning_params->pixels[position];
}

#endif // LED_EFFECT_LIGHTNING_H