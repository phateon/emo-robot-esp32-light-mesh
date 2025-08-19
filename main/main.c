#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "esp_http_client.h"

#include "freertos/FreeRTOS.h"
#include "driver/temperature_sensor.h"

#include "main.h"
#include "main_server.h"
#include "main_client.h"

#include "webserver/http_client.h"

#include "utils/synced_timer.h"
#include "utils/button.h"
#include "utils/map.h"

#include "cJSON.h"

#include "led/renderer.h"
#include "led/emotions/sad.h"
#include "led/emotions/anger.h"
#include "led/emotions/happy.h"
#include "led/emotions/relaxed.h"
#include "led/emotions/flirty.h"

#include "led/transitions/smooth.h"


led_renderer_t* renderer_handler = NULL;
uint8_t effect_index = 0;

void on_button_press(uint8_t gpio) {
}


void on_button_release(uint8_t gpio) {
    map_t* effect_map = led_renderer_get_effect_map(renderer_handler);
    if (effect_index >= effect_map->size) {
        effect_index = 0;
    }

    led_effect_t* effect = effect_map->entries[effect_index].value;
    led_renderer_set_next_effect(renderer_handler, effect->name);
    effect_index++;
}


void set_new_effect(const char* json_str) {
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE("cJSON", "Parse error: %s.", error_ptr);
        }
        cJSON_Delete(json);
        return;
    }

    cJSON *mood = cJSON_GetObjectItem(json, "mood");
    if (mood == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE("cJSON", "Parse error: %s.", error_ptr);
        }
        cJSON_Delete(json);
        return;
    }

    cJSON *mood_val = cJSON_GetObjectItem(mood, "value");
    if (mood == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE("cJSON", "Parse error: %s.", error_ptr);
        }
        cJSON_Delete(json);
        return;
    }

    if (!cJSON_IsString(mood_val)) {
        ESP_LOGE("cJSON", "The mood.value is not a string or NULL.");
        cJSON_Delete(json);
        return;
    }

    if (mood_val->valuestring == NULL) {
        ESP_LOGE("cJSON", "The mood.value is NULL.");
        cJSON_Delete(json);
        return;
    }

    const char* new_mood = mood_val->valuestring;
    led_renderer_set_next_effect(renderer_handler, new_mood);
    cJSON_Delete(json);
}


esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    const char* json = (char*)evt->data;
    switch(evt->event_id) {
        case HTTP_EVENT_ON_DATA: set_new_effect(json); break;
        default:
            break;
    }
    return ESP_OK;
}


void web_server_task(void* args) {
    const char* url = "https://emotibot-relay.fly.dev/mood";

    // The HTTP GET task will now handle the connect/disconnect cycle
    while(true) {
        ESP_LOGI("web client", "Running web request.");
        http_connect_get(url, http_event_handler);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void led_render_task(void *arg) {
    button_handler_t button = button_init(9);
    button->on_press = on_button_press;
    button->on_release = on_button_release;

    synced_timer_t* timer = (synced_timer_t*)arg;
    update_local_time(timer);

    led_renderer_t renderer;
    led_render_init(&renderer);
    renderer_handler = &renderer;

    //led_renderer_add_effect(&renderer, "angry", &anger_effect);
    led_renderer_add_effect(&renderer, "happy", &happy_effect);
    //led_renderer_add_effect(&renderer, "calm", &relaxed_effect);
    led_renderer_add_effect(&renderer, "flirty", &flirty_effect);
    //led_renderer_add_effect(&renderer, "sad", &sad_effect);

    led_renderer_set_next_effect(renderer_handler, "sad");

    while (1) {
        update_local_time(timer);
        led_render_start(&renderer);
        led_render(&renderer, timer);
        led_render_finish(&renderer);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    led_render_free(&renderer);
}


void measure_temp_task(void* args) {
    temperature_sensor_handle_t temp_handle = NULL;
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_handle));

    float tsens_out;
    while (1) {
        ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
        ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));
        ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));
        
        ESP_LOGI("Temperature", "Current temperature: %.2f °C", tsens_out);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void monitor_resource_usage_task(void* unused) {
    int32_t delay_ms = 1000;

    ESP_LOGI("ResourceMonitor", "Monitoring resource usage...");
    while (1) {
        ESP_LOGI("ResourceMonitor", "Heap size: %d", (int)esp_get_free_heap_size());
        ESP_LOGI("ResourceMonitor", "Min heap size: %d", (int)esp_get_minimum_free_heap_size());
        vTaskDelay(delay_ms / portTICK_PERIOD_MS);
    }
}


void app_main(void) {
    synced_timer_t* timer = malloc(sizeof(synced_timer_t));
    timer->is_timer_server = true;

    start_web_client();
    
    xTaskCreate(led_render_task, "render_task", 2048, (void*)timer, tskIDLE_PRIORITY + 3, NULL);
    //xTaskCreate(web_server_task, "web_server_task", 8192 * 2, NULL, tskIDLE_PRIORITY + 2, NULL);
    //xTaskCreate(measure_temp_task, "measure_temp_task", 2048, NULL, tskIDLE_PRIORITY, NULL);
    //xTaskCreate(monitor_resource_usage_task, "monitor_task", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
}
