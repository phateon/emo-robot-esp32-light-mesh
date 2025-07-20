#ifndef ESPNOW_SERVER_H
#define ESPNOW_SERVER_H

#include <stdlib.h>
#include "esp_now.h"
#include "espnow/espnow_connect.h"

/* Parameters of sending ESPNOW data. */
typedef struct {
    espnow_connect_t connection; // Pointer to the connection object
} espnow_server_t;

typedef void (*espnow_connect_event_handler_t)(
    espnow_connect_t *connection, 
    espnow_event_id_t event_id
);

espnow_server_t* espnow_server_start(
    const char* ssid,
    const char* password
);
esp_err_t espnow_server_stop(espnow_server_t *server);
esp_err_t espnow_server_broadcast_message_to_clients(
    espnow_server_t *server,
    const char* message
);

#endif
