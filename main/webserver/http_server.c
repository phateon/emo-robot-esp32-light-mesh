#include "sdkconfig.h"
#include "mdns.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

const char* TAG = "http_server";


esp_err_t start_http_server(httpd_handle_t http_server) {
    // Start web server
    ESP_LOGI(TAG, "Starting http webserver and mDNS system");
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(httpd_start(&http_server, &config));
    ESP_LOGI(TAG, "HTTP Server started");

    // Start mDNS for mesh.local
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("mesh"));
    ESP_ERROR_CHECK(mdns_instance_name_set("ESP32 Mesh Root"));
    ESP_LOGI(TAG, "mDNS set for mesh.local");

    return ESP_OK;
}

esp_err_t stop_http_server(httpd_handle_t http_server) {
    return ESP_OK;
}

