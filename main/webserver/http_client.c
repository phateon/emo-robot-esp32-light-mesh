#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "nvs_flash.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"

#define WIFI_SSID "Benji2.4"
#define WIFI_PASS "somethingeasy"
#define WIFI_CONNECTED_BIT BIT0

static const char *TAG = "HTTP_CLIENT";
static EventGroupHandle_t wifi_event_group;
static bool is_wifi_init = false;
static uint8_t wifi_fail_count = 0;
static const TickType_t xTicksToWait = 5000 / portTICK_PERIOD_MS; 
esp_http_client_config_t config = {
    .crt_bundle_attach = esp_crt_bundle_attach
};

/**
 * @brief Wi-Fi event handler function.
 *
 * This function handles Wi-Fi and IP events. It sets the `WIFI_CONNECTED_BIT`
 * in the event group when a Wi-Fi connection is established and an IP is assigned.
 */
static void _wifi_event_handler(
    void *arg, 
    esp_event_base_t event_base, 
    int32_t event_id, 
    void *event_data
) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        is_wifi_init = true;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


/**
 * @brief Task for making the HTTP GET request.
 *
 * This task runs in an infinite loop. In each cycle, it connects the Wi-Fi,
 * performs an HTTP request, and then explicitly disconnects the Wi-Fi to
 * enter a low-power mode for the modem. It then delays before the next cycle.
 */
void http_connect_get(
    const char* url, 
    http_event_handle_cb http_event_handler
) {
    // Check if wifi is ready
    if (!is_wifi_init) {
        ESP_LOGE(TAG, "WiFI not initialized.");
        return;
    }

    // Connect Wi-Fi
    esp_err_t con_err = esp_wifi_connect();
    if (con_err != ESP_OK) {
        ESP_LOGE(TAG, "WiFI connection failed: %s", esp_err_to_name(con_err));
        return;
    }

    // Wait until Wi-Fi is connected
    EventBits_t uxBits = xEventGroupWaitBits(
        wifi_event_group, 
        WIFI_CONNECTED_BIT, 
        pdTRUE, 
        pdFALSE, 
        xTicksToWait
    );

    if((uxBits & WIFI_CONNECTED_BIT) != 0 ) {
        config.url = url;
        config.event_handler = http_event_handler;

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_err_t err = esp_http_client_perform(client);

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
            wifi_fail_count++;
        } else {
            wifi_fail_count = 0;
        }

        // Disconnect Wi-Fi to save power
        esp_http_client_cleanup(client);
    } else { 
        ESP_LOGE(TAG, "WiFI connection timed out %d times.", wifi_fail_count);
        wifi_fail_count++;
    }

    esp_wifi_disconnect();
    if (wifi_fail_count > 3) {
        ESP_LOGE(TAG, 
            "FATAL HTTP error get failed %d times. Restarting...", 
            wifi_fail_count
        );
        esp_restart();
    }
}

/**
 * @brief Wi-Fi initialization for station mode.
 *
 * This function sets up the Wi-Fi in station mode and enables the
 * minimal modem sleep power-save mode.
 */
static void wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT, 
            ESP_EVENT_ANY_ID, 
            _wifi_event_handler, 
            NULL));
    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT, 
            IP_EVENT_STA_GOT_IP, 
            _wifi_event_handler, 
            NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/**
 * @brief Main application entry point.
 *
 * This is the first function called upon startup. It initializes the system,
 * initializes Wi-Fi, and creates the HTTP task.
 */
void start_web_client(void) {
    // Initialize NVS (Non-Volatile Storage)
    ESP_LOGI("WEB_CLIENT", "Started nvs");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi
    wifi_init_sta();
}

void cleanup_web_client(void) {
    // Cleanup Wi-Fi resources
    esp_event_handler_unregister(
        WIFI_EVENT, 
        ESP_EVENT_ANY_ID, 
        _wifi_event_handler);
    esp_event_handler_unregister(
        IP_EVENT, 
        IP_EVENT_STA_GOT_IP, 
        _wifi_event_handler);
    esp_wifi_stop();
    esp_wifi_deinit();
    vEventGroupDelete(wifi_event_group);
}


#endif // HTTP_CLIENT_H
