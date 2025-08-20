#ifndef LED_EFFECT_H
#define LED_EFFECT_H

#include <stdint.h>
#include "utils/color.h"
#include "utils/synced_timer.h"


typedef struct led_renderer_t led_renderer_t; 

// Type declarations
typedef enum led_effect_state_t {
    LED_EFFECT_NOT_STARTED,
    LED_EFFECT_IN_PROGRESS,
    LED_EFFECT_COMPLETE
} led_effect_state_t;

typedef enum led_wrap_type_t {
    WRAP_LOOP,
    WRAP_CLAMP
} led_wrap_type_t;

// Edit an effect
typedef void (*led_effect_change_t)(
    const led_renderer_t* renderer,
    const synced_timer_t* timer, 
    void* params
);

// Render color
typedef void (*led_render_color_t)(
    const uint16_t position,
    const void* params,
    rgb_color_t* color
);

// Render intensity value
typedef uint8_t (*led_render_intensity_t)(
    const uint16_t position,
    const void* params
);

// Get the current state
typedef led_effect_state_t (*get_led_effect_state_t)(const void* params);

// Struct for an effect
typedef struct {
    led_effect_change_t init;
    led_effect_change_t update;
    led_effect_change_t reset;
    led_effect_change_t free;
    get_led_effect_state_t get_state;
    void* params;
} led_effect_base_t;

// Render color effect
typedef struct {
    led_effect_base_t base;
    led_render_color_t render;
    const char* name;
} led_effect_color_t;

// Render intensity effect
typedef struct {
    led_effect_base_t base;
    led_render_intensity_t render;
    const char* name;
} led_effect_intensity_t;

#endif //  LED_EFFECT_H
