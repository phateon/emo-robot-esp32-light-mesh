#ifndef LED_EFFECT_CHASE_H
#define LED_EFFECT_CHASE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include "utils/color.h"
#include "utils/synced_timer.h"
#include "utils/integer_math/lib8tion.h"

#include "led_strip.h"
#include "led/effect.h"
#include "led/buffer.h"


// Struct to encapsulate parameters for an effect call
typedef struct {
    float speed;              // Speed of the effect in pixel per second
    float width;              // Width of the effect in pixels
    float sqr_width;
    float position;           // Current position of the effect
    uint16_t pixel_count;
    uint16_t half_length;
} chase_params_t;


void chase_before_render (
    const synced_timer_t* timer,
    void* params
) {
    chase_params_t* chase_params = (chase_params_t*)params;
    const int led_strip_length = chase_params->pixel_count;
    chase_params->position += (timer->delta_time / 1000.0) * chase_params->speed;
    if (chase_params->position >= led_strip_length) {
        chase_params->position -= led_strip_length; // Wrap around
    }

    // Update the LED strip with the chase effect
    chase_params->sqr_width = (chase_params->width * chase_params->width);
    chase_params->half_length = led_strip_length / 2;
}

uint8_t chase_render(
    const uint16_t position,
    const void* params
) {
    const chase_params_t* chase_params = (const chase_params_t*)params;
    float intensity = 0;
    float dist = fabsf(position - chase_params->position);
    if (dist > chase_params->half_length) {
        dist = chase_params->pixel_count - dist;
    }
    intensity = expf(-0.5f * (dist * dist) / chase_params->sqr_width);
    return sclamp8(intensity * 255);
}

#endif // LED_EFFECT_CHASE_H
