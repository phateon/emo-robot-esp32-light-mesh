#include <stdlib.h>
#include "esp_timer.h"
#include "utils/synced_timer.h"


void update_remote_time(synced_timer_t* timer, const int64_t remote_time) {
    timer->last_remote_time = remote_time;
    timer->local_time_at_sync = esp_timer_get_time() / 1000;
    timer->delta_time = timer->local_time_at_sync - timer->last_local_time;
    timer->last_local_time = timer->local_time_at_sync;
}

void update_local_time(synced_timer_t* timer) {
    uint64_t current = esp_timer_get_time() / 1000;
    timer->delta_time = current - timer->last_local_time;
    timer->last_local_time = current;
}

int64_t get_synced_time(const synced_timer_t* timer) {
    if (timer->is_timer_server) {
        return timer->last_local_time;
    }

    int64_t local_delta = timer->last_local_time - timer->local_time_at_sync;
    return timer->last_remote_time + local_delta;
};
