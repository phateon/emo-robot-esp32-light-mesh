#include "esp_log.h"
#include "esp_err.h"
#include "led_strip.h"

#include "utils/map.h"
#include "utils/color.h"
#include "utils/synced_timer.h"
#include "utils/integer_math/lib8tion.h"

#include "led/buffer.h"
#include "led/effect.h"
#include "led/renderer.h"

// Fixed parameters
#define MAX_LED_EFFECTS 12
static const char *TAG = "renderer";

// Map for effect management using string map
static map_entry_t emotions[MAX_LED_EFFECTS];
static map_t effect_map = {
    .entries = emotions,
    .size = 0,
    .capacity = MAX_LED_EFFECTS
};

// Construction functions
void led_render_init(led_renderer_t* renderer) {
    renderer->strip_config.max_leds = 150;
    renderer->strip_config.strip_gpio_num = 6;
    renderer->strip_config.led_pixel_format = LED_PIXEL_FORMAT_GRB;
    renderer->strip_config.led_model = LED_MODEL_WS2812;
    renderer->rmt_config.resolution_hz = 10 * 1000 * 1000; // 10MHz
    renderer->rmt_config.flags.with_dma = false;

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &renderer->strip_config, 
            &renderer->rmt_config, 
            &renderer->strip_handle
        )
    );
    led_buffer_init(&renderer->buffer, renderer->strip_config.max_leds);
    led_strip_clear(renderer->strip_handle);
}

bool led_renderer_add_effect(led_renderer_t* renderer, const char *key, led_effect_t* effect) {
    if (map_set(&effect_map, (void *)key, (void *)effect) != 0) {
        ESP_LOGW(TAG, "Effect '%s' already exists or map is full", key);
        return false;
    }
    effect->name = key;
    effect->init(renderer, effect->effect_params);
    return true;
}

void led_render_free(led_renderer_t* renderer) {
    led_buffer_free(&renderer->buffer);
    led_strip_del(renderer->strip_handle);
}

// Effect functions
bool led_renderer_remove_effect(led_renderer_t* renderer, const char *key) {
    if (map_remove(&effect_map, (void *)key) != 0) {
        ESP_LOGW(TAG, "Effect '%s' not found", key);
        return false;
    }
    return true;
}

led_effect_t* led_renderer_find_effect(led_renderer_t* renderer, const char *key) {
    led_effect_t* effect = (led_effect_t*)map_get(&effect_map, (void *)key);
    if (effect == NULL)  {
        ESP_LOGW(TAG, "Effect <%s> not found.", key);
        return NULL;
    }
    return effect;
}

map_t* led_renderer_get_effect_map(led_renderer_t* renderer) {
    return &effect_map;
}

void led_renderer_set_next_effect(led_renderer_t* renderer, const char *key) {
    led_effect_t* effect = led_renderer_find_effect(renderer, key);
    if (effect==NULL) {
        return;
    }

    led_effect_t* current = renderer->current_effect;
    led_effect_t* next = renderer->next_effect;

    if (current == NULL) {
        ESP_LOGW(TAG, "Current set as <%s> from NULL", key);
        renderer->current_effect = effect;
        return;
    }
    
    if (next == NULL) {
        if(strcmp(current->name, key) == 0){
            ESP_LOGW(TAG, "Mood:<%s> is current <%s>.", key, current->name);
            return;
        }

        ESP_LOGW(TAG, "Next set as <%s> from NULL", key);
        renderer->next_effect = effect;
        return;
    }

    if(strcmp(next->name, key) == 0){
        ESP_LOGW(TAG, "Mood:<%s> is next <%s>", key, next->name);
        return;
    }

    ESP_LOGW(TAG, "Next set as <%s> from <%s>", key, next->name);
    renderer->next_effect = effect;
}


// Render functions
void led_render_start(led_renderer_t* renderer) {
    led_buffer_clear(&renderer->buffer);
}


void led_renderer_clear(led_renderer_t* renderer) {
    led_buffer_clear(&renderer->buffer);
}


void led_render(led_renderer_t* renderer, synced_timer_t* timer) {
    if(renderer->current_effect == NULL) {
        ESP_LOGW(TAG, "No current effect selected.");
        return;
    }

    rgb_color_t current_color = {0};
    rgb_color_t next_color = {0};
    uint8_t blend = 0;

    led_effect_t* current = renderer->current_effect;
    led_effect_t* next = renderer->next_effect;
    current->pre_render_effect(timer, current->effect_params);
    if(next != NULL) {
        current->pre_render_transition(timer, current->transition_params);
        next->pre_render_effect(timer, next->effect_params);
    }

    for(uint16_t pos = 0; pos < renderer->buffer.length; pos++) {
        current->render_effect(pos, current->effect_params, &current_color);
        if(next != NULL) {
            blend = current->render_transition(pos, current->transition_params);
            next->render_effect(pos, next->effect_params, &next_color);
        }
        current_color.r = blend8(current_color.r, next_color.r, blend);
        current_color.g = blend8(current_color.g, next_color.g, blend);
        current_color.b = blend8(current_color.b, next_color.b, blend);
        led_buffer_apply(&renderer->buffer, pos, &current_color, BUFFER_ADD);
    }

    transition_state_t state = current->get_transition_state(
        timer, 
        current->transition_params
    );
    
    if(state == TRANSITION_COMPLETE) {
        if(next != NULL) {
            renderer->current_effect = next;
            renderer->next_effect = NULL;
            current->reset_transition(current->transition_params);
        }
    }
}

void led_render_finish(led_renderer_t* renderer) {
    led_buffer_push(&renderer->buffer, renderer->strip_handle);
}
