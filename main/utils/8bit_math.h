#ifndef MATH_H
#define MATH_H

#include <stdint.h>
#include <stdio.h>
#include <math.h>

// 8-bit fixed-point smoothstep function
// Input x is an 8-bit unsigned integer representing a value between 0 and 255 (0.0 to 1.0)
// Output is an 8-bit unsigned integer representing the smoothstepped value
uint8_t smoothstep_8bit(uint8_t edge0, uint8_t edge1, uint8_t x) {
    // Scale x to the range [0, 255] relative to edge0 and edge1
    uint16_t t_unclamped = (uint16_t)x - edge0;
    uint16_t range = (uint16_t)edge1 - edge0;
    uint8_t t = (range == 0) ? 0 : (uint8_t)((t_unclamped * 255 + (range / 2)) / range);

    // Apply the smoothstep formula: 3t^2 - 2t^3
    uint16_t t16 = t;
    uint16_t t2 = (t16 * t16 + 0x80) >> 8; // t^2
    uint16_t t3 = (t2 * t16 + 0x80) >> 8;  // t^3
    int16_t result = 3 * t2 - 2 * t3;

    return (uint8_t)result;
}

// 8-bit sine function
// Input x is an 8-bit unsigned integer representing an angle from 0 to 255 (0 to 2*PI)
// Output is an 8-bit unsigned integer representing the sine value (0 to 255, centered at 127)
uint8_t sin_8bit(uint8_t x) {
    // This is a lookup table for a quarter sine wave (0 to PI/2) scaled to 0-63 input and 0-127 output.
    // The full sine wave (0-255 input) can be constructed from this quarter wave.
    // Values are scaled: 0 -> 127, 127 -> 255, 255 -> 127, 383 -> 0 (approx)
    // This table has 64 entries for 0 to PI/2 (0 to 63 in 8-bit angle)
    const uint8_t sine_quarter_table[] = {
        0,   6,  12,  18,  24,  30,  36,  42,  48,  53,  59,  64,  69,  74,  79,  84,
        88,  92,  96, 100, 104, 107, 110, 113, 115, 117, 119, 121, 122, 123, 124, 125,
        126, 127, 127, 127, 126, 125, 124, 123, 122, 121, 119, 117, 115, 113, 110, 107,
        104, 100,  96,  92,  88,  84,  79,  74,  69,  64,  59,  53,  48,  42,  36,  30,
        24,  18,  12,   6,   0 };

    uint8_t quadrant = x >> 6; // Determine quadrant (0-3)
    uint8_t index = x & 0x3F;  // Get index within the quadrant (0-63)

    uint8_t value;
    if (quadrant == 0) { // 0 to PI/2
        value = sine_quarter_table[index];
    } else if (quadrant == 1) { // PI/2 to PI
        value = sine_quarter_table[63 - index];
    } else if (quadrant == 2) { // PI to 3*PI/2
        value = 255 - sine_quarter_table[index];
    } else { // 3*PI/2 to 2*PI
        value = 255 - sine_quarter_table[63 - index];
    }
    return value;
}

// 8-bit linear interpolation function
// Input a and b are 8-bit unsigned integers
// Input t is an 8-bit unsigned integer representing the interpolation factor (0.0 to 1.0)
// Output is an 8-bit unsigned integer representing the interpolated value
uint8_t lerp_8bit(uint8_t a, uint8_t b, uint8_t t) {
    // Scale t to 0-256 for multiplication, then divide by 256 (>> 8)
    // This avoids floating-point arithmetic.
    // (a * (255 - t) + b * t) / 255
    // To avoid overflow, cast to uint16_t before multiplication
    uint16_t result = ((uint16_t)a * (255 - t) + (uint16_t)b * t + 127) / 255;
    if (result > 255) result = 255; // Clamp to 255 in case of rounding issues
    return (uint8_t)result;
}


// 8-bit pseudo-random number generator based on input x and a tile_id
// This function aims to provide a deterministic "random" value for a given x and tile_id,
// useful for generating consistent noise patterns across different tiles or positions.
// Input x is an 8-bit unsigned integer (0-255), representing a position or phase.
// Input tile_id is an 8-bit unsigned integer (0-255), representing a specific tile or context.
// Output is an 8-bit unsigned integer (0-255) representing the pseudo-random value.
uint8_t random_8bit(uint8_t x, uint8_t tile_id) {
    // A simple, deterministic hash function combining x and tile_id.
    // Using prime numbers for multiplication and addition helps distribute values.
    // The modulo 256 operation keeps the result within 8 bits.
    uint8_t hash = (x * 157 + tile_id * 113 + 79) % 256;
    // Further mix the bits for better distribution
    hash = (hash ^ (hash >> 4)) * 233;
    return hash;
}


// 8-bit 1D Perlin-like noise function
// Input x is an 8-bit unsigned integer representing a value between 0 and 255 (0.0 to 1.0)
// Output is an 8-bit unsigned integer representing the noise value (0 to 255)
uint8_t noise1d_8bit(uint8_t x, uint8_t tile) {
    // Simple pseudo-random number generator for gradient values
    // This is a Perlin-like approximation for 8-bit.

    // For 8-bit, we consider 'x' as both the integer part and the fractional part.
    // We generate two pseudo-random values based on 'x' and 'x+1' (wrapping around 255).
    // The 'tile_id' parameter in random_8bit can be used as a seed for the noise,
    // but for 1D noise, we can just use a constant or 0 if not needed for seeding.
    // Here, we use 0 for tile_id as it's a 1D noise function.
    uint8_t val0 = random_8bit(x, tile); // Pseudo-random value for 'x'
    uint8_t val1 = random_8bit(x + 1, tile); // Pseudo-random value for 'x+1'
    uint8_t t = smoothstep_8bit(0, 255, x); // Use smoothstep for interpolation factor

    return lerp_8bit(val0, val1, t);
}


#endif // MATH_H
