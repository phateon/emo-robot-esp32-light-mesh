#include <math.h>
#include "esp_log.h"
#include "led_strip.h"
#include "led/effect_list.h"
#include "utils/map.h"

#define BLINK_GPIO 6
#define LED_STRIP_LENGTH 50
#define MAX_LED_EFFECTS 8

static const char *TAG = "effect_list";
static led_strip_handle_t led_strip_handle;


// Map for effect management using generic map
static map_entry_t effect_entries[MAX_LED_EFFECTS];
static map_t effect_map = {
    .entries = effect_entries,
    .size = 0,
    .capacity = MAX_LED_EFFECTS,
    .key_cmp = string_key_comparison,
    .value_free = no_cleanup, // No cleanup needed for function pointers
};

static led_strip_config_t strip_config = {
    .strip_gpio_num = BLINK_GPIO,
    .max_leds = LED_STRIP_LENGTH,
};

static const led_strip_rmt_config_t rmt_config = {
    .resolution_hz = 10 * 1000 * 1000, // 10MHz
    .flags.with_dma = false,
};

void configure_led(uint16_t led_count) {
    strip_config.max_leds = led_count;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_handle));
    led_strip_clear(led_strip_handle);
}

bool led_effect_add(const char *key, led_effect_func_t effect) {
    if (map_set(&effect_map, (void *)key, (void *)effect) != 0) {
        ESP_LOGW(TAG, "Effect '%s' already exists or map is full", key);
        return false;
    }
    return true;
}

bool led_effect_remove(const char *key) {
    if (map_remove(&effect_map, (void *)key) != 0) {
        ESP_LOGW(TAG, "Effect '%s' not found", key);
        return false;
    }
    return true;
}

void led_effect_run(int64_t time, const char *key) {
    led_effect_func_t fn = (led_effect_func_t)map_get(&effect_map, (void *)key);
    if (fn == NULL)  {
        ESP_LOGW(TAG, "Effect <%s> not found.", key);
        return;
    }
    fn(time, led_strip_handle, &strip_config);
}
