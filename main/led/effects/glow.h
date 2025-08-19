#ifndef LED_EFFECT_GLOW_H
#define LED_EFFECT_GLOW_H

#include <stdint.h>

#include <esp_log.h>

#include "utils/synced_timer.h"
#include "utils/integer_math/lib8tion.h"

#include "led/effect.h"
#include "led/renderer.h"

 
typedef struct {
    uint16_t duration;  // Duration of the transition in milliseconds
    uint16_t current;   // Current time of transition in milliseconds
    uint16_t position;  
    float max_radius;
    float radius;
    float sqr_width;
} glow_params_t;


void glow_before_render(
    const synced_timer_t* timer, 
    void* params
){
    glow_params_t* glow = (glow_params_t*)params;
    if(glow->duration - glow->current > timer->delta_time) {
        glow->current += timer->delta_time;
    }
    else {
        glow->current = glow->duration;
    }

    glow->radius = ((float)glow->current/glow->duration) * glow->max_radius;
    glow->sqr_width = (glow->radius * glow->radius);
}

uint8_t glow_render(const uint16_t position, const void* params){
    const glow_params_t* glow = (const glow_params_t*)params;
    float intensity = 0;
    float dist = position - glow->position;
    intensity = expf(-0.5f * (dist * dist) / glow->sqr_width);
    return sclamp8(intensity * 255);
}

#endif 
