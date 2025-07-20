#ifndef SYNCED_TIMER_H
#define SYNCED_TIMER_H

#include <stdlib.h>

void set_remote_time(const int64_t remote_time);
int64_t get_synced_time();

#endif