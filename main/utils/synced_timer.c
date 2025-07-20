#include <stdlib.h>
#include "esp_timer.h"
#include "utils/synced_timer.h"

static int64_t last_remote_time = 0;
static int64_t local_time_at_sync = 0;


void set_remote_time(const int64_t remote_time) {
    last_remote_time = remote_time;
    local_time_at_sync = esp_timer_get_time();
}

int64_t get_synced_time() {
    int64_t local_delta = esp_timer_get_time() - local_time_at_sync;
    return last_remote_time + local_delta;
};
