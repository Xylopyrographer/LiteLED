//
/*
    LiteLEDpioGroup — multi-strip PARLIO driver
    LiteLEDpioLane  — per-lane pixel-buffer handle

    LiteLEDpioGroup owns one PARLIO TX unit and one shared DMA bitstream
    buffer.  Each strip is registered as a bit-lane (0..data_width-1) via
    addStrip().  Every show() call encodes ALL lane pixel buffers in a single
    DMA transfer, guaranteeing perfectly synchronised output.

    LiteLEDpioLane is a thin reference class.  Its pixel methods operate on
    the pixel colour buffer for one lane; show() delegates to the parent
    LiteLEDpioGroup so all lanes always transmit together.
*/

#include <Arduino.h>
#include "LiteLED.h"
#include "ll_strip_pixels.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "esp32-hal-periman.h"
#ifdef __cplusplus
}
#endif

#ifndef SOC_PARLIO_SUPPORTED
    #define SOC_PARLIO_SUPPORTED 0
#endif

#if SOC_PARLIO_SUPPORTED
#include "llparlio.h"

// -------------------------------------------------------------------------
// Peripheral Manager bus type
// -------------------------------------------------------------------------
#ifdef ESP32_BUS_TYPE_PARLIO_TX
    #define LL_PARLIO_BUS_TYPE  ESP32_BUS_TYPE_PARLIO_TX
#else
    #define LL_PARLIO_BUS_TYPE  ESP32_BUS_TYPE_GPIO
#endif

// ==========================================================================
// LiteLEDpioLane — implementation
// ==========================================================================

esp_err_t LiteLEDpioLane::show() {
    if ( !_group ) {
        return ESP_ERR_INVALID_STATE;
    }
    return _group->show();
}

esp_err_t LiteLEDpioLane::setPixel( size_t num, rgb_t color, bool doShow ) {
    esp_err_t res = _checkState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_set_pixel( _strip, num, color ) ) != ESP_OK ) {
        return res;
    }
    return doShow ? show() : ESP_OK;
}

esp_err_t LiteLEDpioLane::setPixel( size_t num, crgb_t color, bool doShow ) {
    return setPixel( num, rgb_from_code( color ), doShow );
}

esp_err_t LiteLEDpioLane::setPixels( size_t start, size_t len, rgb_t *data, bool doShow ) {
    esp_err_t res = _checkState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_set_pixels( _strip, start, len, data ) ) != ESP_OK ) {
        return res;
    }
    return doShow ? show() : ESP_OK;
}

esp_err_t LiteLEDpioLane::setPixels( size_t start, size_t len, crgb_t *data, bool doShow ) {
    esp_err_t res = _checkState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_set_pixels_c( _strip, start, len, data ) ) != ESP_OK ) {
        return res;
    }
    return doShow ? show() : ESP_OK;
}

esp_err_t LiteLEDpioLane::fill( rgb_t color, bool doShow ) {
    esp_err_t res = _checkState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_fill( _strip, color ) ) != ESP_OK ) {
        return res;
    }
    return doShow ? show() : ESP_OK;
}

esp_err_t LiteLEDpioLane::fill( crgb_t color, bool doShow ) {
    return fill( rgb_from_code( color ), doShow );
}

esp_err_t LiteLEDpioLane::clear( bool doShow ) {
    esp_err_t res = _checkState();
    if ( res != ESP_OK ) {
        return res;
    }
    size_t n = _strip->length * ( 3 + ( _strip->is_rgbw ? 1 : 0 ) );
    led_strip_clear( _strip, n );
    return doShow ? show() : ESP_OK;
}

esp_err_t LiteLEDpioLane::brightness( uint8_t bright, bool doShow ) {
    esp_err_t res = _checkState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_set_brightness( _strip, bright ) ) != ESP_OK ) {
        return res;
    }
    return doShow ? show() : ESP_OK;
}

uint8_t LiteLEDpioLane::getBrightness() {
    if ( !_strip ) {
        return 0;
    }
    return led_strip_get_brightness( _strip );
}

rgb_t LiteLEDpioLane::getPixel( size_t num ) {
    if ( !_strip ) {
        return { 0, 0, 0 };
    }
    return led_strip_get_pixel( _strip, num );
}

crgb_t LiteLEDpioLane::getPixelC( size_t num ) {
    return rgb_to_code( getPixel( num ) );
}

esp_err_t LiteLEDpioLane::fillRandom( bool doShow ) {
    esp_err_t res = _checkState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_fill_random( _strip ) ) != ESP_OK ) {
        return res;
    }
    return doShow ? show() : ESP_OK;
}

esp_err_t LiteLEDpioLane::setOrder( color_order_t led_order ) {
    esp_err_t res = _checkState();
    if ( res != ESP_OK ) {
        return res;
    }
    led_strip_set_color_order( _strip, led_order );
    return ESP_OK;
}

