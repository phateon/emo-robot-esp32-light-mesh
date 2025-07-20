#include <string.h>

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "portmacro.h"

#include "freertos/FreeRTOS.h"

#include "utils/synced_timer.h"
#include "espnow/espnow_connect.h"
#include "espnow/espnow_client.h"


static const char *TAG = "espnow_client";


// ESPNOW client received a request for join response from server
static void handle_join_response(
    espnow_event_t *event,
    espnow_client_t *client
) {
    espnow_event_recv_cb_t *server = &event->info.recv_cb;
    memcpy(
        client->server_mac, 
        server->mac_addr, 
        ESP_NOW_ETH_ALEN
    );
    ESP_LOGI(TAG, "Registered with server: "MACSTR, MAC2STR(server->mac_addr));
}

// Update the internal server time
static void handle_sync_message(
    espnow_event_t *event, 
    espnow_client_t *client
) {
    const espnow_data_t* data = (espnow_data_t*)event->info.recv_cb.data;
    set_remote_time(data->send_time);

    ESP_LOGI(TAG, "Server send timer update.");
}

// ESPNOW server update
static void handle_update_message(
    espnow_event_t *event, 
    espnow_client_t *client
) {
    if (client->update_handler) {
        client->update_handler(client, event);
    }
    ESP_LOGI(TAG, "Server send update message.");
}

// Handle the queue task for processing ESPNOW events.
static void queue_task(void* client_ptr) {
    espnow_client_t* client = (espnow_client_t *)client_ptr;

    // Register event handler for server messages
    espnow_event_t event;
    while (xQueueReceive(client->connection.queue, &event, portMAX_DELAY) == pdTRUE) {
        switch (event.id) {
            case ESPNOW_JOIN_RESPONSE: { handle_join_response(&event, client); break; }
            case ESPNOW_SYNC: { handle_sync_message(&event, client); break;}
            case ESPNOW_UPDATE: { handle_update_message(&event, client); break; }
            default: { break; }
        }

        free(event.info.recv_cb.data);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


// Handle the client task for sending join requests and checking server status.
static void client_task(void* client_ptr) {
    espnow_client_t* client = (espnow_client_t*)client_ptr;
    espnow_data_t data;
    data.event_id = ESPNOW_JOIN_REQUEST;

    while (1) {
        if (client->server_mac[0] == 0) {
            ESP_ERROR_CHECK(
                espnow_connect_send(
                    &client->connection, 
                    &data, 
                    "join_request", 
                    NULL
                )
            );
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        else {
            // Check that the server has send an update in the last 10 seconds.
            // If not, reset client server mac adress to send join requests.
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
    }
}


// Start the ESPNOW client task and initialize the event listener.
espnow_client_t* espnow_client_start(const char* ssid) {
    // Initialize client connection
    int client_size = sizeof(espnow_client_t);
    espnow_client_t* client = malloc(client_size);
    memset(client, 0, client_size);
    client->connection.energy_safer = true; // Start the connection in energy safer mode
    client->connection.wake_window = 100;   // How long to stay awake for in ms
    client->connection.wake_interval = 100; // How long to wait in between wake periods
    client->connection.priority = 2;        // Task priority of handling queue for received ESP NOW messages
    client->connection.ssid = ssid;         // SSID to find wifi channel
    
    // Initialize esp now client connection
    ESP_ERROR_CHECK(espnow_connect_init(&client->connection));

    // Start the ESPNOW client task
    xTaskCreate(
        client_task,
        "espnow_client_task", 
        2048,
        (void*) client,
        tskIDLE_PRIORITY + 2,
        NULL
    );

    // Create the event queue for handling ESPNOW events
    xTaskCreate(
        queue_task,
        "espnow_client_queue",
        2048,
        (void*) client,
        tskIDLE_PRIORITY + 2,
        NULL
    );

    return client;
}

esp_err_t espnow_client_stop(espnow_client_t *client){
    if (client == NULL) {
        ESP_LOGE(TAG, "Client is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    // Deinitialize the client connection
    esp_err_t ret = espnow_connect_deinit(&client->connection);

    // Free the client memory
    free(client);
    client = NULL;

    ESP_LOGI(TAG, "ESP-NOW client stopped.");
    return ret;
} 

