#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_mac.h"

#include "freertos/FreeRTOS.h"
#include "driver/temperature_sensor.h"

#include "main.h"
#include "main_server.h"
#include "main_client.h"

#include "led/effect_list.h"
#include "led/effects/chase.h"


void led_render_task(void *arg) {
    while (1) {
        led_effect_run(1000, effect_name ? effect_name : "anger");
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
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


void app_main(void)
{
    // Initialize the LED strip and effects
    configure_led(50);
    led_effect_add("anger", chase_led_red);
    led_effect_add("happy", chase_led_green);
    led_effect_add("sad", chase_led_blue);
    
    xTaskCreate(led_render_task, "render_task", 4096, NULL, tskIDLE_PRIORITY + 3, NULL);
    // xTaskCreate(measure_temp_task, "measure_temp_task", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
    // xTaskCreate(monitor_resource_usage_task, "monitor_task", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);

    bool run_server = true;
    if (run_server) { 
        start_server();
        effect_name = strdup("anger");
    } 
    else { 
        start_client();
        effect_name = strdup("happy");
    }
}

