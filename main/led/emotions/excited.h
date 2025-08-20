#ifndef LED_EFFECT_EXCITED_H
#define LED_EFFECT_EXCITED_H

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
} excited_params_t;

excited_params_t excited_params = {
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

void excited_reset(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    excited_params_t* excited = (excited_params_t*)params;
    uint16_t pixel_count = renderer->buffer.length;
    excited->chase_params.pixel_count = pixel_count;
}

void excited_init(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // excited_params_t* excited = (excited_params_t*)params;
    excited_reset(renderer, timer, params);
}

void excited_free(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // Do nothing
}

void excited_update(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    excited_params_t* excited = (excited_params_t*)params;
    chase_before_render(timer, (void*)&excited->chase_params);
    cloud_before_render(timer, (void*)&excited->cloud_params);
}

led_effect_state_t excited_get_state(const void* params) {
    // excited_params_t* excited = (excited_params_t*)params;
    return LED_EFFECT_IN_PROGRESS;
}

void excited_render(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
) {
    //const excited_params_t* excited_params = (const excited_params_t*)params;
    
    color->r = 0;
    color->g = 255;
    color->b = 255;
}

led_effect_color_t excited_effect = {
    .base = {
        .init = excited_init,
        .update = excited_update,
        .reset = excited_reset,
        .free = excited_free,
        .get_state = excited_get_state,
        .params = &excited_params
    },
    .render = excited_render
};

#endif // LED_EFFECT_EXCITED_H