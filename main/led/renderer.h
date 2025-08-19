#ifndef LED_RENDERER_LISTS_H
#define LED_RENDERER_LISTS_H

#include <stdint.h>
#include "led_strip.h"

#include "led/effect.h"
#include "led/buffer.h"

#include "utils/color.h"
#include "utils/synced_timer.h"

// Renderer definition.
typedef struct led_renderer_t {
    led_buffer_t buffer;
    led_strip_config_t strip_config;
    led_strip_handle_t strip_handle;
    led_strip_rmt_config_t rmt_config;
    led_effect_t* current_effect;
    led_effect_t* next_effect;
} led_renderer_t;


// Led render system API
void led_render_init(led_renderer_t* renderer);
void led_render_free(led_renderer_t* renderer);

// Effect functions
bool led_renderer_add_effect(
    led_renderer_t* renderer, 
    const char *key, 
    led_effect_t* effect
);

bool led_renderer_remove_effect(
    led_renderer_t* renderer, 
    const char *key
);

led_effect_t* led_renderer_find_effect(
    led_renderer_t* renderer, 
    const char *key
);

map_t* led_renderer_get_effect_map(led_renderer_t* renderer);
void led_renderer_set_next_effect(led_renderer_t* renderer, const char *key);

// Render functions
void led_render_start(led_renderer_t* renderer);
void led_render_clear(led_renderer_t* renderer);
void led_render(led_renderer_t* renderer, synced_timer_t* timer);
void led_render_finish(led_renderer_t* renderer);


#endif //  LED_EFFECT_LISTS_H