esp_err_t LiteLEDpioLane::resetOrder() {
    esp_err_t res = _checkState();
    if ( res != ESP_OK ) {
        return res;
    }
    led_strip_set_default_color_order( _strip );
    return ESP_OK;
}

// ==========================================================================
// LiteLEDpioGroup — implementation
// ==========================================================================

LiteLEDpioGroup::LiteLEDpioGroup( led_strip_type_t led_type, size_t length, bool rgbw )
    : _type( led_type ),
      _length( length ),
      _is_rgbw( rgbw ),
      _brightness( 255 ),
      _valid( false ) {

    // Zero the group config struct.
    memset( &_groupCfg, 0, sizeof( _groupCfg ) );

    // All lane objects start as null (no strip, no group pointer) until
    // _addStrip() initialises them.
}

LiteLEDpioGroup::~LiteLEDpioGroup() {
    _free();
}

// -------------------------------------------------------------------------
// _addStrip — internal: assign gpio to a specific bit lane
// -------------------------------------------------------------------------
LiteLEDpioLane &LiteLEDpioGroup::_addStrip( uint8_t lane_idx, uint8_t gpio ) {
    if ( _valid ) {
        log_e( "LiteLEDpioGroup::addStrip: cannot add strips after begin() — ignored" );
        return _null_lane;
    }
    if ( lane_idx >= PARLIO_TX_UNIT_MAX_DATA_WIDTH ) {
        // Should not be reachable from the template (caught at compile time),
        // but guards the non-template sequential path.
        log_e( "LiteLEDpioGroup::addStrip: lane %u >= max (%d) — ignored",
               lane_idx, PARLIO_TX_UNIT_MAX_DATA_WIDTH );
        return _null_lane;
    }
    if ( _groupCfg.lanes[ lane_idx ].assigned ) {
        log_e( "LiteLEDpioGroup::addStrip: lane %u already assigned — ignored", lane_idx );
        return _null_lane;
    }

    // Populate this lane's led_strip_t with shared parameters.
    led_strip_t &strip = _groupCfg.lanes[ lane_idx ].strip;
    strip.type         = _type;
    strip.length       = _length;
    strip.is_rgbw      = _is_rgbw;
    strip.auto_w       = true;
    strip.gpio         = ( gpio_num_t )gpio;
    strip.brightness   = _brightness;
    strip.bright_act   = _brightness;
    strip.buf          = NULL;
    strip.use_psram    = false;

    _groupCfg.lanes[ lane_idx ].assigned = true;
    _groupCfg.lane_count++;

    // Init the lane handle to point at this strip and back to this group.
    _lanes[ lane_idx ]._init( &strip, this );

    log_d( "LiteLEDpioGroup::addStrip: lane %u → GPIO %u", lane_idx, gpio );
    return _lanes[ lane_idx ];
}

// -------------------------------------------------------------------------
// addStrip — sequential: assigns the next available lane
// -------------------------------------------------------------------------
LiteLEDpioLane &LiteLEDpioGroup::addStrip( uint8_t gpio ) {
    for ( uint8_t n = 0; n < PARLIO_TX_UNIT_MAX_DATA_WIDTH; n++ ) {
        if ( !_groupCfg.lanes[ n ].assigned ) {
            return _addStrip( n, gpio );
        }
    }
    log_e( "LiteLEDpioGroup::addStrip: all %d lanes assigned — ignored",
           PARLIO_TX_UNIT_MAX_DATA_WIDTH );
    return _null_lane;
}

