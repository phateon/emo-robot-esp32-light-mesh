#ifndef LED_EFFECT_H
#define LED_EFFECT_H

#include <stdint.h>
#include "utils/color.h"
#include "utils/synced_timer.h"

typedef enum transition_state_t {
    TRANSITION_NOT_STARTED,
    TRANSITION_IN_PROGRESS,
    TRANSITION_COMPLETE
} transition_state_t;

typedef enum wrap_type_t {
    WRAP_LOOP,
    WRAP_CLAMP
} wrap_type_t;


typedef struct led_renderer_t led_renderer_t; 

// Initi all features of the effect
typedef void (*effect_init_t)(
    led_renderer_t* renderer,
    void* params
);

// Function pointer type for effect pre render function
typedef void (*effect_pre_render_t)(
    const synced_timer_t* timer, 
    void* params
);

// Function pointer type for effect render function
typedef void (*effect_reset_t)(
    const void* params
);

// Function pointer type for effect render function
typedef void (*effect_render_color_t)(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
);

// Function pointer type for effect render function
typedef uint8_t (*effect_render_intensity_t)(
    const uint16_t position,
    const void* params
);

// Function pointer type for effect render function
typedef transition_state_t (*get_transition_state_t)(
    const synced_timer_t* timer, 
    void* params
);

// Struct for an effect
typedef struct {
    effect_init_t init;

    effect_pre_render_t pre_render_effect;
    effect_render_color_t render_effect;
    effect_reset_t reset_effect;
    void* effect_params;

    effect_pre_render_t pre_render_transition;
    effect_render_intensity_t render_transition;
    effect_reset_t reset_transition;
    get_transition_state_t get_transition_state;
    void* transition_params;

    const char* name;
} led_effect_t;

#endif //  LED_EFFECT_H
