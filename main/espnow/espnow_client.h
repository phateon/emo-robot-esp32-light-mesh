/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef ESPNOW_CLIENT_H
#define ESPNOW_CLIENT_H

#include <stdlib.h>
#include "esp_now.h"
#include "espnow/espnow_connect.h"

typedef struct espnow_client_t espnow_client_t;

typedef void (*espnow_connect_client_event_handler_t)(
    espnow_client_t *client, 
    espnow_event_t* event
);

// Client for esp now connections.
struct espnow_client_t {
    espnow_connect_t connection;                            // Pointer to the connection object
    espnow_connect_client_event_handler_t update_handler;   // Event handler for ESPNOW events.
    uint8_t server_mac[ESP_NOW_ETH_ALEN];                   // MAC address of the server this client is connected to.
};

espnow_client_t* espnow_client_start(const char* ssid);
esp_err_t espnow_client_stop(espnow_client_t* client);

#endif