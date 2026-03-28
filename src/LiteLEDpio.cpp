//
/*
    LiteLEDpio — PARLIO-backed LED strip driver
    Public API implementation.
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
// Peripheral Manager bus type for PARLIO TX.
// Fall back to GPIO type if the specific PARLIO type is not defined in
// this version of arduino-esp32.
// -------------------------------------------------------------------------
#ifdef ESP32_BUS_TYPE_PARLIO_TX
    #define LL_PARLIO_BUS_TYPE  ESP32_BUS_TYPE_PARLIO_TX
#else
    #define LL_PARLIO_BUS_TYPE  ESP32_BUS_TYPE_GPIO
#endif

// -------------------------------------------------------------------------
// Constructor / Destructor
// -------------------------------------------------------------------------
LiteLEDpio::LiteLEDpio( led_strip_type_t led_type, bool rgbw ) {
    theStrip.type       = led_type;
    theStrip.is_rgbw    = rgbw;
    theStrip.brightness = 255;
    theStrip.bright_act = 255;
    theStrip.buf        = NULL;
    theStrip.use_psram  = false;
    parlioCfg.parlio_chan      = NULL;
    parlioCfg.parlio_buf       = NULL;
    parlioCfg.parlio_buf_bytes = 0;
    valid_instance = false;
}

LiteLEDpio::~LiteLEDpio() {
    LiteLEDpio::free();
}

// -------------------------------------------------------------------------
// begin() — simple form
// -------------------------------------------------------------------------
esp_err_t LiteLEDpio::begin( uint8_t data_pin, size_t length, bool auto_w ) {
    if ( valid_instance ) {
        log_d( "LiteLEDpio: already initialized, cleaning up first" );
        free();
    }

    theStrip.gpio      = ( gpio_num_t )data_pin;
    theStrip.length    = length;
    theStrip.auto_w    = auto_w;
    theStrip.use_psram = false;

    if ( !perimanPinIsValid( data_pin ) ) {
        log_d( "LiteLEDpio: GPIO %u is not valid", data_pin );
        return ESP_ERR_INVALID_ARG;
    }
    peripheral_bus_type_t cur = perimanGetPinBusType( data_pin );
    if ( cur != ESP32_BUS_TYPE_INIT ) {
        log_d( "LiteLEDpio: GPIO %u already in use by %s",
               data_pin, perimanGetTypeName( cur ) );
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t res = parlio_strip_init( &theStrip, &parlioCfg );
    if ( res != ESP_OK ) {
        return res;
    }

    res = parlio_strip_install( &theStrip, &parlioCfg );
    if ( res != ESP_OK ) {
        return res;
    }

    if ( !perimanSetPinBus( data_pin, LL_PARLIO_BUS_TYPE,
                            ( void * )parlioCfg.parlio_chan, -1, -1 ) ) {
        log_d( "LiteLEDpio: Peripheral Manager registration failed for GPIO %u", data_pin );
        parlio_strip_free( &theStrip, &parlioCfg );
        return ESP_ERR_INVALID_STATE;
    }
    perimanSetPinBusExtraType( data_pin, "LiteLEDpio" );

    valid_instance = true;
    return ESP_OK;
}

// -------------------------------------------------------------------------
// begin() — with PSRAM option
// -------------------------------------------------------------------------
esp_err_t LiteLEDpio::begin( uint8_t data_pin, size_t length,
                             ll_psram_t psram_flag, bool auto_w ) {
    if ( valid_instance ) {
        log_d( "LiteLEDpio: already initialized, cleaning up first" );
        free();
    }

    theStrip.gpio   = ( gpio_num_t )data_pin;
    theStrip.length = length;
    theStrip.auto_w = auto_w;

    if ( psram_flag == PSRAM_AUTO ) {
        #if CONFIG_SPIRAM
        theStrip.use_psram = psramFound();
        #else
        theStrip.use_psram = false;
        #endif
    }
    else {
        theStrip.use_psram = ( psram_flag == PSRAM_ENABLE );
    }

    if ( !perimanPinIsValid( data_pin ) ) {
        log_d( "LiteLEDpio: GPIO %u is not valid", data_pin );
        return ESP_ERR_INVALID_ARG;
    }
    peripheral_bus_type_t cur = perimanGetPinBusType( data_pin );
    if ( cur != ESP32_BUS_TYPE_INIT ) {
        log_d( "LiteLEDpio: GPIO %u already in use by %s",
               data_pin, perimanGetTypeName( cur ) );
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t res = parlio_strip_init( &theStrip, &parlioCfg );
    if ( res != ESP_OK ) {
        return res;
    }

    res = parlio_strip_install( &theStrip, &parlioCfg );
    if ( res != ESP_OK ) {
        return res;
    }

    if ( !perimanSetPinBus( data_pin, LL_PARLIO_BUS_TYPE,
                            ( void * )parlioCfg.parlio_chan, -1, -1 ) ) {
        log_d( "LiteLEDpio: Peripheral Manager registration failed for GPIO %u", data_pin );
        parlio_strip_free( &theStrip, &parlioCfg );
        return ESP_ERR_INVALID_STATE;
    }
    perimanSetPinBusExtraType( data_pin, "LiteLEDpio" );

    valid_instance = true;
    return ESP_OK;
}

// -------------------------------------------------------------------------
// show() — encode buffer → DMA bitstream → transmit
// -------------------------------------------------------------------------
esp_err_t LiteLEDpio::show() {
    if ( !isValid() ) {
        log_d( "LiteLEDpio::show(): instance is no longer valid" );
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t res = parlio_strip_flush( &theStrip, &parlioCfg );
    if ( res == ESP_OK ) {
        theStrip.bright_act = theStrip.brightness;
    }
    return res;
}

// -------------------------------------------------------------------------
// Pixel operations — delegate to the shared ll_strip_pixels layer
// -------------------------------------------------------------------------
esp_err_t LiteLEDpio::setPixel( size_t num, rgb_t color, bool show ) {
    esp_err_t res = ll_checkPinState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_set_pixel( &theStrip, num, color ) ) != ESP_OK ) {
        return res;
    }
    return show ? LiteLEDpio::show() : ESP_OK;
}

esp_err_t LiteLEDpio::setPixel( size_t num, crgb_t color, bool show ) {
    esp_err_t res = ll_checkPinState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_set_pixel( &theStrip, num, rgb_from_code( color ) ) ) != ESP_OK ) {
        return res;
    }
    return show ? LiteLEDpio::show() : ESP_OK;
}

esp_err_t LiteLEDpio::setPixels( size_t start, size_t len, rgb_t *data, bool show ) {
    esp_err_t res = ll_checkPinState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_set_pixels( &theStrip, start, len, data ) ) != ESP_OK ) {
        return res;
    }
    return show ? LiteLEDpio::show() : ESP_OK;
}

esp_err_t LiteLEDpio::setPixels( size_t start, size_t len, crgb_t *data, bool show ) {
    esp_err_t res = ll_checkPinState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_set_pixels_c( &theStrip, start, len, data ) ) != ESP_OK ) {
        return res;
    }
    return show ? LiteLEDpio::show() : ESP_OK;
}

esp_err_t LiteLEDpio::fill( rgb_t color, bool show ) {
    esp_err_t res = ll_checkPinState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_fill( &theStrip, color ) ) != ESP_OK ) {
        return res;
    }
    return show ? LiteLEDpio::show() : ESP_OK;
}

esp_err_t LiteLEDpio::fill( crgb_t color, bool show ) {
    return LiteLEDpio::fill( rgb_from_code( color ), show );
}

esp_err_t LiteLEDpio::clear( bool show ) {
    esp_err_t res = ll_checkPinState();
    if ( res != ESP_OK ) {
        return res;
    }
    size_t n = theStrip.length * ( 3 + ( theStrip.is_rgbw != 0 ) );
    led_strip_clear( &theStrip, n );
    return show ? LiteLEDpio::show() : ESP_OK;
}

esp_err_t LiteLEDpio::brightness( uint8_t bright, bool show ) {
    esp_err_t res = ll_checkPinState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_set_brightness( &theStrip, bright ) ) != ESP_OK ) {
        return res;
    }
    return show ? LiteLEDpio::show() : ESP_OK;
}

uint8_t LiteLEDpio::getBrightness() {
    return led_strip_get_brightness( &theStrip );
}

rgb_t LiteLEDpio::getPixel( size_t num ) {
    return led_strip_get_pixel( &theStrip, num );
}

crgb_t LiteLEDpio::getPixelC( size_t num ) {
    return rgb_to_code( led_strip_get_pixel( &theStrip, num ) );
}

esp_err_t LiteLEDpio::fillRandom( bool show ) {
    esp_err_t res = ll_checkPinState();
    if ( res != ESP_OK ) {
        return res;
    }
    if ( ( res = led_strip_fill_random( &theStrip ) ) != ESP_OK ) {
        return res;
    }
    return show ? LiteLEDpio::show() : ESP_OK;
}

esp_err_t LiteLEDpio::setOrder( color_order_t led_order ) {
    led_strip_set_color_order( &theStrip, led_order );
    return ESP_OK;
}

esp_err_t LiteLEDpio::resetOrder() {
    led_strip_set_default_color_order( &theStrip );
    return ESP_OK;
}

// -------------------------------------------------------------------------
// isValid / static helpers
// -------------------------------------------------------------------------
bool LiteLEDpio::isValid() const {
    if ( !valid_instance ) {
        return false;
    }
    return ( theStrip.buf != NULL && parlioCfg.parlio_chan != NULL );
}

bool LiteLEDpio::isGpioAvailable( uint8_t gpio_pin ) {
    if ( !perimanPinIsValid( gpio_pin ) ) {
        return false;
    }
    return ( perimanGetPinBusType( gpio_pin ) == ESP32_BUS_TYPE_INIT );
}

uint8_t LiteLEDpio::getActiveInstanceCount() {
    // Reuse the RMT registry count; PARLIO instances are tracked separately
    // via Peripheral Manager. This is a best-effort count.
    return 0;  // TODO: add a dedicated PARLIO instance counter if needed
}

// -------------------------------------------------------------------------
// free() — internal cleanup
// -------------------------------------------------------------------------
esp_err_t LiteLEDpio::free() {
    if ( !parlioCfg.parlio_chan && !theStrip.buf ) {
        return ESP_ERR_INVALID_ARG;  // never initialized
    }

    valid_instance = false;

    // Unregister from Peripheral Manager before freeing hardware
    if ( theStrip.gpio < GPIO_NUM_MAX ) {
        if ( !perimanSetPinBus( theStrip.gpio, ESP32_BUS_TYPE_INIT, NULL, -1, -1 ) ) {
            log_d( "LiteLEDpio: failed to unregister GPIO %u from Peripheral Manager",
                   theStrip.gpio );
        }
    }

    return parlio_strip_free( &theStrip, &parlioCfg );
}

#endif /* SOC_PARLIO_SUPPORTED */

//  --- EOF --- //
