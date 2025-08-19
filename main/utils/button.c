#include <string.h>

#include "esp_timer.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

#include "utils/button.h"

static const char *TAG = "button";


static void button_check_task(void *arg) {
    ESP_LOGI(TAG, "Starting button handler");
    button_handler_t button = (button_handler_t) arg;
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    while(1) {
        int level = gpio_get_level(button->gpio);
        if(level == button->state) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }

        button->last_state = button->state;
        button->state = level;
        button->state_change_time = esp_timer_get_time();

        if(level && button->on_release) {
            button->on_release(button->gpio);
        } else if (button->on_press) {
            button->on_press(button->gpio);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

button_handler_t button_init(uint8_t gpio) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    int button_size = sizeof(button_t);
    button_handler_t button = malloc(button_size);
    if(!button) {
        ESP_LOGE(TAG, "Malloc for button on GPIO %hhu failed.", gpio);
    }
    memset(button, 0, button_size);
    button->state_change_time = esp_timer_get_time(); 
    button->state = 1;
    button->gpio = gpio;

    xTaskCreate(
        button_check_task,
        "button_task",
        2048,
        button,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    return button;
}
