#ifndef LED_EFFECT_HAPPY_H
#define LED_EFFECT_HAPPY_H

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
} happy_params_t;

happy_params_t happy_params = {
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


void happy_init(led_renderer_t* renderer, void* params){
    happy_params_t* happy = (happy_params_t*)params;
    uint16_t pixel_count = renderer->buffer.length;
    happy->chase_params.pixel_count = pixel_count;
}


void happy_free(led_renderer_t* renderer, void* params) {}


void happy_before_render(
    const synced_timer_t* timer,
    void* params
) {
    happy_params_t* happy = (happy_params_t*)params;
    chase_before_render(timer, (void*)&happy->chase_params);
    cloud_before_render(timer, (void*)&happy->cloud_params);
}


void happy_render(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
) {
    //const happy_params_t* happy_params = (const happy_params_t*)params;
    
    color->r = 0;
    color->g = 0;
    color->b = 255;
}

led_effect_t happy_effect = {
    .init = happy_init,

    .pre_render_effect = happy_before_render,
    .render_effect = happy_render,
    .effect_params = &happy_params,

    .pre_render_transition = smooth_before_render,
    .get_transition_state = smooth_transition_state,
    .render_transition = smooth_render,
    .reset_transition = smooth_reset,
    .transition_params = &smooth_params
};

#endif // LED_EFFECT_ANGER_H