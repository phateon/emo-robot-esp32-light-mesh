#ifndef LED_EFFECT_LISTS_H
#define LED_EFFECT_LISTS_H

#include <stdint.h>
#include "led_strip.h"

// Function pointer type for LED effects
typedef void (*led_effect_func_t)(
    int64_t time, 
    const led_strip_handle_t led_strip, 
    const led_strip_config_t* strip_config);

void configure_led(uint16_t num_leds);

// Effect system API
bool led_effect_add(const char *key, led_effect_func_t effect);
bool led_effect_remove(const char *key);
void led_effect_run(int64_t time, const char *key);

#endif //  LED_EFFECT_LISTS_H
