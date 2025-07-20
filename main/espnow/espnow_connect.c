#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "esp_timer.h"

#include "espnow/espnow_connect.h"
#include "utils/map.h"


#define ESPNOW_MAXDELAY 512
#define ESP_EVENT_CALLBACK_MAX 10

static uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static espnow_connect_t* local_connection;
static QueueHandle_t s_espnow_queue = NULL;

static const char *TAG = "espnow_connect";


///////////////////////////////////////////////////////////////////////////////
// ESPNOW client and peer registration
///////////////////////////////////////////////////////////////////////////////

// Print all peer addresses
static void print_peers() {
    esp_now_peer_info_t peer;
    bool from_head = true;

    ESP_LOGI(TAG, "Connected peers:");
    while (esp_now_fetch_peer(from_head, &peer) == ESP_OK) {
        ESP_LOGI(TAG, "-- "MACSTR, MAC2STR(peer.peer_addr));
        from_head = false;
    }
}


// register a peer
static esp_err_t register_peer(const uint8_t* mac_addr) {
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        espnow_connect_deinit();
        return ESP_ERR_NO_MEM;
    }

    if (!local_connection || !local_connection->channel) {
        return ESP_ERR_WIFI_STATE;
    }

    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = local_connection->channel;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = true;
    memcpy(peer->lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
    memcpy(peer->peer_addr, mac_addr, ESP_NOW_ETH_ALEN);
    esp_err_t error = esp_now_add_peer(peer);
    free(peer);

    ESP_LOGI(TAG, "Registerd new peer ("MACSTR")", MAC2STR(mac_addr));
    print_peers();

    return error;
} 


///////////////////////////////////////////////////////////////////////////////
// WiFi Callback functions:
// Callback functions called by WiFi task. Users should not do lengthy
// operations from this task. Instead, transfer data to a queue.
///////////////////////////////////////////////////////////////////////////////

// WiFi send data callback function
static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    espnow_event_t evt;
    espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send callback argument error");
        return;
    }
    if (status != ESP_NOW_SEND_SUCCESS) {
        ESP_LOGE(TAG, "Send ESPNOW data to "MACSTR" failed", MAC2STR(mac_addr));
        return;
    }

    evt.id = ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    if (xQueueSend(s_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send queue fail");
        return;
    }

    // ESP_LOGD(TAG, "Send message to "MACSTR" successfully", MAC2STR(mac_addr));
}

// WiFi receive data callback function
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    espnow_event_t evt;
    espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
    uint8_t * mac_addr = recv_info->src_addr;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    if (esp_now_is_peer_exist(mac_addr) == false) {
        ESP_ERROR_CHECK(register_peer(mac_addr));
    }

    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }

    evt.id = ((espnow_data_t*)data)->event_id;
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(s_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }

    // ESP_LOGD(TAG, "Received message from "MACSTR" len:%d", MAC2STR(mac_addr), len);
}


///////////////////////////////////////////////////////////////////////////////
// ESPNOW Data Parse and Prepare Functions
// These functions handle the parsing and preparation of ESPNOW data packets.
///////////////////////////////////////////////////////////////////////////////

// Parse ESPNOW data received from the peer device.
int espnow_validate_message(espnow_event_recv_cb_t *message) {
    uint16_t data_len = message->data_len;
    espnow_data_t *buf = (espnow_data_t *)message->data;
    uint16_t crc, crc_cal = 0;

    if (data_len < sizeof(espnow_data_t)) {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
        return ESP_FAIL;
    }

    if(buf->payload_size > 16) {
        ESP_LOGE(TAG, "Payload size exceeds maximum limit of 16 bytes");
        return ESP_FAIL;
    }
    
    if (buf->payload[buf->payload_size - 1] != '\0') {
        ESP_LOGE(TAG, "Payload is not null-terminated.");
        return ESP_FAIL;
    }

    // Check if the payload is valid    
    crc = buf->crc;
    buf->crc = 0;
    crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, data_len);

    if (crc_cal == crc) { return ESP_OK; }
    ESP_LOGE(TAG, "ESPNOW data CRC error, expect: 0x%04x, cal: 0x%04x", crc, crc_cal);

    return ESP_FAIL;
}

