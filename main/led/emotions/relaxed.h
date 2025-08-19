#ifndef LED_EFFECT_RELAXED_H
#define LED_EFFECT_RELAXED_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include "utils/color.h"

#include "led/effects/clouds.h"
#include "led/effects/chase.h"

#include "led/transitions/smooth.h"

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


void relaxed_init(led_renderer_t* renderer, void* params){
    relaxed_params_t* relaxed = (relaxed_params_t*)params;
    uint16_t pixel_count = renderer->buffer.length;
    relaxed->chase_params.pixel_count = pixel_count;
}


void relaxed_free(led_renderer_t* renderer, void* params) {}


void relaxed_before_render(
    const synced_timer_t* timer,
    void* params
) {
    relaxed_params_t* relaxed = (relaxed_params_t*)params;
    chase_before_render(timer, (void*)&relaxed->chase_params);
    cloud_before_render(timer, (void*)&relaxed->cloud_params);
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


led_effect_t relaxed_effect = {
    .init = relaxed_init,

    .pre_render_effect = relaxed_before_render,
    .render_effect = relaxed_render,
    .effect_params = &relaxed_params,

    .pre_render_transition = smooth_before_render,
    .get_transition_state = smooth_transition_state,
    .render_transition = smooth_render,
    .reset_transition = smooth_reset,
    .transition_params = &smooth_params
};

#endif // LED_EFFECT_ANGER_H