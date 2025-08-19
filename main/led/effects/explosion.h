#ifndef LED_EFFECT_EXPLOSION_H
#define LED_EFFECT_EXPLOSION_H

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
 
    float energy; 
    float air_density;
    float smooth;
    float scale;

    float position_start;
    float position;
    float gravity;
    float speed;

    float radius;
} explosion_params_t;


void explosion_before_render (
    const synced_timer_t* timer,
    void* params
) {
    explosion_params_t* explosion = (explosion_params_t*)params;

    if(explosion->duration - explosion->current > timer->delta_time) {
        explosion->current += timer->delta_time;
    } else {
        explosion->current = explosion->duration;
    }

    if(explosion->air_density<=0) {
        ESP_LOGW(TAG, "Air density is 0");
        explosion->air_density = .001;
    }

    if(explosion->smooth<=0) {
        ESP_LOGW(TAG, "Smoothing factor is 0");
        explosion->smooth = 1;
    }

    // Update the LED strip with the explosion effect
    float t = (float)explosion->current / explosion->duration;
    float p = explosion->position_start;
    float v = explosion->speed;
    float a = explosion->gravity;
    explosion->position = p + v * t + 0.5 * a * (t * t);

    float e = pow((explosion->energy / explosion->air_density), 0.2);
    float d = pow(t * explosion->smooth * explosion->scale, 0.4);
    explosion->radius = e * d;
}

uint8_t explosion_render (
    const uint16_t x,
    const void* params
) {
    const explosion_params_t* explosion = (const explosion_params_t*)params;
    float p = fabsf((x - explosion->position) / explosion->smooth);
    float r = explosion->radius / explosion->smooth;
    float c = sclamp8((1.0 - fminf(fabsf(p - r), 1.0)) * 255);
    return c;
}

#endif // LED_EFFECT_explosion_H