// Prepare ESPNOW data to be sent.
void espnow_prepare_data(
    espnow_connect_t *connection, 
    espnow_data_t* data,
    const char* payload
) {
    //TODO: add more checks and validations.
    data->seq_num = connection->sent_count++;
    data->crc = 0;
    data->payload_size = 0;
    data->send_time = esp_timer_get_time();

    if (data->payload_size > 16) {
        ESP_LOGE(TAG, "Payload size exceeds maximum limit of 16 bytes");
        data->payload_size = 16; // Limit to 16 bytes
    }

    if (payload) {
        data->payload_size = strlen(payload) + 1;
        memcpy(data->payload, payload, data->payload_size);
    }

    // Fill all remaining bytes after the data with random values
    esp_fill_random(
        &data->payload[data->payload_size], 
        16 - data->payload_size
    );
    
    data->crc = esp_crc16_le(
        UINT16_MAX, 
        (uint8_t const *)data, 
        sizeof(espnow_data_t)
    );
}

// Send ESPNOW data to a specific destination MAC address. If no destination
// MAC address is provided, it uses the broadcast address.
esp_err_t espnow_connect_send(
    espnow_connect_t *connection, 
    espnow_data_t *data,
    const char *payload,
    const uint8_t *dest_mac
) {
    esp_err_t ret;
    if (dest_mac == NULL) {
        dest_mac = s_broadcast_mac; // Use broadcast address if none provided
        data->is_broadcast = true;
    }
    else {
        data->is_broadcast = false;
    }

    // Prepare ESPNOW data
    espnow_prepare_data(connection, data, payload);

    // Send ESPNOW data
    ret = esp_now_send(dest_mac, (uint8_t *)data, sizeof(espnow_data_t));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Send ESPNOW data failed: %s", esp_err_to_name(ret));
        return ret;
    }

    //ESP_LOGI(TAG, "ESPNOW data sent to "MACSTR, MAC2STR(dest_mac));
    return ESP_OK;
}


///////////////////////////////////////////////////////////////////////////////
// ESPNOW connect initialization and deinitialization functions
// These functions initialize and deinitialize the ESPNOW connection, including
// setting up the WiFi interface, event queue, and ESPNOW parameters.
///////////////////////////////////////////////////////////////////////////////

// Handle failure in ESPNOW connection initialization. This function is called 
// when there is a failure in initializing the ESPNOW connection.
esp_err_t espnow_connect_fail() {
    ESP_LOGE(TAG, "Malloc peer information fail");
    vQueueDelete(s_espnow_queue);
    s_espnow_queue = NULL;
    esp_now_deinit();
    return ESP_FAIL;
}


// Find the channel to setup connection with.
int get_channel_for_ap(const char* ssid) {
    wifi_scan_config_t scan_config = {0};
    esp_wifi_scan_start(&scan_config, true); // true = block until done

    uint16_t ap_num = 0;
    esp_wifi_scan_get_ap_num(&ap_num);
    wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_num);
    if (!ap_records) return -1;

    esp_wifi_scan_get_ap_records(&ap_num, ap_records);

    int channel = -1;
    for (int i = 0; i < ap_num; ++i) {
        if (strcmp((const char*)ap_records[i].ssid, ssid) == 0) {
            channel = ap_records[i].primary;
            break;
        }
    }
    free(ap_records);
    return channel; // -1 if not found
}


// Initialize wifi access conneciton 
esp_err_t connect_to_ap(espnow_connect_t* connection) {
    if (!connection->ssid) { return ESP_ERR_WIFI_SSID; }
    if (!connection->ssid) { return ESP_ERR_WIFI_PASSWORD; }

    ESP_ERROR_CHECK(esp_wifi_stop());
    wifi_config_t wifi_config = {
        .sta.channel = connection->channel
    };
    memcpy(wifi_config.sta.ssid, 
        connection->ssid, 
        strlen(connection->ssid) + 1);
    memcpy(wifi_config.sta.password, 
        connection->password, 
        strlen(connection->password) + 1);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    return ESP_OK;
}


