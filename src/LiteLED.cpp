//
/*
    ESP32 RMT-based driver for various types of RGB LED strips
*/

#include <Arduino.h>
#include "driver/rmt_tx.h"
#include "LiteLED.h"
#include "llrmt.h"
#include "ll_registry.h"

// Static flag to print capability info only once
static bool ll_capability_logged = false;

// Log hardware capabilities once during first initialization
static void ll_log_capabilities() {
    if ( !ll_capability_logged ) {
        ll_capability_logged = true;
#if !LL_DMA_SUPPORT
        log_d( "LiteLED: RMT DMA not supported on this ESP32 model (will use non-DMA mode)" );
#endif
#if !LL_INT_PRIORITY_SUPPORT
        log_d( "LiteLED: RMT interrupt priority setting not supported in this core version (will use default priority)" );
#endif
    }
}

// constructor
LiteLED::LiteLED( led_strip_type_t led_type, bool rgbw ) {
    // populate the LED strip type structure
    theStrip.type = led_type;
    theStrip.is_rgbw = rgbw;
    theStrip.brightness = 255;
    theStrip.bright_act = 255;
    theStrip.buf = NULL;
    theStrip.use_psram = false;  // Default to internal RAM
    valid_instance = false;      // Not valid until begin() succeeds
}

// destructor
LiteLED::~LiteLED() {
    // delete all resources used by the strip
    LiteLED::free();
}

