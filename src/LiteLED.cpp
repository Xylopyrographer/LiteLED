//
/*
    ESP32 RMT-based driver for various types of RGB LED strips
    - MIT Licensed as described in the file LICENSE
*/

#include <Arduino.h>
#include "driver/rmt_tx.h"
#include "LiteLED.h"
#include "llrmt.h"

// constructor
LiteLED::LiteLED( led_strip_type_t led_type, bool rgbw ) {
    // populate the LED strip type structure
    theStrip.type = led_type;
    theStrip.is_rgbw = rgbw;
    theStrip.brightness = 255;
    theStrip.bright_act = 255;
    theStrip.buf = NULL;
}

// destructor
LiteLED::~LiteLED() {
    // delete all resources used by the strip
    LiteLED::free();
}

esp_err_t LiteLED::begin( uint8_t data_pin, size_t length, bool auto_w ) {
    /* add the data pin & length to the structure */
    theStrip.gpio = ( gpio_num_t )data_pin;
    theStrip.length = length;
    theStrip.auto_w = auto_w;
    esp_err_t res = led_strip_init( &theStrip );
    if ( res != ESP_OK ) {
        log_e( "Error in 'begin()'. Cannot initialize strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    res = led_strip_install( &theStrip );
    if ( res != ESP_OK ) {
        log_e( "Error in 'begin()'. Cannot install strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    return ESP_OK;
}

esp_err_t LiteLED::begin( uint8_t data_pin, size_t length, ll_dma_t dma_flag, ll_priority_t priority, bool auto_w ) {
    /* add the data pin & length to the structure */
    theStrip.gpio = ( gpio_num_t )data_pin;
    theStrip.length = length;
    theStrip.auto_w = auto_w;
    esp_err_t res = led_strip_init( &theStrip );
    if ( res != ESP_OK ) {
        log_e( "Error in 'begin()'. Cannot initialize strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    res = led_strip_init_modify( &theStrip, dma_flag, priority );
    if ( res != ESP_OK ) {
        log_e( "Error in 'begin()'. Issue setting strip DMA or interrupt priority . Result = %s", esp_err_to_name( res ) );
        return res;
    }
    res = led_strip_install( &theStrip );
    if ( res != ESP_OK ) {
        log_e( "Error in 'begin()'. Cannot install strip. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    return ESP_OK;
}

esp_err_t LiteLED::show() {
    esp_err_t _res = led_strip_flush( &theStrip );
    if ( _res != ESP_OK ) {
        log_e( "Error in 'show()'. Cannot flush strip. Result = %s", esp_err_to_name( _res ) );
        return _res;
    }
    theStrip.bright_act = theStrip.brightness;
    return _res;
}

esp_err_t LiteLED::setPixel( size_t num, rgb_t color, bool show ) {
    esp_err_t _res = ESP_OK;
    if ( ( _res = led_strip_set_pixel( &theStrip, num,  color ) ) != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::setPixel( size_t num, crgb_t color, bool show ) {
    esp_err_t _res = ESP_OK;
    if ( ( _res = led_strip_set_pixel( &theStrip, num, rgb_from_code( color ) ) ) != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::setPixels( size_t start, size_t len, rgb_t* data, bool show ) {
    esp_err_t _res = led_strip_set_pixels( &theStrip, start, len, data );
    if ( _res != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::setPixels( size_t start, size_t len, crgb_t* data, bool show ) {
    esp_err_t _res = led_strip_set_pixels_c( &theStrip, start, len, data );
    if ( _res != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::fill( rgb_t color, bool show ) {
    esp_err_t _res = led_strip_fill( &theStrip, color );
    if ( _res != ESP_OK ) {
        return _res;
    }
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::fill( crgb_t color, bool show ) {;
    return LiteLED::fill( rgb_from_code( color ), show );
}

esp_err_t LiteLED::clear( bool show ) {   
    esp_err_t _res = ESP_OK;
    led_strip_clear( &theStrip, PIXEL_SIZE( &theStrip ) );
    if ( show ) {
        _res = LiteLED::show();
    }
    return _res;
}

esp_err_t LiteLED::brightness( uint8_t bright, bool show ) {
    esp_err_t _res = led_strip_set_brightness( &theStrip, bright );
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
    esp_err_t res = ESP_OK;
    if ( (res = led_strip_fill_random( &theStrip ) ) != ESP_OK ) {
        return res;
    }
    if ( show ) {
        res = led_strip_flush( &theStrip );
    }
    return res;
}

esp_err_t LiteLED::setOrder( color_order_t led_order) {
    led_strip_set_color_order( &theStrip, led_order );
    return ESP_OK;
}

esp_err_t LiteLED::resetOrder() {
    led_strip_set_default_color_order( &theStrip );
    return ESP_OK;
}

esp_err_t LiteLED::free() {
    return led_strip_free( &theStrip );
}

//  --- EOF ---
