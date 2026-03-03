//
/*
    LiteLED PARLIO Core Operations — Implementation
*/

#include "ll_parlio_core.h"

#if SOC_PARLIO_SUPPORTED

#include "llrgb.h"
#include "ll_led_timings.h"
#include <string.h>

// -------------------------------------------------------------------------
// Peripheral Manager PARLIO bus type
// arduino-esp32 may not yet define ESP32_BUS_TYPE_PARLIO_TX on all
// versions; fall back to GPIO type so the pin is still marked in-use.
// -------------------------------------------------------------------------
#ifdef ESP32_BUS_TYPE_PARLIO_TX
    #define LL_PARLIO_BUS_TYPE  ESP32_BUS_TYPE_PARLIO_TX
#else
    #define LL_PARLIO_BUS_TYPE  ESP32_BUS_TYPE_GPIO
#endif

// Utility macros — identical names to ll_strip_core.h so they read naturally
#define PIO_COLOR_SIZE( strip ) ( 3 + ( (strip)->is_rgbw != 0 ) )
#define PIO_PIXEL_SIZE( strip ) ( PIO_COLOR_SIZE( strip ) * (strip)->length )

// -------------------------------------------------------------------------
// Internal: encode one LED colour byte to PARLIO_SAMPLES_PER_BIT output bytes.
//
// Each of the 8 input bits is expanded to 3 PARLIO clock cycles packed MSB-
// first.  For 3 SPB the output is always exactly 3 bytes:
//
//   bit value 0  →  0b100  (sample pattern: HIGH LOW  LOW )
//   bit value 1  →  0b110  (sample pattern: HIGH HIGH LOW )
//
// Example (input 0xFF, all bits 1):
//   24-bit stream:  110 110 110 110 110 110 110 110
//   output bytes:   0xDB  0x6D  0xB6
// -------------------------------------------------------------------------
static IRAM_ATTR void parlio_encode_byte( uint8_t b,
        uint8_t bit0_pat,
        uint8_t bit1_pat,
        uint8_t *out ) {
    uint32_t expanded = 0;
    for ( int i = 7; i >= 0; i-- ) {
        expanded = ( expanded << PARLIO_SAMPLES_PER_BIT )
                   | ( ( ( b >> i ) & 1 ) ? ( uint32_t )bit1_pat
                       : ( uint32_t )bit0_pat );
    }
    out[ 0 ] = ( expanded >> 16 ) & 0xFF;
    out[ 1 ] = ( expanded >>  8 ) & 0xFF;
    out[ 2 ] =   expanded         & 0xFF;
}

// -------------------------------------------------------------------------
esp_err_t parlio_strip_init( led_strip_t *strip, parlio_strip_cfg_t *cfg ) {
    if ( !( strip && cfg && strip->length > 0 && strip->type < LED_STRIP_TYPE_MAX ) ) {
        log_d( "parlio_strip_init: invalid arguments" );
        return ESP_ERR_INVALID_ARG;
    }
    cfg->parlio_chan      = NULL;
    cfg->parlio_buf       = NULL;
    cfg->parlio_buf_bytes = 0;
    log_d( "parlio_strip_init: OK" );
    return ESP_OK;
}