// -------------------------------------------------------------------------
// begin
// -------------------------------------------------------------------------
esp_err_t LiteLEDpioGroup::begin( ll_psram_t psram_flag ) {
    if ( _valid ) {
        log_d( "LiteLEDpioGroup::begin: already initialized" );
        return ESP_ERR_INVALID_STATE;
    }
    if ( _groupCfg.lane_count == 0 ) {
        log_e( "LiteLEDpioGroup::begin: no strips added — call addStrip() first" );
        return ESP_ERR_INVALID_STATE;
    }

    // Resolve PSRAM preference and apply to all assigned lanes.
    bool use_psram = false;
    if ( psram_flag == PSRAM_AUTO ) {
        #if CONFIG_SPIRAM
        use_psram = psramFound();
        #endif
    }
    else {
        use_psram = ( psram_flag == PSRAM_ENABLE );
    }
    for ( uint8_t n = 0; n < PARLIO_TX_UNIT_MAX_DATA_WIDTH; n++ ) {
        if ( _groupCfg.lanes[ n ].assigned ) {
            _groupCfg.lanes[ n ].strip.use_psram = use_psram;
        }
    }

    // Validate all assigned GPIO pins.
    for ( uint8_t n = 0; n < PARLIO_TX_UNIT_MAX_DATA_WIDTH; n++ ) {
        if ( !_groupCfg.lanes[ n ].assigned ) {
            continue;
        }
        uint8_t gpio = _groupCfg.lanes[ n ].strip.gpio;
        if ( !perimanPinIsValid( gpio ) ) {
            log_e( "LiteLEDpioGroup::begin: GPIO %u (lane %u) is not valid", gpio, n );
            return ESP_ERR_INVALID_ARG;
        }
        if ( perimanGetPinBusType( gpio ) != ESP32_BUS_TYPE_INIT ) {
            log_e( "LiteLEDpioGroup::begin: GPIO %u (lane %u) already in use by %s",
                   gpio, n, perimanGetTypeName( perimanGetPinBusType( gpio ) ) );
            return ESP_ERR_INVALID_STATE;
        }
    }

    esp_err_t res = parlio_group_install( &_groupCfg );
    if ( res != ESP_OK ) {
        return res;
    }

    // Register all GPIOs with the Peripheral Manager.
    for ( uint8_t n = 0; n < PARLIO_TX_UNIT_MAX_DATA_WIDTH; n++ ) {
        if ( !_groupCfg.lanes[ n ].assigned ) {
            continue;
        }
        uint8_t gpio = _groupCfg.lanes[ n ].strip.gpio;
        if ( !perimanSetPinBus( gpio, LL_PARLIO_BUS_TYPE,
                                ( void * )_groupCfg.parlio_chan, -1, -1 ) ) {
            log_d( "LiteLEDpioGroup::begin: Peripheral Manager registration failed for GPIO %u", gpio );
            // Unregister any already-registered pins and roll back hardware.
            for ( uint8_t k = 0; k < n; k++ ) {
                if ( _groupCfg.lanes[ k ].assigned ) {
                    perimanSetPinBus( _groupCfg.lanes[ k ].strip.gpio,
                                      ESP32_BUS_TYPE_INIT, NULL, -1, -1 );
                }
            }
            parlio_group_free( &_groupCfg );
            return ESP_ERR_INVALID_STATE;
        }
        perimanSetPinBusExtraType( gpio, "LiteLEDpioGroup" );
    }

    _valid = true;
    return ESP_OK;
}

// -------------------------------------------------------------------------
// show
// -------------------------------------------------------------------------
esp_err_t LiteLEDpioGroup::show() {
    if ( !_valid ) {
        log_d( "LiteLEDpioGroup::show: not initialized" );
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t res = parlio_group_flush( &_groupCfg );
    if ( res == ESP_OK ) {
        // Sync bright_act for all lanes.
        for ( uint8_t n = 0; n < PARLIO_TX_UNIT_MAX_DATA_WIDTH; n++ ) {
            if ( _groupCfg.lanes[ n ].assigned ) {
                _groupCfg.lanes[ n ].strip.bright_act =
                    _groupCfg.lanes[ n ].strip.brightness;
            }
        }
    }
    return res;
}

// -------------------------------------------------------------------------
// brightness — sets all lanes to the same value
// -------------------------------------------------------------------------
esp_err_t LiteLEDpioGroup::brightness( uint8_t bright, bool doShow ) {
    _brightness = bright;
    for ( uint8_t n = 0; n < PARLIO_TX_UNIT_MAX_DATA_WIDTH; n++ ) {
        if ( _groupCfg.lanes[ n ].assigned ) {
            led_strip_set_brightness( &_groupCfg.lanes[ n ].strip, bright );
        }
    }
    return doShow ? show() : ESP_OK;
}

uint8_t LiteLEDpioGroup::getBrightness() {
    return _brightness;
}

// -------------------------------------------------------------------------
// operator[] — access lane by index
// -------------------------------------------------------------------------
LiteLEDpioLane &LiteLEDpioGroup::operator[]( uint8_t lane ) {
    if ( lane >= PARLIO_TX_UNIT_MAX_DATA_WIDTH || !_groupCfg.lanes[ lane ].assigned ) {
        log_d( "LiteLEDpioGroup[]: lane %u not assigned — returning null lane", lane );
        return _null_lane;
    }
    return _lanes[ lane ];
}

// -------------------------------------------------------------------------
// _free — internal cleanup
// -------------------------------------------------------------------------
esp_err_t LiteLEDpioGroup::_free() {
    if ( !_groupCfg.parlio_chan ) {
        return ESP_OK;  // never initialized or already freed
    }

    _valid = false;

    // Unregister all GPIOs from Peripheral Manager.
    for ( uint8_t n = 0; n < PARLIO_TX_UNIT_MAX_DATA_WIDTH; n++ ) {
        if ( _groupCfg.lanes[ n ].assigned &&
                _groupCfg.lanes[ n ].strip.gpio < GPIO_NUM_MAX ) {
            perimanSetPinBus( _groupCfg.lanes[ n ].strip.gpio,
                              ESP32_BUS_TYPE_INIT, NULL, -1, -1 );
        }
    }

    return parlio_group_free( &_groupCfg );
}

#endif /* SOC_PARLIO_SUPPORTED */

//  --- EOF --- //
