#include <stdlib.h>
#include <string.h>

#include "led/buffer.h"
#include "utils/integer_math/lib8tion.h"

void led_buffer_init(led_buffer_t* buf, int length) {
    buf->data = (rgb_color_t*)malloc(sizeof(rgb_color_t) * length);
    buf->length = length;
}

void led_buffer_clear(led_buffer_t* buf) {
    memset(buf->data, 0, sizeof(rgb_color_t) * buf->length);
}

void led_buffer_free(led_buffer_t* buf) {
    free(buf->data);
    buf->data = NULL;
    buf->length = 0;
}

void led_buffer_push(const led_buffer_t* buf, const led_strip_handle_t led_strip_handle) {
    for (int i = 0; i < buf->length; ++i) {
        rgb_color_t c = buf->data[i];
        led_strip_set_pixel(led_strip_handle, i, c.r, c.g, c.b);
    }
    led_strip_refresh(led_strip_handle);
}

void led_buffer_join(
    led_buffer_t* buffer, 
    const led_buffer_t* other, 
    const buffer_op_t operation
) {
    for (int i = 0; i < other->length; ++i) {
        led_buffer_apply(buffer, i, &other->data[i], operation);
    }
}

void led_buffer_apply(
    led_buffer_t* buffer, 
    const uint16_t position, 
    const rgb_color_t* color, 
    const buffer_op_t operation
) {
    rgb_color_t* target = &buffer->data[position];
    switch(operation) {
        case BUFFER_ADD: 
            target->r = qadd8(target->r, color->r);
            target->g = qadd8(target->g, color->g);
            target->b = qadd8(target->b, color->b);
            break;
        case BUFFER_SUBTRACT: 
            target->r = qsub8(target->r, color->r);
            target->g = qsub8(target->g, color->g);
            target->b = qsub8(target->b, color->b);
            break;
        case BUFFER_MULTIPLY: 
            target->r = fmul8(color->r, target->r);
            target->g = fmul8(color->g, target->g);
            target->b = fmul8(color->b, target->b);
            break;
        case BUFFER_MAX: 
            target->r = max8(target->r, color->r);
            target->g = max8(target->g, color->g);
            target->b = max8(target->b, color->b);
            break;
        case BUFFER_MIN: 
            target->r = min8(target->r, color->r);
            target->g = min8(target->g, color->g);
            target->b = min8(target->b, color->b);
            break;
    }
}

