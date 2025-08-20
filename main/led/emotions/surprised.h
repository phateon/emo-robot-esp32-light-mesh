#ifndef LED_EFFECT_SURPRISED_H
#define LED_EFFECT_SURPRISED_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include "utils/color.h"

#include "led/effects/clouds.h"
#include "led/effects/chase.h"


// Struct to encapsulate parameters for an effect call
typedef struct {
    chase_params_t chase_params;
    cloud_params_t cloud_params;
} surprised_params_t;

surprised_params_t surprised_params = {
    .chase_params = {
        .position = 100,
        .pixel_count = 0,
        .speed = 10.5,
        .width = 9
    },
    .cloud_params = {
        .step_size = 10000,
        .min_intensity = 100,
        .max_intensity = 255,
        .speed = 4.0,
        .t1 = 0
    }
};

void surprised_reset(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    surprised_params_t* surprised = (surprised_params_t*)params;
    uint16_t pixel_count = renderer->buffer.length;
    surprised->chase_params.pixel_count = pixel_count;
}

void surprised_init(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // surprised_params_t* surprised = (surprised_params_t*)params;
    surprised_reset(renderer, timer, params);
}

void surprised_free(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // Do nothing
}

void surprised_update(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    surprised_params_t* surprised = (surprised_params_t*)params;
    chase_before_render(timer, (void*)&surprised->chase_params);
    cloud_before_render(timer, (void*)&surprised->cloud_params);
}

led_effect_state_t surprised_get_state(const void* params) {
    // surprised_params_t* surprised = (surprised_params_t*)params;
    return LED_EFFECT_IN_PROGRESS;
}

void surprised_render(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
) {
    //const surprised_params_t* surprised_params = (const surprised_params_t*)params;
    
    color->r = 255;
    color->g = 255;
    color->b = 0;
}

led_effect_color_t surprised_effect = {
    .base = {
        .init = surprised_init,
        .update = surprised_update,
        .reset = surprised_reset,
        .free = surprised_free,
        .get_state = surprised_get_state,
        .params = &surprised_params
    },
    .render = surprised_render
};

#endif // LED_EFFECT_HAPPY_H