#include <stdint.h>
#include <string.h>

#include "esp_mac.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_now.h"

#include "espnow/espnow_connect.h"
#include "espnow/espnow_server.h"
#include "utils/map.h"


#define MAX_CLIENTS 16

static const char *TAG = "espnow_connect_server";


// ESPNOW server received a client request for join
static void handle_join_request(
    espnow_event_t *event, 
    espnow_server_t *server
) {
    espnow_event_recv_cb_t *client = &event->info.recv_cb;
    espnow_data_t data;
    data.event_id = ESPNOW_JOIN_RESPONSE;
    ESP_ERROR_CHECK(
        espnow_connect_send(
            &server->connection, 
            &data, 
            "join_response", 
            client->mac_addr
        )
    );

    ESP_LOGI(TAG, "Responded to join request from "MACSTR, MAC2STR(client->mac_addr));
}


// Handle the queue task for processing ESPNOW events.
static void queue_task(void* server_ptr) {
    espnow_server_t* server = (espnow_server_t *)server_ptr;

    // Register event handler for server messages
    espnow_event_t event;
    while (xQueueReceive(server->connection.queue, &event, portMAX_DELAY) == pdTRUE) {
        switch (event.id) {
            case ESPNOW_JOIN_REQUEST: { handle_join_request(&event, server); break; }
            default: { break; }
        }

        free(event.info.recv_cb.data);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


// Handle the client task for sending join requests and checking server status.
static void server_task(void* server_ptr) {
    espnow_server_t* server = (espnow_server_t*)server_ptr;
    espnow_data_t data;
    data.event_id = ESPNOW_SYNC;

    while (1) {
        espnow_connect_send(
            &server->connection,
            &data,
            "sync",
            NULL
        );
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


// Start the ESPNOW server task and initialize the event listener.
espnow_server_t* espnow_server_start(
    const char* ssid,
    const char* password
){
    // Initialize client connection
    int server_size = sizeof(espnow_server_t);
    espnow_server_t* server = malloc(server_size);
    memset(server, 0, server_size);
    server->connection.energy_safer = true; // Start the connection in energy safer mode
    server->connection.wake_window = 100;   // How long to stay awake for in ms
    server->connection.wake_interval = 100; // How long to wait in between wake periods
    server->connection.priority = 2;        // Task priority of handling queue for received ESP NOW messages
    server->connection.ssid = ssid;         // SSID
    server->connection.password = password; // Password

    // Initialize esp now client connection
    ESP_ERROR_CHECK(espnow_connect_init(&server->connection));

    // Start the ESPNOW client task
    xTaskCreate(
        queue_task,
        "espnow_server_queue", 
        2048,
        (void*) server,
        tskIDLE_PRIORITY + 2,
        NULL
    );

    // Start the server task
    xTaskCreate(
        server_task,
        "espnow_server_task", 
        2048,
        (void*) server,
        tskIDLE_PRIORITY + 2,
        NULL
    );

    return server;
}

esp_err_t espnow_server_stop(espnow_server_t *server) {
    if (server == NULL) {
        ESP_LOGE(TAG, "server is NULL.");
        return ESP_ERR_INVALID_ARG;
    }

    // Deinitialize the client connection
    esp_err_t ret = espnow_connect_deinit(&server->connection);

    // Free the client memory
    free(server);
    server = NULL;

    ESP_LOGI(TAG, "ESP-NOW server stopped.");
    return ret;
}

esp_err_t espnow_server_broadcast_message_to_clients(
    espnow_server_t *server,
    const char* message
) {
    espnow_data_t data;
    data.event_id = ESPNOW_UPDATE;
    espnow_connect_send(&server->connection, &data, message, NULL);
    return ESP_OK;
}

