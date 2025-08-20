#ifndef LED_EFFECT_DROP_H
#define LED_EFFECT_DROP_H

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
    uint32_t current;
    uint32_t duration;

    uint16_t pixel_count;
    uint16_t half_length;

    float position_start;     // Starting position of the effect
    float radius;             // Radius of the effect
    float sqr_r;

    float acceleration;       // Acceleration of the effect
    float speed;              // Speed of the effect in pixel per second
    float position;           // Current position of the effect

    led_wrap_type_t wrap_type;
 } drop_params_t;


void drop_before_render (
    const synced_timer_t* timer,
    void* params
) {
    drop_params_t* drop = (drop_params_t*)params;
    if(drop->duration - drop->current > timer->delta_time) {
        drop->current += timer->delta_time;
    } else {
        drop->current = drop->duration;
     }

    float t = (float)drop->current / drop->duration;
    float p = drop->position_start;
    float v = drop->speed;
    float a = drop->acceleration;
    drop->position = p + v * t + 0.5 * a * (t * t);

    // Update the LED strip with the drop effect
    drop->sqr_r = (drop->radius + drop->speed) * (drop->radius + drop->speed);
    drop->half_length = drop->pixel_count / 2;
}

uint8_t drop_render(
    const uint16_t position,
    const void* params
) {
    const drop_params_t* drop = (const drop_params_t*)params;
    float intensity = 0;
    float dist = fabsf(position - drop->position);
    if (dist > drop->half_length && drop->wrap_type == WRAP_LOOP) {
        dist = drop->pixel_count - dist;
    }
    intensity = expf(-0.5f * (dist * dist) / drop->sqr_r);
    return sclamp8(intensity * 255);
}

#endif // LED_EFFECT_drop_H
