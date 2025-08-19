#ifndef LED_EFFECT_FLIRTY_H
#define LED_EFFECT_FLIRTY_H

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
} flirty_params_t;

flirty_params_t flirty_params = {
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


void flirty_init(led_renderer_t* renderer, void* params){
    flirty_params_t* flirty = (flirty_params_t*)params;
    uint16_t pixel_count = renderer->buffer.length;
    flirty->chase_params.pixel_count = pixel_count;
}


void flirty_free(led_renderer_t* renderer, void* params) {}


void flirty_before_render(
    const synced_timer_t* timer,
    void* params
) {
    flirty_params_t* flirty = (flirty_params_t*)params;
    chase_before_render(timer, (void*)&flirty->chase_params);
    cloud_before_render(timer, (void*)&flirty->cloud_params);
}


void flirty_render(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
) {
    //const flirty_params_t* flirty_params = (const flirty_params_t*)params;
    
    color->r = 255;
    color->g = 0;
    color->b = 0;
}


led_effect_t flirty_effect = {
    .init = flirty_init,

    .pre_render_effect = flirty_before_render,
    .render_effect = flirty_render,
    .effect_params = &flirty_params,

    .pre_render_transition = smooth_before_render,
    .get_transition_state = smooth_transition_state,
    .render_transition = smooth_render,
    .reset_transition = smooth_reset,
    .transition_params = &smooth_params
};

#endif // LED_EFFECT_ANGER_H