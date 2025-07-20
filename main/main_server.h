#ifndef LED_SERVER_H
#define LED_SERVER_H

#include <string.h>
#include <stdlib.h>

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_http_server.h"

#include "main.h"
#include "utils/button.h"
#include "espnow/espnow_connect.h"
#include "espnow/espnow_server.h"
#include "webserver/http_server.h"


espnow_server_t* espnow_server = NULL;
httpd_handle_t http_server = NULL;
static const char* TAG = "led_server";


static esp_err_t set_effect(const char* new_effect) {
    if(effect_name) {
        free(effect_name);
    }
    effect_name = strdup(new_effect);
    return ESP_OK;
}


static void on_button_released(uint8_t gpio) {
    if(memcmp(effect_name, "sad", 3) == 0) {
        ESP_ERROR_CHECK(set_effect("happy"));
    } else {
        ESP_ERROR_CHECK(set_effect("sad"));
    }

    ESP_ERROR_CHECK(espnow_server_broadcast_message_to_clients(
        espnow_server, 
        effect_name)
    );

    ESP_LOGI(TAG, "Key press handled");
}


static esp_err_t effect_handler(httpd_req_t *req)
{
    char buf[64];
    int ret = httpd_req_get_url_query_str(req, buf, sizeof(buf));
    if (ret != ESP_OK) {
        httpd_resp_send_404(req);
        return ret;
    }

    char effect[32];
    ret = httpd_query_key_value(buf, "name", effect, sizeof(effect));
    if (ret != ESP_OK) {
        httpd_resp_send_err(req, 400, "Request parameter 'name' is missing.");
        return ret;
    }
    
    ESP_ERROR_CHECK(set_effect(effect));
    ESP_ERROR_CHECK(espnow_server_broadcast_message_to_clients(espnow_server, effect_name));
    ESP_ERROR_CHECK(httpd_resp_send(req, effect, HTTPD_RESP_USE_STRLEN));
    return ESP_OK;
}


static esp_err_t help_handler(httpd_req_t *req)
{
    const char *html =
        "<!DOCTYPE html>"
        "<html><head><title>ESP32 LED Effects</title></head>"
        "<body>"
        "<h1>ESP32 LED Effects Control</h1>"
        "<p>Available effects:</p>"
        "<ul>"
        "<li><a href='/effect?name=happy'>happy</a></li>"
        "<li><a href='/effect?name=sad'>sad</a></li>"
        "<li><a href='/effect?name=anger'>anger</a></li>"
        "</ul>"
        "<p>Use <code>/effect?name=EFFECT</code> to activate an effect.</p>"
        "</body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


void start_server() {
    ESP_LOGI(TAG, "Server starting");
    espnow_server = espnow_server_start(s_wifi_ssid, s_wifi_password);
    if (espnow_server == NULL) {
        ESP_LOGE(TAG, "Failed to start ESPNOW server");
        return;
    }

    uint8_t gpio = 9;
    button_handler_t button = button_init(gpio);
    button->on_release = on_button_released;

    ESP_ERROR_CHECK(start_http_server(http_server));

    httpd_uri_t effect_uri = {
        .uri = "/effect",
        .method = HTTP_GET,
        .handler = effect_handler,
        .user_ctx = NULL
    };

    httpd_uri_t help_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = help_handler,
        .user_ctx = NULL
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &effect_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &help_uri));
}

#endif