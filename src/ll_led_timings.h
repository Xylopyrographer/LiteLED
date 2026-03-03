//
/*
    LiteLED LED Timing Definitions

    RMT symbol timing definitions for various LED strip types:
    - WS2812 / WS2812B
    - WS2812 RGB variant
    - APA106
    - SM16703
    - SK6812
*/

#ifndef __LL_LED_TIMINGS_H__
#define __LL_LED_TIMINGS_H__

#include "LiteLED.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000    /* 10 MHz resolution, 1 tick = 0.1us (led strip needs a high resolution) */

// WS2812 / WS2812B timing (most common addressable LEDs)
static const rmt_symbol_word_t ws2812_zero = {
    static_cast<uint16_t>( 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T0H=0.3us
    1,                                                                      // .level0
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T0L=0.9us
    0                                                                       // .level1
};

static const rmt_symbol_word_t ws2812_one = {
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T1H=0.9us
    1,                                                                      // .level0
    static_cast<uint16_t>( 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T1L=0.3us
    0                                                                       // .level1
};

static const rmt_symbol_word_t ws2812_reset = {                                 /* WS2812 reset is 50uS */
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2 ),    //.duration0 = 25us
    0,                                                                          // .level0
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2 ),    //.duration1 = 25us
    0                                                                           // .level1
};

