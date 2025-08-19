#ifndef PERLIN_32BIT_H
#define PERLIN_32BIT_H

#include <stdint.h>

#define FIXED_SHIFT 8
#define FIXED_ONE (1 << FIXED_SHIFT)

// Multiply two 8.8 fixed-point numbers using only int32_t
static inline int32_t fixed_mul(int32_t a, int32_t b) {
    return (a * b) >> FIXED_SHIFT;
}

// Lerp: a + (b - a) * t
static inline int32_t lerp(int32_t a, int32_t b, int32_t t) {
    return a + fixed_mul(b - a, t);
}

// Optimized gradient function — returns ±1 in fixed-point
static inline int32_t grad(int32_t x) {
    x = (x << 13) ^ x;
    x = x * (x * x * 60493 + 19990303) + 1376312589;
    return (x & 1) ? FIXED_ONE : -FIXED_ONE;
}

// Fast fade curve approximation: 3t² - 2t³
static inline int32_t fade(int32_t t) {
    int32_t t2 = fixed_mul(t, t);
    int32_t t3 = fixed_mul(t2, t);
    return fixed_mul(3 * t2 - 2 * t3, FIXED_ONE);
}

int perlin1d_int(int x) {
    int32_t xi = x >> FIXED_SHIFT;
    int32_t xf = x & (FIXED_ONE - 1);

    int32_t g0 = grad(xi);
    int32_t g1 = grad(xi + 1);

    int32_t d0 = fixed_mul(g0, xf);
    int32_t d1 = fixed_mul(g1, xf - FIXED_ONE);

    int32_t u = fade(xf);
    return lerp(d0, d1, u);
}

#endif // PERLIN_32BIT_H
