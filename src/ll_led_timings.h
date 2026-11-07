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
    .led_1 = sk6812_one,
    .led_reset = sk6812_reset,
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
    "sm16703",
    "custom"
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

// Return the active timing/colour-order parameters for a strip
static inline const led_params_t &ll_strip_params( const led_strip_t *strip ) {
    return ( strip->type == LED_STRIP_CUSTOM ) ? strip->params : led_params[ strip->type ];
}

// Convert a time to RMT ticks, rounded to nearest
static inline uint64_t ll_ns_to_ticks( uint64_t ns, uint32_t res_hz ) {
    return ( ns * res_hz + 500000000ULL ) / 1000000000ULL;
}

#define LL_RMT_MAX_DURATION 32767   /* 15-bit duration field */

// Build RMT symbols from a custom timing. Returns ESP_ERR_INVALID_ARG if any
// time rounds to zero ticks or exceeds the 15-bit RMT duration field.
static inline esp_err_t ll_build_custom_params( const ll_custom_timing_t &t, color_order_t order, led_params_t &out ) {
    if ( t.resolution_hz == 0 || order >= ORDER_MAX ) {
        return ESP_ERR_INVALID_ARG;
    }
    uint64_t t0h = ll_ns_to_ticks( t.t0h_ns, t.resolution_hz );
    uint64_t t0l = ll_ns_to_ticks( t.t0l_ns, t.resolution_hz );
    uint64_t t1h = ll_ns_to_ticks( t.t1h_ns, t.resolution_hz );
    uint64_t t1l = ll_ns_to_ticks( t.t1l_ns, t.resolution_hz );
    // reset is sent as one symbol of two equal low halves
    uint64_t rst = ll_ns_to_ticks( ( uint64_t )t.reset_us * 1000ULL, t.resolution_hz ) / 2;
    uint64_t v[] = { t0h, t0l, t1h, t1l, rst };
    for ( uint64_t x : v ) {
        if ( x < 1 || x > LL_RMT_MAX_DURATION ) {
            return ESP_ERR_INVALID_ARG;
        }
    }
    out.led_0     = { ( uint16_t )t0h, 1, ( uint16_t )t0l, 0 };
    out.led_1     = { ( uint16_t )t1h, 1, ( uint16_t )t1l, 0 };
    out.led_reset = { ( uint16_t )rst, 0, ( uint16_t )rst, 0 };
    out.order     = order;
    return ESP_OK;
}

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

#define PARLIO_LED_STRIP_CLK_HZ  2500000UL  // 2.5 MHz → 400 ns per PARLIO clock
#define PARLIO_SAMPLES_PER_BIT   3          // PARLIO clock cycles per LED data bit
// data_width=8: each DMA byte = 1 PARLIO clock = 400 ns.
// Each LED data bit expands to (PARLIO_SAMPLES_PER_BIT * 8) = 24 DMA bytes.
// Reset: 1000 zero-bytes × 400 ns = 400 µs LOW, satisfies all supported LED types.
#define PARLIO_BYTES_PER_BIT     ( PARLIO_SAMPLES_PER_BIT * 8 )   // 24 DMA bytes per LED bit
#define PARLIO_RESET_BYTES       1000

static const parlio_led_params_t parlio_led_params[] = {
    [ LED_STRIP_WS2812     ] = { PARLIO_LED_STRIP_CLK_HZ, 1, 2, 2, 1, PARLIO_SAMPLES_PER_BIT, PARLIO_RESET_BYTES, ORDER_GRB },
    [ LED_STRIP_WS2812_RGB ] = { PARLIO_LED_STRIP_CLK_HZ, 1, 2, 2, 1, PARLIO_SAMPLES_PER_BIT, PARLIO_RESET_BYTES, ORDER_RGB },
    [ LED_STRIP_SK6812     ] = { PARLIO_LED_STRIP_CLK_HZ, 1, 2, 2, 1, PARLIO_SAMPLES_PER_BIT, PARLIO_RESET_BYTES, ORDER_GRB },
    [ LED_STRIP_APA106     ] = { PARLIO_LED_STRIP_CLK_HZ, 1, 2, 2, 1, PARLIO_SAMPLES_PER_BIT, PARLIO_RESET_BYTES, ORDER_RGB },
    [ LED_STRIP_SM16703    ] = { PARLIO_LED_STRIP_CLK_HZ, 1, 2, 2, 1, PARLIO_SAMPLES_PER_BIT, PARLIO_RESET_BYTES, ORDER_RGB },
};

// Return the active PARLIO parameters for a strip
static inline const parlio_led_params_t &ll_pio_params( const led_strip_t *strip ) {
    return ( strip->type == LED_STRIP_CUSTOM ) ? strip->pio : parlio_led_params[ strip->type ];
}

// Candidate PARLIO clocks, lowest (smallest DMA buffer) first. All divide the PARLIO source clocks.
static const uint32_t ll_pio_candidate_clk_hz[] = { 2500000, 4000000, 5000000, 8000000, 10000000, 16000000, 20000000, 40000000 };
#define LL_PIO_MAX_SAMPLES_PER_BIT 64

// Choose the lowest PARLIO clock at which every phase rounds to within tol_ns of the request.
// Returns ESP_ERR_INVALID_ARG if none does. A shorter bit period is padded low to match the longer.
static inline esp_err_t ll_build_parlio_params( const ll_custom_timing_t &t, color_order_t order, parlio_led_params_t &out ) {
    if ( order >= ORDER_MAX ) {
        return ESP_ERR_INVALID_ARG;
    }
    const uint32_t req[ 4 ] = { t.t0h_ns, t.t0l_ns, t.t1h_ns, t.t1l_ns };
    for ( uint32_t f : ll_pio_candidate_clk_hz ) {
        uint32_t n[ 4 ];
        bool ok = true;
        for ( int i = 0; i < 4 && ok; i++ ) {
            n[ i ] = ( uint32_t )ll_ns_to_ticks( req[ i ], f );
            int64_t actual_ns = ( int64_t )n[ i ] * 1000000000LL / f;
            int64_t err = actual_ns - ( int64_t )req[ i ];
            ok = n[ i ] >= 1 && ( err < 0 ? -err : err ) <= ( int64_t )t.parlio_tolerance_ns;
        }
        uint32_t spb = ok ? ( ( n[ 0 ] + n[ 1 ] ) > ( n[ 2 ] + n[ 3 ] ) ? n[ 0 ] + n[ 1 ] : n[ 2 ] + n[ 3 ] ) : 0;
        if ( !ok || spb > LL_PIO_MAX_SAMPLES_PER_BIT ) {
            continue;
        }
        uint64_t rst = ll_ns_to_ticks( ( uint64_t )t.reset_us * 1000ULL, f );
        if ( rst < 1 ) {
            return ESP_ERR_INVALID_ARG;
        }
        out.clk_hz = f;
        out.n0h = n[ 0 ]; out.n0l = n[ 1 ]; out.n1h = n[ 2 ]; out.n1l = n[ 3 ];
        out.samples_per_bit = spb;
        out.reset_samples = ( uint32_t )rst;
        out.order = order;
        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}

#endif /* SOC_PARLIO_SUPPORTED */

#endif /* __LL_LED_TIMINGS_H__ */

//  --- EOF --- //
