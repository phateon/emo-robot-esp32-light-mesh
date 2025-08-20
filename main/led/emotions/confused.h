#ifndef LED_EFFECT_CONFUSED_H
#define LED_EFFECT_CONFUSED_H

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
} confused_params_t;

confused_params_t confused_params = {
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

void confused_reset(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    confused_params_t* confused = (confused_params_t*)params;
    uint16_t pixel_count = renderer->buffer.length;
    confused->chase_params.pixel_count = pixel_count;
}

void confused_init(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // confused_params_t* confused = (confused_params_t*)params;
    confused_reset(renderer, timer, params);
}

void confused_free(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // Do nothing
}

void confused_update(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    confused_params_t* confused = (confused_params_t*)params;
    chase_before_render(timer, (void*)&confused->chase_params);
    cloud_before_render(timer, (void*)&confused->cloud_params);
}

led_effect_state_t confused_get_state(const void* params) {
    // confused_params_t* confused = (confused_params_t*)params;
    return LED_EFFECT_IN_PROGRESS;
}

void confused_render(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
) {
    //const confused_params_t* confused_params = (const confused_params_t*)params;
    
    color->r = 255;
    color->g = 0;
    color->b = 255;
}

led_effect_color_t confused_effect = {
    .base = {
        .init = confused_init,
        .update = confused_update,
        .reset = confused_reset,
        .free = confused_free,
        .get_state = confused_get_state,
        .params = &confused_params
    },
    .render = confused_render
};

#endif // LED_EFFECT_CONFUSED_H