// -------------------------------------------------------------------------
esp_err_t parlio_strip_install( led_strip_t *strip, parlio_strip_cfg_t *cfg ) {
    // ---- allocate pixel colour buffer (obeys PSRAM preference) -----------
    size_t pixel_bytes = strip->length * PIO_COLOR_SIZE( strip );

    if ( strip->use_psram ) {
        #if CONFIG_SPIRAM
        if ( psramFound() ) {
            strip->buf = ( uint8_t* )heap_caps_calloc( strip->length,
                         PIO_COLOR_SIZE( strip ),
                         MALLOC_CAP_SPIRAM );
            if ( strip->buf ) {
                log_d( "PARLIO pixel buffer in PSRAM (%u bytes)", pixel_bytes );
            }
            else {
                log_d( "PSRAM alloc failed, falling back to internal RAM" );
            }
        }
        #endif
        if ( !strip->buf ) {
            strip->buf = ( uint8_t* )calloc( strip->length, PIO_COLOR_SIZE( strip ) );
            if ( strip->buf ) {
                log_d( "PARLIO pixel buffer in internal RAM (%u bytes)", pixel_bytes );
            }
        }
    }
    else {
        strip->buf = ( uint8_t* )calloc( strip->length, PIO_COLOR_SIZE( strip ) );
        if ( strip->buf ) {
            log_d( "PARLIO pixel buffer in internal RAM (%u bytes)", pixel_bytes );
        }
    }

    if ( !strip->buf ) {
        log_d( "parlio_strip_install: failed to allocate pixel buffer" );
        return ESP_ERR_NO_MEM;
    }

    // ---- allocate DMA bitstream buffer (must be internal DMA-capable RAM) -
    const parlio_led_params_t *p = &parlio_led_params[ strip->type ];
    size_t encoded_bytes  = pixel_bytes   * p->samples_per_bit;  // 3× pixel data
    size_t total_bytes    = encoded_bytes  + PARLIO_RESET_BYTES;  // + 400 µs LOW
    total_bytes           = ( total_bytes + 3 ) & ~( size_t )3;  // 4-byte aligned

    cfg->parlio_buf = ( uint8_t* )heap_caps_calloc( 1, total_bytes,
                      MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL );
    if ( !cfg->parlio_buf ) {
        log_d( "parlio_strip_install: failed to allocate DMA bitstream buffer (%u bytes)", total_bytes );
        free( strip->buf );
        strip->buf = NULL;
        return ESP_ERR_NO_MEM;
    }
    cfg->parlio_buf_bytes = total_bytes;
    log_d( "PARLIO DMA buffer: %u bytes (%u encoded + %u reset, aligned)",
           total_bytes, encoded_bytes, PARLIO_RESET_BYTES );

    // ---- create PARLIO TX unit -------------------------------------------
    parlio_tx_unit_config_t chan_cfg = {};
    chan_cfg.clk_src             = PARLIO_CLK_SRC_DEFAULT;
    chan_cfg.data_width           = 1;
    chan_cfg.clk_in_gpio_num      = GPIO_NUM_NC;  // no external clock input
    chan_cfg.clk_out_gpio_num     = GPIO_NUM_NC;  // no clock output (clockless protocol)
    for ( int i = 0; i < PARLIO_TX_UNIT_MAX_DATA_WIDTH; i++ ) {
        chan_cfg.data_gpio_nums[ i ] = GPIO_NUM_NC;  // not connected
    }
    chan_cfg.data_gpio_nums[ 0 ]  = ( gpio_num_t )strip->gpio;
    chan_cfg.output_clk_freq_hz   = p->clk_hz;
    chan_cfg.trans_queue_depth    = 4;
    chan_cfg.max_transfer_size    = total_bytes;
    // idle_value is now set per-transmit in parlio_transmit_config_t
    chan_cfg.flags.clk_gate_en    = false;

    esp_err_t res = parlio_new_tx_unit( &chan_cfg, &cfg->parlio_chan );
    if ( res != ESP_OK ) {
        log_d( "parlio_strip_install: parlio_new_tx_unit() failed - %s", esp_err_to_name( res ) );
        heap_caps_free( cfg->parlio_buf );
        cfg->parlio_buf = NULL;
        free( strip->buf );
        strip->buf = NULL;
        return res;
    }

    if ( ( res = parlio_tx_unit_enable( cfg->parlio_chan ) ) != ESP_OK ) {
        log_d( "parlio_strip_install: parlio_tx_unit_enable() failed - %s", esp_err_to_name( res ) );
        parlio_del_tx_unit( cfg->parlio_chan );
        cfg->parlio_chan = NULL;
        heap_caps_free( cfg->parlio_buf );
        cfg->parlio_buf = NULL;
        free( strip->buf );
        strip->buf = NULL;
        return res;
    }

    log_d( "parlio_strip_install: PARLIO TX channel ready on GPIO %u at %.2f MHz",
           strip->gpio, p->clk_hz / 1e6f );

    #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
    parlio_strip_debug_dump( strip, cfg );
    #endif

    return ESP_OK;
}

