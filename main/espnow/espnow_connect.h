/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef ESPNOW_CONNECT_H
#define ESPNOW_CONNECT_H

#include <stdlib.h>
#include "esp_now.h"
#include "freertos/FreeRTOS.h"
 

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */
#if CONFIG_ESPNOW_WIFI_MODE_STATION
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_AP
#endif

#define ESPNOW_QUEUE_SIZE           6

#define IS_BROADCAST_ADDR(addr) (memcmp(addr, s_broadcast_mac, ESP_NOW_ETH_ALEN) == 0)


typedef enum {
    ESPNOW_JOIN_REQUEST,    // Client requests to join the network
    ESPNOW_JOIN_RESPONSE,   // Server responds to client join request
    ESPNOW_UPDATE,          // Server sends a message to clients
    ESPNOW_SYNC,            // Server send timer sync
    ESPNOW_RECV_CB,
    ESPNOW_SEND_CB,
    ESPNOW_IGNORE
} espnow_event_id_t;

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} espnow_event_send_cb_t;

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    uint8_t *data;
    int data_len;
} espnow_event_recv_cb_t;

typedef union {
    espnow_event_send_cb_t send_cb;
    espnow_event_recv_cb_t recv_cb;
} espnow_event_info_t;

/* When ESPNOW sending or receiving callback function is called, post event to ESPNOW task. */
typedef struct {
    espnow_event_id_t id;
    espnow_event_info_t info;
} espnow_event_t;

/* User defined field of ESPNOW data in this example. */
typedef struct {
    bool is_broadcast;      // Was this a broadcast or unicast message?
    uint8_t event_id;       // Event id to handle request on recevier.
    uint32_t seq_num;       // Sequence number of message.
    int64_t send_time;      // Time this message was send in sender local time.   
    uint16_t crc;           // CRC16 value of message.
    uint8_t payload_size;   // Size in bytes used for actual data in payload. The rest will be filled with random values.
    char payload[16];       // Payload of ESPNOW data.
} __attribute__((packed)) espnow_data_t;

/* Parameters of sending ESPNOW data. */
typedef struct {
    uint8_t priority;           // Task priority of sending ESPNOW data.
    bool energy_safer;          // Start the connection in energy sfer mode.
    uint16_t wake_window;       // How long to stay awake for in ms.
    uint16_t wake_interval;     // How long to wait in between in ms.
    uint32_t sent_count;        // Total count of ESPNOW data sent.
    uint32_t receive_count;     // Total count of ESPNOW data received.
    const char* ssid;           // SSID for the wifi access point.
    const char* password;       // Password for the access point.
    uint8_t channel;            // Channel of the access point.
    QueueHandle_t queue;        // Queue for handling ESPNOW events.
} espnow_connect_t;

esp_err_t espnow_connect_init(espnow_connect_t *connection);
esp_err_t espnow_connect_deinit();
esp_err_t espnow_connect_send(
    espnow_connect_t *connection, 
    espnow_data_t *data,
    const char *payload,
    const uint8_t *dest_mac
);

#endif