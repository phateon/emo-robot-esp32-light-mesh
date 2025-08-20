#ifndef LED_EFFECT_ANGER_H
#define LED_EFFECT_ANGER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <esp_log.h>

#include "utils/color.h"

#include "led/effects/clouds.h"
#include "led/effects/lightning.h"


// Struct to encapsulate parameters for an effect call
typedef struct {
    lightning_params_t lightning_params;
    cloud_params_t cloud_params;
} anger_params_t;


anger_params_t anger_params = {
    .lightning_params = {
        .bolt_timer = 0,
        .x = 0,
        .delay_factor = 200,
        .reset_delay_factor = 2000,
        .lightning_fade = 0.95,
    },
    .cloud_params = {
        .step_size = 10000,
        .min_intensity = 175,
        .max_intensity = 255,
        .speed = 0.0,
        .t1 = 0
    }
};

void anger_reset(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // anger_params_t* anger = (anger_params_t*)params;
}

void anger_init(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    uint16_t pixel_count = renderer->buffer.length;
    size_t pixel_size = sizeof(int16_t) * pixel_count;
    anger_params_t* anger = (anger_params_t*)params;
    anger->lightning_params.pixels = malloc(pixel_size),
    anger->lightning_params.pixel_count = pixel_count,
    memset(anger->lightning_params.pixels, 0, pixel_size);
}

void anger_free(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    anger_params_t* anger = (anger_params_t*)params;
    free(anger->lightning_params.pixels);
}

void anger_update(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    anger_params_t* anger_params = (anger_params_t*)params;
    anger_params->cloud_params.speed += ((rand() % 1000) - 500.0) / 5000.0;
    if(fabsf(anger_params->cloud_params.speed) > 2) {
        anger_params->cloud_params.speed = 0;
        anger_params->cloud_params.t1 = 0;
    }
 
    cloud_before_render(timer, (void*)&anger_params->cloud_params);
    lightning_before_render(timer, (void*)&anger_params->lightning_params);
}

led_effect_state_t anger_get_state(const void* params) {
    // anger_params_t* anger = (anger_params_t*)params;
    return LED_EFFECT_IN_PROGRESS;
}

void anger_render(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
) {
    const anger_params_t* anger_params = (const anger_params_t*)params;
    uint8_t cloud = cloud_render(position, (void*)&anger_params->cloud_params);
    uint8_t lightning = lightning_render(position, (void*)&anger_params->lightning_params);
    uint16_t r = fmul8(175, cloud) + fmul8(255, lightning);
    uint16_t g = fmul8(0, cloud) + fmul8(20, lightning);
    uint16_t b = fmul8(0, cloud) + fmul8(10, lightning);
    color->r = sclamp8(r);
    color->g = sclamp8(g);
    color->b = sclamp8(b);
}

led_effect_color_t anger_effect = {
    .base = {
        .init = anger_init,
        .update = anger_update,
        .reset = anger_reset,
        .free = anger_free,
        .get_state = anger_get_state,
        .params = &anger_params
    },
    .render = anger_render
};

#endif // LED_EFFECT_ANGER_H
