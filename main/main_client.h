#ifndef LED_CLIENT_H
#define LED_CLIENT_H

#include <string.h>
#include <stdlib.h>

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"

#include "main.h"
#include "espnow/espnow_connect.h"
#include "espnow/espnow_client.h"


void update_handler(
    espnow_client_t *client, 
    espnow_event_t *event
) {
    espnow_event_recv_cb_t* message = &event->info.recv_cb;
    espnow_data_t* data = (espnow_data_t*)message->data;
    
    bool is_server = memcmp(client->server_mac, message->mac_addr, 6) == 0;
    const char* msg_type = data->is_broadcast ? "broadcast" : "unicast";
    const char* sender = is_server ? "server" : "client";

    ESP_LOGI(
        "espnow_client", 
        "Received %d'th %s message from "MACSTR" (%s): '%s'",
        (int)data->seq_num,
        msg_type,
        MAC2STR(message->mac_addr), 
        sender,
        (char *)data->payload
    );

    effect_name = strdup(data->payload);
}

void start_client() {
    espnow_client_t* client = espnow_client_start(s_wifi_ssid);
    if (client == NULL) {
        ESP_LOGE("espnow_client", "Failed to start ESPNOW client");
        return;
    }

    client->update_handler = update_handler;
    ESP_LOGI("espnow_client", "Client starting");
}

#endif