#ifndef LED_EMOTION_SAD_H
#define LED_EMOTION_SAD_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include "utils/color.h"

#include "led/effects/explosion.h"
#include "led/effects/fade.h"
#include "led/effects/drop.h"
#include "led/effects/glow.h"
#include "led/effects/clouds.h"

// Struct to encapsulate parameters for an effect call
typedef enum {
    SAD_EFFECT_STAGE_BUILD,
    SAD_EFFECT_STAGE_DROP,
    SAD_EFFECT_STAGE_SPLASH,
    SAD_EFFECT_STAGE_DONE
} sad_effect_t;

typedef struct {
    explosion_params_t explosion;
    fade_params_t fade;
    drop_params_t drop;
    glow_params_t glow;
    cloud_params_t cloud;

    uint16_t pixel_count;
    sad_effect_t stage;
} sad_params_t;

sad_params_t sad_params = {
    .explosion = {
        .duration = 3000.0f,
        .current = 0,

        .position_start = 0.0f,
        .air_density = 1.225f,
        .energy = 1000000.0f,
        .speed = 0.0f,
        .gravity = 150.0f,

        .smooth = 17.0f,
        .scale = 3.0f,
    },

    .fade = {
        .duration = 3000,
        .current = 0
    },
    
    .drop = {
        .duration = 3000,
        .current = 0,
        
        .position_start = 3.0f,
        .acceleration = 0.0f,  // Set later to half pixel count.
        .speed = 0.0f,
        
        .radius = 3.0f,
        .wrap_type = WRAP_CLAMP
    },

    .glow = {
        .duration = 3000,
        .current = 0,

        .position = 3.0f,
        .max_radius = 3.0f
    },

    .cloud = {
        .step_size = 20000,
        .min_intensity = 125,
        .max_intensity = 255,
        .speed = 0.5f,
        .t1 = 0
    }
};

void sad_reset(
    const led_renderer_t* renderer, 
    const synced_timer_t* timer, 
    void* params
) {
    sad_params_t* sad = (sad_params_t*)params;
    sad->stage = SAD_EFFECT_STAGE_BUILD;
    sad->explosion.current = 0;
    sad->explosion.position_start = sad->pixel_count - 3;
    sad->fade.current = 0;
    sad->drop.current = 0;
    sad->drop.position = 0;
    sad->glow.current = 0;
}

void sad_init(
    const led_renderer_t* renderer, 
    const synced_timer_t* timer, 
    void* params
){
    sad_params_t* sad = (sad_params_t*)params;
    uint16_t pixel_count = renderer->buffer.length;
    sad->pixel_count = pixel_count;
    sad->drop.pixel_count = pixel_count;
    sad->drop.acceleration = pixel_count * 2;
    sad_reset(renderer, timer, params);
}

void sad_free(
    const led_renderer_t* renderer, 
    const synced_timer_t* timer, 
    void* params
) {
    // Do nothing
}

led_effect_state_t sad_get_state(const void* params) {
    const sad_params_t* sad = (const sad_params_t*)params;
    if(sad->stage == SAD_EFFECT_STAGE_DONE) {
        return LED_EFFECT_COMPLETE;
    }
    return LED_EFFECT_IN_PROGRESS;
}

void sad_update(
    const led_renderer_t* renderer,
    const synced_timer_t* timer,
    void* params
) {
    sad_params_t* sad = (sad_params_t*)params;
    if(sad->glow.current < sad->glow.duration) {
        sad->stage = SAD_EFFECT_STAGE_BUILD; 
    } else if(sad->drop.position < sad->pixel_count) {
        sad->stage = SAD_EFFECT_STAGE_DROP;
    } else if(sad->explosion.current < sad->explosion.duration) {
        sad->stage = SAD_EFFECT_STAGE_SPLASH;
    } else {
        sad->stage = SAD_EFFECT_STAGE_DONE;
    }

    cloud_before_render(timer, (void*)&sad->cloud);
    switch (sad->stage) {
        case SAD_EFFECT_STAGE_BUILD: 
            glow_before_render(timer, (void*)&sad->glow);
            break;
        case SAD_EFFECT_STAGE_DROP:
            drop_before_render(timer, (void*)&sad->drop);
            break;
        case SAD_EFFECT_STAGE_SPLASH:
            explosion_before_render(timer, (void*)&sad->explosion);
            fade_before_render(timer, (void*)&sad->fade);
            break;
        case SAD_EFFECT_STAGE_DONE:
            sad_reset(renderer, timer, params);
            break; 
        default:
            break;
    }
}

void sad_render(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
) {
    const sad_params_t* sad = (const sad_params_t*)params;
    uint8_t intensity = 0;
    uint8_t c = cloud_render(position, (void*)&sad->cloud);

    switch (sad->stage) {
        case SAD_EFFECT_STAGE_BUILD: 
            intensity = glow_render(position, (void*)&sad->glow);
            break;
        case SAD_EFFECT_STAGE_DROP:
            intensity = drop_render(position, (void*)&sad->drop);
            break;
        case SAD_EFFECT_STAGE_SPLASH:
            uint8_t e = explosion_render(position, (void*)&sad->explosion);
            uint8_t f = fade_render(position, (void*)&sad->fade);
            intensity = fmul8(e, f);
            intensity = fmul8(intensity, c);
            break;
        case SAD_EFFECT_STAGE_DONE:
            break; 
        default:
            break;
    }
    
    color->r = sclamp8((uint8_t)fmul8(c, 30) + fmul8(intensity, 10));
    color->g = sclamp8((uint8_t)fmul8(c, 10) + fmul8(intensity, 30));
    color->b = sclamp8((uint8_t)fmul8(c, 40) + fmul8(intensity, 240));
}

led_effect_color_t sad_effect = {
    .base = {
        .init = sad_init,
        .update = sad_update,
        .reset = sad_reset,
        .free = sad_free,
        .get_state = sad_get_state,
        .params = &sad_params
    },
    .render = sad_render
};

#endif // LED_EMOTION_SAD_H
