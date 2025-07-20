#ifndef WIFI_HTTP_SERVER_H
#define WIFI_HTTP_SERVER_H

#include "esp_http_server.h"

esp_err_t start_http_server(httpd_handle_t http_server);
esp_err_t stop_http_server(httpd_handle_t http_server);


#endif // WIFI_HTTP_SERVER_H
