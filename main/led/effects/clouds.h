#ifndef LED_EFFECT_CLOUDS_H
#define LED_EFFECT_CLOUDS_H

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "utils/integer_math/lib8simplex.h"
#include "utils/integer_math/lib8perlin.h"
#include "utils/integer_math/lib8tion.h"
#include "utils/synced_timer.h"
#include "utils/color.h"

// All values are now integer-based
typedef struct {
    uint32_t step_size;
    uint8_t min_intensity;
    uint8_t max_intensity;
    float multiplier;
    float speed;
    uint32_t t1;
} cloud_params_t;

void cloud_before_render(
    const synced_timer_t* timer, 
    void* params
) {
    cloud_params_t* cloud_params = (cloud_params_t*)params;
    cloud_params->t1 += timer->delta_time;
    cloud_params->multiplier = 
        cloud_params->max_intensity / 
        (cloud_params->max_intensity - cloud_params->min_intensity);
}

uint8_t cloud_render(
    const uint16_t position,
    const void* params
) {
    cloud_params_t* cloud_params = (cloud_params_t*)params;

    int p = position * cloud_params->step_size;
    int t = (cloud_params->t1 / 1000.0) * 
            cloud_params->step_size * cloud_params->speed;
    int n = inoise16_1d(p + t) >> 8;
    n = (n - cloud_params->min_intensity);
    return n < 0 ? 0 : n * cloud_params->multiplier;
}

#endif // LED_EFFECT_CLOUD_H