// APA106 timing
static const rmt_symbol_word_t apa106_zero = {
    static_cast<uint16_t>( 0.35 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T0H=0.35us
    1,                                                                       // .level0
    static_cast<uint16_t>( 1.36 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T0L=1.36us
    0                                                                        // .level1
};

static const rmt_symbol_word_t apa106_one = {
    static_cast<uint16_t>( 1.36 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T1H=1.36us
    1,                                                                       // .level0
    static_cast<uint16_t>( 0.35 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T1L=0.35us
    0                                                                        // .level1
};

static const rmt_symbol_word_t apa106_reset = {                                 /* APA106 reset is 50uS */
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2 ),    //.duration0 = 25us
    0,                                                                          // .level0
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2 ),    //.duration1 = 25us
    0                                                                           // .level1
};

// SM16703 timing
static const rmt_symbol_word_t sm16703_zero = {
    static_cast<uint16_t>( 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T0H=0.3us
    1,                                                                      // .level0
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T0L=0.9us
    0                                                                       // .level1
};

static const rmt_symbol_word_t sm16703_one = {
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T1H=0.9us
    1,                                                                      // .level0
    static_cast<uint16_t>( 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T1L=0.3us
    0                                                                       // .level1
};

static const rmt_symbol_word_t sm16703_reset = {                                /* SM16703 reset is 210uS */
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 210 / 2 ),   //.duration0 = 105us
    0,                                                                          // .level0
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 210 / 2 ),   //.duration1 = 105us
    0                                                                           // .level1
};

// SK6812 timing (RGBW LEDs)
static const rmt_symbol_word_t sk6812_zero = {
    static_cast<uint16_t>( 0.32 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T0H=0.32us
    1,                                                                       // .level0
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),    // .duration1 = T0L=0.9us
    0                                                                        // .level1
};

static const rmt_symbol_word_t sk6812_one = {
    static_cast<uint16_t>( 0.64 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T1H=0.64us
    1,                                                                       // .level0
    static_cast<uint16_t>( 0.4 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),    // .duration1 = T1L=0.4us
    0                                                                        // .level1
};

static const rmt_symbol_word_t sk6812_reset = {                                 /* SK6812 reset is 90uS */
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 90 / 2 ),    //.duration0 = 45us
    0,                                                                          // .level0
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 90 / 2 ),    //.duration1 = 45us
    0                                                                           // .level1
};

// LED type parameter structures (combine timing + color order)
static const led_params_t led_ws2812 = {
    .led_0 = ws2812_zero,
    .led_1 = ws2812_one,
    .led_reset = ws2812_reset,
    .order = ORDER_GRB      // Standard WS2812 color order
};

static const led_params_t led_ws2812_rgb = {
    /* Some "WS2812" LEDs have non-standard RGB colour order */
    .led_0 = ws2812_zero,
    .led_1 = ws2812_one,
    .led_reset = ws2812_reset,
    .order = ORDER_RGB
};

static const led_params_t led_apa106 = {
    .led_0 = apa106_zero,
    .led_1 = apa106_one,
    .led_reset = apa106_reset,
    .order = ORDER_RGB
};

static const led_params_t led_sm16703 = {
    .led_0 = sm16703_zero,
    .led_1 = sm16703_one,
    .led_reset = sm16703_reset,
    .order = ORDER_RGB
};

static const led_params_t led_sk6812 = {
    .led_0 = sk6812_zero,
    .led_1 = sm16703_one,
    .led_reset = sm16703_reset,
    .order = ORDER_GRB
};

// LED parameters lookup table indexed by led_strip_type_t
static const led_params_t led_params[] = {
    [ LED_STRIP_WS2812 ]     = led_ws2812,
    [ LED_STRIP_WS2812_RGB ] = led_ws2812_rgb,
    [ LED_STRIP_SK6812 ]     = led_sk6812,
    [ LED_STRIP_APA106 ]     = led_apa106,
    [ LED_STRIP_SM16703 ]    = led_sm16703
};

// String names for LED types (for debugging)
static const char *led_type[] = {
    "ws2812",
    "ws2812_rgb",
    "sk6812",
    "apa106",
    "sm16703"
};

// String names for color orders (for debugging)
static const char *col_ord[] = {
    "order_rgb",
    "order_rbg",
    "order_grb",
    "order_gbr",
    "order_brg",
    "order_bgr",
    "order_max"
};

// Color order management
static color_order_t custom_color_order = ORDER_GRB;
static bool use_custom_color_order = false;

// -------------------------------------------------------------------------
// PARLIO timing parameters
// -------------------------------------------------------------------------
// All supported LED types are driven at 2.5 MHz with 3 samples per bit.
// Each LED data bit maps to 3 PARLIO clock cycles (400 ns each):
//
//   LED bit 0  →  [HIGH, LOW,  LOW ] → T0H = 400 ns, T0L = 800 ns
//   LED bit 1  →  [HIGH, HIGH, LOW ] → T1H = 800 ns, T1L = 400 ns
//
// This satisfies WS2812 / WS2813 / SK6812 / SM16703 timing requirements.
//
// APA106 note: the datasheet T0L minimum (1210 ns) is not met at this
// clock rate (T0L = 800 ns). Works in practice but is an approximation.
// A future refinement can add per-type clock rates for strict compliance.
//
// Reset: 125 zero-bytes × 8 bits × 400 ns = 400 µs LOW — covers all types.
// -------------------------------------------------------------------------

#ifndef SOC_PARLIO_SUPPORTED
    #define SOC_PARLIO_SUPPORTED 0
#endif

#if SOC_PARLIO_SUPPORTED

#define PARLIO_LED_STRIP_CLK_HZ  2500000UL  // 2.5 MHz → 400 ns per PARLIO sample
#define PARLIO_SAMPLES_PER_BIT   3          // PARLIO clock cycles per LED data bit
#define PARLIO_BIT0_PATTERN      0x04       // 0b100 : 1 high, 2 low
#define PARLIO_BIT1_PATTERN      0x06       // 0b110 : 2 high, 1 low
#define PARLIO_RESET_BYTES       125        // 400 µs low — satisfies all supported LED types

typedef struct {
    uint32_t      clk_hz;           /* PARLIO output clock frequency (Hz) */
    uint8_t       samples_per_bit;  /* PARLIO clock cycles per LED protocol data bit */
    uint8_t       bit0_pattern;     /* Bit pattern for LED data 0 (MSB = first sample out) */
    uint8_t       bit1_pattern;     /* Bit pattern for LED data 1 (MSB = first sample out) */
    color_order_t order;            /* LED colour order (shared with RMT led_params) */
} parlio_led_params_t;

static const parlio_led_params_t parlio_led_params[] = {
    [ LED_STRIP_WS2812     ] = { PARLIO_LED_STRIP_CLK_HZ, PARLIO_SAMPLES_PER_BIT, PARLIO_BIT0_PATTERN, PARLIO_BIT1_PATTERN, ORDER_GRB },
    [ LED_STRIP_WS2812_RGB ] = { PARLIO_LED_STRIP_CLK_HZ, PARLIO_SAMPLES_PER_BIT, PARLIO_BIT0_PATTERN, PARLIO_BIT1_PATTERN, ORDER_RGB },
    [ LED_STRIP_SK6812     ] = { PARLIO_LED_STRIP_CLK_HZ, PARLIO_SAMPLES_PER_BIT, PARLIO_BIT0_PATTERN, PARLIO_BIT1_PATTERN, ORDER_GRB },
    [ LED_STRIP_APA106     ] = { PARLIO_LED_STRIP_CLK_HZ, PARLIO_SAMPLES_PER_BIT, PARLIO_BIT0_PATTERN, PARLIO_BIT1_PATTERN, ORDER_RGB },
    [ LED_STRIP_SM16703    ] = { PARLIO_LED_STRIP_CLK_HZ, PARLIO_SAMPLES_PER_BIT, PARLIO_BIT0_PATTERN, PARLIO_BIT1_PATTERN, ORDER_RGB },
};

#endif /* SOC_PARLIO_SUPPORTED */

#endif /* __LL_LED_TIMINGS_H__ */

//  --- EOF --- //
