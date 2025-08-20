#ifndef LED_EFFECT_RELAXED_H
#define LED_EFFECT_RELAXED_H

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
} relaxed_params_t;

relaxed_params_t relaxed_params = {
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


void relaxed_reset(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    relaxed_params_t* relaxed = (relaxed_params_t*)params;
    uint16_t pixel_count = renderer->buffer.length;
    relaxed->chase_params.pixel_count = pixel_count;
}

void relaxed_init(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // relaxed_params_t* relaxed = (relaxed_params_t*)params;
    relaxed_reset(renderer, timer, params);
}

void relaxed_free(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // Do nothing
}

void relaxed_update(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    relaxed_params_t* relaxed = (relaxed_params_t*)params;
    chase_before_render(timer, (void*)&relaxed->chase_params);
    cloud_before_render(timer, (void*)&relaxed->cloud_params);
}

led_effect_state_t relaxed_get_state(const void* params) {
    // relaxed_params_t* relaxed = (relaxed_params_t*)params;
    return LED_EFFECT_IN_PROGRESS;
} 

void relaxed_render(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
) {
    const relaxed_params_t* relaxed_params = (const relaxed_params_t*)params;

    uint8_t cloud = cloud_render(position, (void*)&relaxed_params->cloud_params);
    uint8_t chase = chase_render(position, (void*)&relaxed_params->chase_params);
    
    color->r = sclamp8((uint16_t)fmul8(cloud, 50) + fmul8(chase, 250));
    color->g = sclamp8((uint16_t)fmul8(cloud, 100) + fmul8(chase, 100));
    color->b = sclamp8((uint16_t)fmul8(cloud, 200) + fmul8(chase, 50));
}

led_effect_color_t relaxed_effect = {
    .base = {
        .init = relaxed_init,
        .update = relaxed_update,
        .reset = relaxed_reset,
        .free = relaxed_free,
        .get_state = relaxed_get_state,
        .params = &relaxed_params
    },
    .render = relaxed_render
};

#endif // LED_EFFECT_ANGER_H
