#ifndef LED_COLOR_H
#define LED_COLOR_H

#include <stdint.h>
#include <math.h>

#define APPLY_DIMMING(X) (X)
#define HSV_SECTION_6 (0x20)
#define HSV_SECTION_3 (0x40)

// RGB color struct
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;


// Convert HSV struct to RGB (0-255)
static inline void rgb_from_hsv(
    const uint8_t hue, 
    const uint8_t saturation, 
    const uint8_t value,
    rgb_color_t* out
) {
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t v = APPLY_DIMMING(value);

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = APPLY_DIMMING(255 - saturation);
    uint8_t brightness_floor = (v * invsat) / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = hue / HSV_SECTION_3; // 0..2
    uint8_t offset = hue % HSV_SECTION_3;  // 0..63

    uint8_t rampup = offset; // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

    // We now scale rampup and rampdown to a 0-255 range -- at least
    // in theory, but here's where architecture-specific decsions
    // come in to play:
    // To scale them up to 0-255, we'd want to multiply by 4.
    // But in the very next step, we multiply the ramps by other
    // values and then divide the resulting product by 256.
    // So which is faster?
    //   ((ramp * 4) * othervalue) / 256
    // or
    //   ((ramp    ) * othervalue) /  64
    // It depends on your processor architecture.
    // On 8-bit AVR, the "/ 256" is just a one-cycle register move,
    // but the "/ 64" might be a multicycle shift process. So on AVR
    // it's faster do multiply the ramp values by four, and then
    // divide by 256.
    // On ARM, the "/ 256" and "/ 64" are one cycle each, so it's
    // faster to NOT multiply the ramp values by four, and just to
    // divide the resulting product by 64 (instead of 256).
    // Moral of the story: trust your profiler, not your insticts.

    // Since there's an AVR assembly version elsewhere, we'll
    // assume what we're on an architecture where any number of
    // bit shifts has roughly the same cost, and we'll remove the
    // redundant math at the source level:

    //  // scale up to 255 range
    //  //rampup *= 4; // 0..252
    //  //rampdown *= 4; // 0..252

    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj = (rampup * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor = rampup_amp_adj + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;

    if (section)
    {
        if (section == 1)
        {
            // section 1: 0x40..0x7F
            out->r = brightness_floor;
            out->g = rampdown_adj_with_floor;
            out->b = rampup_adj_with_floor;
        }
        else
        {
            // section 2; 0x80..0xBF
            out->r = rampup_adj_with_floor;
            out->g = brightness_floor;
            out->b = rampdown_adj_with_floor;
        }
    }
    else
    {
        // section 0: 0x00..0x3F
        out->r = rampdown_adj_with_floor;
        out->g = rampup_adj_with_floor;
        out->b = brightness_floor;
    }
}


// Gamma = 2.2 lookup table for 8-bit values (0–255)
static const uint8_t gamma8_table[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
    2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7,
    8, 8, 9, 9,10,10,11,12,12,13,14,14,15,16,16,17,
   18,19,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
   33,34,36,37,38,39,41,42,43,45,46,48,49,51,52,54,
   55,57,59,60,62,64,66,68,69,71,73,75,77,79,81,83,
   85,87,89,91,93,96,98,100,103,105,107,110,112,115,
  117,120,122,125,128,130,133,136,139,141,144,147,150,153,156,159,
  162,165,168,171,174,178,181,184,187,191,194,197,201,204,208,211,
  215,218,222,225,229,233,236,240,244,248,251,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
};

static inline void apply_gamma_rgb(rgb_color_t *color) {
    color->r = gamma8_table[color->r];
    color->g = gamma8_table[color->g];
    color->b = gamma8_table[color->b];
};

#endif // LED_COLOR_H
