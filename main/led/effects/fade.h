#ifndef LED_EFFECT_FADE_H
#define LED_EFFECT_FADE_H

#include <stdint.h>

#include <esp_log.h>

#include "utils/color.h"
#include "utils/synced_timer.h"
#include "utils/integer_math/lib8tion.h"

#include "led/effect.h"
#include "led/renderer.h"

 
typedef struct {
    uint16_t duration;      // Duration of the transition in milliseconds
    uint16_t current;       // Current time of transition in milliseconds
    uint8_t intensity;      // Intensity of the transition
} fade_params_t;


void fade_before_render(
    const synced_timer_t* timer, 
    void* params
){
    fade_params_t* fade = (fade_params_t*)params;
    if(fade->duration - fade->current > timer->delta_time) {
        fade->current += timer->delta_time;
    }
    else {
        fade->current = fade->duration;
    }

    fade->intensity = 255 - range_map16(
        fade->current,
        0, fade->duration,
        0, 255
    );

    // fade = smoothstep_8bit(fade, 0, 255);
}

uint8_t fade_render(const uint16_t position, const void* params){
    const fade_params_t* fade = (const fade_params_t*)params;
    return fade->intensity;
}

#endif 