esp_err_t LiteLED::begin( uint8_t data_pin, size_t length, bool auto_w ) {
    /* add the data pin & length to the structure */
    ll_log_capabilities();  // Log hardware capabilities once

    // If already initialized, clean up first to prevent resource leaks
    if ( valid_instance ) {
        log_d( "LiteLED: Instance already initialized, cleaning up before reinitializing" );
        free();
    }

    theStrip.gpio = ( gpio_num_t )data_pin;
    theStrip.length = length;
    theStrip.auto_w = auto_w;
    theStrip.use_psram = false;  // Default to internal RAM for simple begin()

    // Check if pin is valid for Peripheral Manager
    if ( !perimanPinIsValid( data_pin ) ) {
        log_d( "LiteLED: GPIO %u is not valid", data_pin );
        return ESP_ERR_INVALID_ARG;
    }

    // Check current pin usage
    peripheral_bus_type_t current_type = perimanGetPinBusType( data_pin );
    if ( current_type != ESP32_BUS_TYPE_INIT ) {
        const char *current_usage = perimanGetTypeName( current_type );
        log_d( "LiteLED: GPIO %u is already in use by %s", data_pin, current_usage );
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t res = led_strip_init( &theStrip );
    if ( res != ESP_OK ) {
        log_d( "Failed to initialize strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    res = led_strip_install( &theStrip );
    if ( res != ESP_OK ) {
        log_d( "Failed to install strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }

    // Register channel -> instance mapping BEFORE registering with Peripheral Manager
    // This ensures the deinit callback can find the instance if triggered
    res = ll_register_channel_instance( theStrip.stripCfg.led_chan, this );
    if ( res != ESP_OK ) {
        log_d( "LiteLED: Failed to register channel instance" );
        led_strip_free( &theStrip );
        return res;
    }

    // Register with Peripheral Manager
    if ( !perimanSetPinBus( data_pin, ESP32_BUS_TYPE_RMT_TX, ( void * )theStrip.stripCfg.led_chan, -1, -1 ) ) {
        log_d( "LiteLED: Failed to register GPIO %u with Peripheral Manager", data_pin );
        ll_unregister_channel_instance( theStrip.stripCfg.led_chan );
        led_strip_free( &theStrip );
        return ESP_ERR_INVALID_STATE;
    }

    // Set extra type identifier
    perimanSetPinBusExtraType( data_pin, "LiteLED" );

    valid_instance = true;  // Mark instance as valid
    return ESP_OK;
}

esp_err_t LiteLED::begin( uint8_t data_pin, size_t length, ll_psram_t psram_flag, bool auto_w ) {
    /* add the data pin & length to the structure */
    ll_log_capabilities();  // Log hardware capabilities once

    // If already initialized, clean up first to prevent resource leaks
    if ( valid_instance ) {
        log_d( "LiteLED: Instance already initialized, cleaning up before reinitializing" );
        free();
    }

    theStrip.gpio = ( gpio_num_t )data_pin;
    theStrip.length = length;
    theStrip.auto_w = auto_w;

    // Determine PSRAM usage
    if ( psram_flag == PSRAM_AUTO ) {
        // Auto mode: use PSRAM if available
#if CONFIG_SPIRAM
        theStrip.use_psram = psramFound();
#else
        theStrip.use_psram = false;
#endif
    }
    else {
        theStrip.use_psram = ( psram_flag == PSRAM_ENABLE );
    }

    // Check if pin is valid for Peripheral Manager
    if ( !perimanPinIsValid( data_pin ) ) {
        log_d( "LiteLED: GPIO %u is not valid", data_pin );
        return ESP_ERR_INVALID_ARG;
    }

    // Check current pin usage
    peripheral_bus_type_t current_type = perimanGetPinBusType( data_pin );
    if ( current_type != ESP32_BUS_TYPE_INIT ) {
        const char *current_usage = perimanGetTypeName( current_type );
        log_d( "LiteLED: GPIO %u is already in use by %s", data_pin, current_usage );
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t res = led_strip_init( &theStrip );
    if ( res != ESP_OK ) {
        log_d( "Failed to initialize strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }

    res = led_strip_install( &theStrip );
    if ( res != ESP_OK ) {
        log_d( "Failed to install strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }

    // Register channel -> instance mapping BEFORE registering with Peripheral Manager
    // This ensures the deinit callback can find the instance if triggered
    res = ll_register_channel_instance( theStrip.stripCfg.led_chan, this );
    if ( res != ESP_OK ) {
        log_d( "LiteLED: Failed to register channel instance" );
        led_strip_free( &theStrip );
        return res;
    }

    // Register with Peripheral Manager
    if ( !perimanSetPinBus( data_pin, ESP32_BUS_TYPE_RMT_TX, ( void * )theStrip.stripCfg.led_chan, -1, -1 ) ) {
        log_d( "LiteLED: Failed to register GPIO %u with Peripheral Manager", data_pin );
        ll_unregister_channel_instance( theStrip.stripCfg.led_chan );
        led_strip_free( &theStrip );
        return ESP_ERR_INVALID_STATE;
    }

    // Set extra type identifier
    perimanSetPinBusExtraType( data_pin, "LiteLED" );

    valid_instance = true;  // Mark instance as valid
    return ESP_OK;
}

esp_err_t LiteLED::begin( uint8_t data_pin, size_t length, ll_dma_t dma_flag, ll_priority_t priority, ll_psram_t psram_flag, bool auto_w ) {
    /* add the data pin & length to the structure */
    ll_log_capabilities();  // Log hardware capabilities once

    // If already initialized, clean up first to prevent resource leaks
    if ( valid_instance ) {
        log_d( "LiteLED: Instance already initialized, cleaning up before reinitializing" );
        free();
    }

    theStrip.gpio = ( gpio_num_t )data_pin;
    theStrip.length = length;
    theStrip.auto_w = auto_w;

    // Determine PSRAM usage
    if ( psram_flag == PSRAM_AUTO ) {
        // Auto mode: use PSRAM if available
#if CONFIG_SPIRAM
        theStrip.use_psram = psramFound();
#else
        theStrip.use_psram = false;
#endif
    }
    else {
        theStrip.use_psram = ( psram_flag == PSRAM_ENABLE );
    }

    // Check if pin is valid for Peripheral Manager
    if ( !perimanPinIsValid( data_pin ) ) {
        log_d( "LiteLED: GPIO %u is not valid", data_pin );
        return ESP_ERR_INVALID_ARG;
    }

    // Check current pin usage
    peripheral_bus_type_t current_type = perimanGetPinBusType( data_pin );
    if ( current_type != ESP32_BUS_TYPE_INIT ) {
        const char *current_usage = perimanGetTypeName( current_type );
        log_d( "LiteLED: GPIO %u is already in use by %s", data_pin, current_usage );
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t res = led_strip_init( &theStrip );
    if ( res != ESP_OK ) {
        log_d( "Failed to initialize strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    res = led_strip_init_modify( &theStrip, dma_flag, priority );
    if ( res != ESP_OK ) {
        log_d( "Failed to set strip DMA or interrupt priority. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    res = led_strip_install( &theStrip );
    if ( res != ESP_OK ) {
        log_d( "Failed to install strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }

    // Register channel -> instance mapping BEFORE registering with Peripheral Manager
    // This ensures the deinit callback can find the instance if triggered
    res = ll_register_channel_instance( theStrip.stripCfg.led_chan, this );
    if ( res != ESP_OK ) {
        log_d( "LiteLED: Failed to register channel instance" );
        led_strip_free( &theStrip );
        return res;
    }

    // Register with Peripheral Manager
    if ( !perimanSetPinBus( data_pin, ESP32_BUS_TYPE_RMT_TX, ( void * )theStrip.stripCfg.led_chan, -1, -1 ) ) {
        log_d( "LiteLED: Failed to register GPIO %u with Peripheral Manager", data_pin );
        ll_unregister_channel_instance( theStrip.stripCfg.led_chan );
        led_strip_free( &theStrip );
        return ESP_ERR_INVALID_STATE;
    }

    // Set extra type identifier
    perimanSetPinBusExtraType( data_pin, "LiteLED" );

    valid_instance = true;  // Mark instance as valid
    return ESP_OK;
}

esp_err_t LiteLED::show() {
    // Check if instance is still valid (in case of forced cleanup)
    if ( !isValid() ) {
        log_d( "LiteLED: Instance is no longer valid (pin may have been reassigned)" );
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t _res = led_strip_flush( &theStrip );
    if ( _res != ESP_OK ) {
        log_d( "Error in 'show()'. Cannot flush strip. Result = %s", esp_err_to_name( _res ) );
        return _res;
    }
    theStrip.bright_act = theStrip.brightness;
    return _res;
}

esp_err_t LiteLED::setPixel( size_t num, rgb_t color, bool show ) {
    esp_err_t _res = ll_checkPinState();
    if ( _res != ESP_OK ) {
        return _res;
    }

    if ( ( _res = led_strip_set_pixel( &theStrip, num,  color ) ) != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::setPixel( size_t num, crgb_t color, bool show ) {
    esp_err_t _res = ll_checkPinState();
    if ( _res != ESP_OK ) {
        return _res;
    }

    if ( ( _res = led_strip_set_pixel( &theStrip, num, rgb_from_code( color ) ) ) != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::setPixels( size_t start, size_t len, rgb_t* data, bool show ) {
    esp_err_t _res = ll_checkPinState();
    if ( _res != ESP_OK ) {
        return _res;
    }

    _res = led_strip_set_pixels( &theStrip, start, len, data );
    if ( _res != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::setPixels( size_t start, size_t len, crgb_t* data, bool show ) {
    esp_err_t _res = ll_checkPinState();
    if ( _res != ESP_OK ) {
        return _res;
    }

    _res = led_strip_set_pixels_c( &theStrip, start, len, data );
    if ( _res != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::fill( rgb_t color, bool show ) {
    esp_err_t _res = ll_checkPinState();
    if ( _res != ESP_OK ) {
        return _res;
    }

    _res = led_strip_fill( &theStrip, color );
    if ( _res != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::fill( crgb_t color, bool show ) {
    return LiteLED::fill( rgb_from_code( color ), show );
}

esp_err_t LiteLED::clear( bool show ) {
    esp_err_t _res = ll_checkPinState();
    if ( _res != ESP_OK ) {
        return _res;
    }

    led_strip_clear( &theStrip, PIXEL_SIZE( &theStrip ) );
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::brightness( uint8_t bright, bool show ) {
    esp_err_t _res = ll_checkPinState();
    if ( _res != ESP_OK ) {
        return _res;
    }

    _res = led_strip_set_brightness( &theStrip, bright );
    if ( _res != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

uint8_t LiteLED::getBrightness( ) {
    return led_strip_get_brightness( &theStrip );
}

rgb_t LiteLED::getPixel( size_t num ) {
    rgb_t _res = led_strip_get_pixel( &theStrip, num );
    return _res;
}

crgb_t LiteLED::getPixelC( size_t num ) {
    crgb_t _res = rgb_to_code( led_strip_get_pixel( &theStrip, num ) );
    return _res;
}

esp_err_t LiteLED::fillRandom( bool show ) {
    esp_err_t _res = ll_checkPinState();
    if ( _res != ESP_OK ) {
        return _res;
    }

    _res = led_strip_fill_random( &theStrip );
    if ( _res != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = led_strip_flush( &theStrip );
    }
    return _res;
}

esp_err_t LiteLED::setOrder( color_order_t led_order ) {
    led_strip_set_color_order( &theStrip, led_order );
    return ESP_OK;
}

esp_err_t LiteLED::resetOrder() {
    led_strip_set_default_color_order( &theStrip );
    return ESP_OK;
}

bool LiteLED::isValid() const {
    // Fast check using flag - for external callers who want detailed validation
    if ( !valid_instance ) {
        return false;
    }
    // Secondary validation for robustness
    return ( theStrip.buf != NULL && theStrip.stripCfg.led_chan != NULL );
}

bool LiteLED::isGpioAvailable( uint8_t gpio_pin ) {
    if ( !perimanPinIsValid( gpio_pin ) ) {
        return false;
    }
    peripheral_bus_type_t current_type = perimanGetPinBusType( gpio_pin );
    return ( current_type == ESP32_BUS_TYPE_INIT );
}

uint8_t LiteLED::getActiveInstanceCount() {
    return ll_registry_get_active_count();
}

esp_err_t LiteLED::free() {
    if ( !theStrip.buf ) {
        return ESP_ERR_INVALID_ARG;
    }

    // Mark instance as invalid immediately
    valid_instance = false;

    // Unregister channel -> instance mapping
    ll_unregister_channel_instance( theStrip.stripCfg.led_chan );

    // Unregister from Peripheral Manager
    if ( theStrip.gpio < GPIO_NUM_MAX ) { // Check if GPIO is valid
        if ( !perimanSetPinBus( theStrip.gpio, ESP32_BUS_TYPE_INIT, NULL, -1, -1 ) ) {
            log_d( "LiteLED: Failed to unregister GPIO %u from Peripheral Manager", theStrip.gpio );
        }
    }

    // Proceed with normal cleanup
    return led_strip_free( &theStrip );
}

//  --- EOF ---