// -------------------------------------------------------------------------
esp_err_t parlio_strip_free( led_strip_t *strip, parlio_strip_cfg_t *cfg ) {
    if ( !cfg || !cfg->parlio_chan ) {
        log_d( "parlio_strip_free: called on uninitialized config" );
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t res;

    // Wait for any in-progress DMA transfer (including reset period)
    if ( ( res = parlio_tx_unit_wait_all_done( cfg->parlio_chan, -1 ) ) != ESP_OK ) {
        log_d( "parlio_strip_free: wait_all_done failed - %s", esp_err_to_name( res ) );
        return res;
    }
    if ( ( res = parlio_tx_unit_disable( cfg->parlio_chan ) ) != ESP_OK ) {
        log_d( "parlio_strip_free: disable failed - %s", esp_err_to_name( res ) );
        return res;
    }
    if ( ( res = parlio_del_tx_unit( cfg->parlio_chan ) ) != ESP_OK ) {
        log_d( "parlio_strip_free: delete failed - %s", esp_err_to_name( res ) );
        return res;
    }
    cfg->parlio_chan = NULL;

    if ( cfg->parlio_buf ) {
        heap_caps_free( cfg->parlio_buf );
        cfg->parlio_buf       = NULL;
        cfg->parlio_buf_bytes = 0;
    }
    if ( strip && strip->buf ) {
        free( strip->buf );
        strip->buf = NULL;
    }
    return ESP_OK;
}

// -------------------------------------------------------------------------
esp_err_t parlio_strip_flush( led_strip_t *strip, parlio_strip_cfg_t *cfg ) {
    if ( !( strip && strip->buf && cfg && cfg->parlio_chan && cfg->parlio_buf ) ) {
        log_d( "parlio_strip_flush: called on uninitialized strip or config" );
        return ESP_ERR_INVALID_ARG;
    }

    const parlio_led_params_t *p     = &parlio_led_params[ strip->type ];
    const size_t               pixel_bytes = strip->length * PIO_COLOR_SIZE( strip );
    const uint8_t              brightness  = strip->brightness;
    uint8_t                   *out         = cfg->parlio_buf;

    // Encode each pixel colour byte → 3 PARLIO bytes, with brightness scaling.
    // The reset region (trailing PARLIO_RESET_BYTES bytes = 0x00) is never
    // written here; it was zeroed by calloc and stays zero across calls.
    for ( size_t i = 0; i < pixel_bytes; i++ ) {
        parlio_encode_byte( scale8_video( strip->buf[ i ], brightness ),
                            p->bit0_pattern, p->bit1_pattern,
                            &out[ i * p->samples_per_bit ] );
    }

    // Transmit bitstream (pixel data + reset) via DMA.
    // total_bytes includes PARLIO_RESET_BYTES of trailing zeros.
    parlio_transmit_config_t tx_cfg = { .idle_value = 0 };
    esp_err_t res = parlio_tx_unit_transmit( cfg->parlio_chan,
                    cfg->parlio_buf,
                    cfg->parlio_buf_bytes * 8,
                    &tx_cfg );
    if ( res != ESP_OK ) {
        log_d( "parlio_strip_flush: transmit failed - %s", esp_err_to_name( res ) );
        return res;
    }

    // Block until the full frame (including the reset period) is done.
    if ( ( res = parlio_tx_unit_wait_all_done( cfg->parlio_chan, -1 ) ) != ESP_OK ) {
        log_d( "parlio_strip_flush: wait_all_done failed - %s", esp_err_to_name( res ) );
    }
    return res;
}

// -------------------------------------------------------------------------
void parlio_strip_debug_dump( led_strip_t *strip, parlio_strip_cfg_t *cfg ) {
    #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
    if ( strip && cfg ) {
        const parlio_led_params_t *p = &parlio_led_params[ strip->type ];
        log_printf( "\n" );
        log_printf( "========= LiteLED PARLIO Debug Report =========\n" );
        log_printf( "led_strip_t at: %p\n", strip );
        log_printf( "    type: %u, is_rgbw: %d, auto_w: %d\n",
                    strip->type, strip->is_rgbw, strip->auto_w );
        log_printf( "    length: %u, gpio: %u\n", strip->length, strip->gpio );
        log_printf( "    brightness: %u, bright_act: %u\n",
                    strip->brightness, strip->bright_act );
        log_printf( "    pixel buf: %p (%u bytes)\n",
                    strip->buf, PIO_PIXEL_SIZE( strip ) );
        log_printf( "parlio_strip_cfg_t at: %p\n", cfg );
        log_printf( "    parlio_chan: %p\n",  cfg->parlio_chan );
        log_printf( "    parlio_buf:  %p (%u bytes)\n",
                    cfg->parlio_buf, cfg->parlio_buf_bytes );
        log_printf( "parlio_led_params:\n" );
        log_printf( "    clk_hz:          %lu\n",  p->clk_hz );
        log_printf( "    samples_per_bit: %u\n",   p->samples_per_bit );
        log_printf( "    bit0_pattern:    0x%02X\n", p->bit0_pattern );
        log_printf( "    bit1_pattern:    0x%02X\n", p->bit1_pattern );
        log_printf( "    PARLIO_RESET_BYTES: %u (%.1f us)\n",
                    PARLIO_RESET_BYTES,
                    ( float )PARLIO_RESET_BYTES * 8.0f / ( p->clk_hz / 1e6f ) );
        log_printf( "================================================\n" );
        log_printf( "\n" );
    }
    else {
        log_printf( "parlio_strip_debug_dump: NULL pointer\n" );
    }
    #endif
}

#endif /* SOC_PARLIO_SUPPORTED */

//  --- EOF --- //
