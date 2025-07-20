#ifndef BOOT_BUTTON_H
#define BOOT_BUTTON_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*button_callback_t)(uint8_t gpio);

typedef struct {
    uint8_t gpio;
    uint8_t state;
    uint8_t last_state;
    uint32_t state_change_time;
    button_callback_t on_press;
    button_callback_t on_release;
} button_t;

typedef button_t* button_handler_t;

// Initialize the boot button (GPIO, interrupt, etc.)
button_handler_t button_init(uint8_t gpio);
esp_err_t button_free(button_handler_t button);

#endif // BOOT_BUTTON_H
