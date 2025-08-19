#ifndef SYNCED_TIMER_H
#define SYNCED_TIMER_H

#include <stdlib.h>

//all times have to be in ms
typedef struct {
    bool is_timer_server;
    
    int64_t last_remote_time;
    int64_t local_time_at_sync;
    int64_t last_local_time;
    int64_t delta_time;
} synced_timer_t;

void update_remote_time(synced_timer_t* timer, const int64_t remote_time_ms);
void update_local_time(synced_timer_t* timer);
int64_t get_synced_time(const synced_timer_t* timer);

#endif