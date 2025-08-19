#ifndef LED_BUFFER_H
#define LED_BUFFER_H

#include <stdlib.h>
#include <string.h>
#include "led_strip.h"
#include "utils/color.h"

// Enum for common buffer operations
typedef enum {
    BUFFER_ADD,
    BUFFER_SUBTRACT,
    BUFFER_MULTIPLY,
    BUFFER_MAX,
    BUFFER_MIN,
} buffer_op_t;

// Buffer struct
typedef struct {
    rgb_color_t* data;
    int length;
} led_buffer_t;

// Initialize buffer (allocate memory)
void led_buffer_init(led_buffer_t* buf, int length);

// Reset buffer to all zeros
void led_buffer_clear(led_buffer_t* buf);

// Free buffer memory
void led_buffer_free(led_buffer_t* buf);

// Push buffer content to LED strip
void led_buffer_push(const led_buffer_t* buf, const led_strip_handle_t led_strip_handle);

// Joining two buffers
void led_buffer_join(
    led_buffer_t* buffer, 
    const led_buffer_t* other, 
    const buffer_op_t operation
);

// Apply a buffer operation at a position
void led_buffer_apply(
    led_buffer_t* buffer, 
    const uint16_t position, 
    const rgb_color_t* color, 
    const buffer_op_t operation
);

#endif // RGB_BUFFER_H
