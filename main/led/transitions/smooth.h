#ifndef LED_TRANS_SMOOTH_H
#define LED_TRANS_SMOOTH_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <esp_log.h>

#include "utils/color.h"
#include "utils/synced_timer.h"
#include "utils/integer_math/lib8tion.h"

#include "led_strip.h"
#include "led/effect.h"
#include "led/buffer.h"

 
typedef struct {
    uint16_t pixel_count;
    float pixel_scale;      // Scaling factor
    uint16_t duration;      // Duration of the transition in milliseconds
    uint16_t current;       // Current time of transition in milliseconds
    uint16_t position;      // center of the transition window
    uint8_t lower;
    uint8_t upper;
    uint16_t t1;
} smooth_params_t;

smooth_params_t smooth_params = {
    .pixel_count = 0,
    .pixel_scale = 0,
    .position = 0,
    .duration = 10000,
    .current = 0,
    .lower = 0,
    .upper = 255,
    .t1 = 0
};


void smooth_reset(led_renderer_t* renderer, const void* params) {
    smooth_params_t* smooth = (smooth_params_t*)params;
    smooth->current = 0;
    smooth->position = 0;
    smooth->t1 = 0;
}

void smooth_init(led_renderer_t* renderer, void* params) {
    smooth_params_t* smooth = (smooth_params_t*)params;
    smooth->pixel_count = renderer->buffer.length;
    smooth->pixel_scale = 255.0 / renderer->buffer.length;
}

transition_state_t smooth_transition_state(
    const synced_timer_t* timer, 
    void* params
) {
    smooth_params_t* smooth = (smooth_params_t*)params;
    if(smooth->duration - smooth->current > timer->delta_time) {
        return TRANSITION_IN_PROGRESS;
    } else {
        return TRANSITION_COMPLETE;
    }

    return TRANSITION_NOT_STARTED;
}

void smooth_before_render(
    const synced_timer_t* timer, 
    void* params
){
    smooth_params_t* smooth = (smooth_params_t*)params;
    if(smooth->duration - smooth->current > timer->delta_time) {
        smooth->current += timer->delta_time;
    }
    else {
        smooth->current = smooth->duration;
    }

    if(smooth->pixel_scale == 0) {
        ESP_LOGW("smooth.h", "pixel_scale is 0");
    }

    float t = (float)smooth->current / smooth->duration;
    smooth->position = t * 2 * 255 + 1;
}

uint8_t smooth_render(const uint16_t position, const void* params){
    const smooth_params_t* smooth = (const smooth_params_t*)params;
    uint8_t x = sclamp8(position * smooth->pixel_scale);
    uint8_t p = sclamp8((int)x + smooth->position - 255);
    uint8_t i = smoothstep_8bit(smooth->lower, smooth->upper,  p);
    return i;
}

#endif