// Handler for IP_EVENT_STA_GOT_IP event
static void on_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI("wifi_http_server", "Got IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
}


// Handler for Wi-Fi events
static void on_wifi_event(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI("wifi_http_server", "Wi-Fi connected");
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* disc = (wifi_event_sta_disconnected_t*)event_data;
        ESP_LOGW("wifi_http_server", "Wi-Fi disconnected, reason: %d", disc->reason);
    }
}


// Initialize the WiFi interface for ESPNOW connection.
esp_err_t init_wifi(espnow_connect_t *connection) {
    if (!connection->ssid) { return ESP_ERR_WIFI_SSID; }

    // Initialize None Volatile Storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Event stack and network interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(ESPNOW_WIFI_MODE));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Register event handler for IP adresses
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL, NULL));
    
    // Get the channel we will need to setup 
    ESP_ERROR_CHECK(esp_wifi_start());
    int channel = get_channel_for_ap(connection->ssid);    
    if (channel < 0) {
        return ESP_ERR_WIFI_SSID; 
    }
    
    connection->channel = channel;
    ESP_ERROR_CHECK(esp_wifi_set_channel(
        connection->channel, 
        WIFI_SECOND_CHAN_NONE)
    );
    
    if (connection->password) { 
        ESP_ERROR_CHECK(connect_to_ap(connection)); }
    
    return ESP_OK;
}


// Initialize the event queue for ESPNOW connection.   
esp_err_t init_event_que(espnow_connect_t *connection) {
    // Initialize Event Que
    s_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(espnow_event_t));
    if (s_espnow_queue == NULL) {
        ESP_LOGE(TAG, "Create queue fail");
        return ESP_FAIL;
    }

    connection->queue = s_espnow_queue;
    return ESP_OK;
}


// Register a broadcast peer for ESPNOW communication.
esp_err_t init_broadcast_peer() {
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) { return espnow_connect_fail(NULL); }

    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = local_connection->channel;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;

    memcpy(peer->peer_addr, s_broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK(esp_now_add_peer(peer));
    free(peer);

    ESP_LOGI(TAG, "Broadcast peer registered: "MACSTR, MAC2STR(s_broadcast_mac));
    return ESP_OK;
}


// Initialize ESPNOW connection. This function sets up the WiFi interface, 
// event queue, and ESPNOW parameters. It also registers the send and receive 
// callback functions.
esp_err_t espnow_connect_init(espnow_connect_t *connection) {
    local_connection = connection;

    ESP_ERROR_CHECK(init_wifi(connection));
    ESP_ERROR_CHECK(init_event_que(connection));

    // Initialize ESPNOW for sending and receiving callback function.
    ESP_ERROR_CHECK(esp_now_init() );
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));
    if (connection->energy_safer) {
        ESP_ERROR_CHECK(esp_now_set_wake_window(connection->wake_window));
        ESP_ERROR_CHECK(esp_wifi_connectionless_module_set_wake_interval(
            connection->wake_interval)
        );
    }
    
    // Set primary master key.
    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK));

    // Add broadcast peer information to peer list.
    ESP_ERROR_CHECK(init_broadcast_peer());
    return ESP_OK;
}


// Deinitialize ESPNOW connection. This function cleans up the ESPNOW
// connection, including freeing the allocated memory for the send
// parameters, deleting the event queue, and deinitializing the ESPNOW
// and WiFi interfaces.
esp_err_t espnow_connect_deinit() {
    vQueueDelete(s_espnow_queue);
    s_espnow_queue = NULL;
    esp_now_deinit();
    vTaskDelete(NULL);
    return ESP_OK;
}
