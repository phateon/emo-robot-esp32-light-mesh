#ifndef LED_EFFECT_CHASE_H
#define LED_EFFECT_CHASE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "led_strip.h"

void chase_led(
    int64_t time, 
    const led_strip_handle_t led_strip_handle, 
    const led_strip_config_t* strip_config, 
    const float red,
    const float green, 
    const float blue)
{
    static float s_chase_pos = 0.0f;
    const float speed = 0.1f; // Speed of the chase effect
    const int led_strip_length = strip_config->max_leds; // Define the length of the LED strip

    s_chase_pos += speed * (time / 1000.0f); // Update position based on time
    if (s_chase_pos >= led_strip_length) {
        s_chase_pos -= led_strip_length; // Wrap around
    }

    // Update the LED strip with the chase effect
    // led_strip_clear(led_strip_handle);
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    const float blend_width = 3.0f;
    for (int i = 0; i < led_strip_length; ++i) {
        float dist = fabsf(i - s_chase_pos);
        if (dist > led_strip_length / 2) {
            dist = led_strip_length - dist;
        }
        float intensity = expf(-0.5f * (dist * dist) / (blend_width * blend_width));
        r = (uint8_t)(intensity * red * 255.0f);
        g = (uint8_t)(intensity * green * 255.0f);
        b = (uint8_t)(intensity * blue * 255.0f);
        led_strip_set_pixel(led_strip_handle, i, r, g, b);
    }
    led_strip_refresh(led_strip_handle);
}

void chase_led_red(
    int64_t time, 
    const led_strip_handle_t led_strip_handle, 
    const led_strip_config_t* strip_config)
{
    chase_led(time, led_strip_handle, strip_config, 1.0f, 0.0f, 0.0f);
}

void chase_led_green(
    int64_t time, 
    const led_strip_handle_t led_strip_handle, 
    const led_strip_config_t* strip_config)
{
    chase_led(time, led_strip_handle, strip_config, 0.0f, 1.0f, 0.0f);
}

void chase_led_blue(
    int64_t time, 
    const led_strip_handle_t led_strip_handle, 
    const led_strip_config_t* strip_config)
{
    chase_led(time, led_strip_handle, strip_config, 0.0f, 0.0f, 1.0f);
}
#endif // LED_EFFECT_CHASE_H