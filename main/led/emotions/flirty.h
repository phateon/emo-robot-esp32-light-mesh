#ifndef LED_EFFECT_FLIRTY_H
#define LED_EFFECT_FLIRTY_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include "utils/color.h"

#include "led/effects/clouds.h"
#include "led/effects/chase.h"


#define FLIRTY_CHASE_COUNT 4

typedef struct {
    chase_params_t chase_params;
    rgb_color_t color;
} flirty_chase_t;

typedef struct {
    flirty_chase_t chases[FLIRTY_CHASE_COUNT];
    cloud_params_t cloud_params;
    uint16_t pixel_count;
} flirty_params_t;

flirty_params_t flirty_params;



void flirty_reset(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    flirty_params_t* flirty = (flirty_params_t*)params;
    flirty->pixel_count = renderer->buffer.length;
    for (int i = 0; i < FLIRTY_CHASE_COUNT; ++i) {
        flirty->chases[i].chase_params.pixel_count = flirty->pixel_count;
        flirty->chases[i].chase_params.position = rand() % flirty->pixel_count;
        flirty->chases[i].chase_params.speed = (rand() % 2000 - 1000) / 50.0f;
        flirty->chases[i].chase_params.width = 3 + rand() % 5; // 5 to 14
        flirty->chases[i].color.r = rand() % 240;
        flirty->chases[i].color.g = rand() % 50;
        flirty->chases[i].color.b = rand() % 20;
    }
    // Cloud params (unchanged)
    flirty->cloud_params.step_size = 10000;
    flirty->cloud_params.min_intensity = 100;
    flirty->cloud_params.max_intensity = 255;
    flirty->cloud_params.speed = 4.0;
    flirty->cloud_params.t1 = 0;
}


void flirty_init(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    flirty_reset(renderer, timer, params);
}

void flirty_free(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    // Do nothing
}


void flirty_update(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    flirty_params_t* flirty = (flirty_params_t*)params;
    for (int i = 0; i < FLIRTY_CHASE_COUNT; ++i) {
        chase_before_render(timer, (void*)&flirty->chases[i].chase_params);
    }
    //cloud_before_render(timer, (void*)&flirty->cloud_params);
}

led_effect_state_t flirty_get_state(const void* params) {
    // flirty_params_t* flirty = (flirty_params_t*)params;
    return LED_EFFECT_IN_PROGRESS;
}


void flirty_render(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
) {
    const flirty_params_t* flirty = (const flirty_params_t*)params;
    // Default: black
    color->r = 0;
    color->g = 0;
    color->b = 0;
    // Render all chases
    for (int i = 0; i < FLIRTY_CHASE_COUNT; ++i) {
        const chase_params_t* chase = &flirty->chases[i].chase_params;
        const uint8_t intensity = chase_render(position, (void*)chase);
        color->r = max8(color->r, fmul8(flirty->chases[i].color.r, intensity));
        color->g = max8(color->g, fmul8(flirty->chases[i].color.g, intensity));
        color->b = max8(color->b, fmul8(flirty->chases[i].color.b, intensity));
    }
    // Optionally, blend with cloud effect or add more logic here
}


led_effect_color_t flirty_effect = {
    .base = {
        .init = flirty_init,
        .update = flirty_update,
        .reset = flirty_reset,
        .free = flirty_free,
        .get_state = flirty_get_state,
        .params = &flirty_params
    },
    .render = flirty_render
};

#endif // LED_EFFECT_ANGER_H