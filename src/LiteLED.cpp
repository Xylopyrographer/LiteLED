/*
 * RMT-based ESP-IDF driver for WS2812B/SK6812/APA106/SM16703 LED strips
 *  - MIT Licensed as described in the file LICENSE
 */

#include <hal/rmt_types.h>
#include <cstddef>
#include <hal/gpio_types.h>
#include <Arduino.h>
#include "LiteLED.h"
#include "llrmt.h"

// constructor
LiteLED::LiteLED( led_strip_type_t led_type, bool rgbw, rmt_channel_t channel) {
    // populate the LED strip type structure
    theStrip.type = led_type;
    theStrip.is_rgbw = rgbw;
    theStrip.channel = channel;
    theStrip.brightness = 255;
    theStrip.buf = NULL;
}

esp_err_t LiteLED::begin( uint8_t data_pin, size_t length, bool auto_w ) {
    // add the data pin & length to the structure
    theStrip.length = length;
    theStrip.gpio = (gpio_num_t)data_pin;
    theStrip.auto_w = auto_w;
    led_strip_install();
    return led_strip_init(&theStrip);
}

esp_err_t LiteLED::show() {
    return led_strip_flush( &theStrip );
}

esp_err_t LiteLED::setPixel( size_t num, rgb_t color, bool show ) {
    esp_err_t _res = led_strip_set_pixel( &theStrip, num, color );
    if ( _res != ESP_OK )
        return _res;
    if ( show )
        _res = led_strip_flush( &theStrip );
    return _res;
}

esp_err_t LiteLED::setPixel( size_t num, crgb_t color, bool show ) {
    esp_err_t _res = led_strip_set_pixel( &theStrip, num, rgb_from_code( color ) );
    if ( _res != ESP_OK )
        return _res;
    if ( show )
        _res = led_strip_flush( &theStrip );
    return _res;
}

esp_err_t LiteLED::setPixels( size_t start, size_t len, rgb_t *data, bool show ) {
    esp_err_t _res = led_strip_set_pixels( &theStrip, (size_t)0, (size_t)theStrip.length, data );
    if ( _res != ESP_OK )
        return _res;
    if ( show )
        _res = led_strip_flush( &theStrip );
    return _res;
}

esp_err_t LiteLED::setPixels( size_t start, size_t len, crgb_t* data, bool show ) {
    return led_strip_set_pixels_c( &theStrip, start, len, data );
}

esp_err_t LiteLED::fill( rgb_t color, bool show ) {
    esp_err_t _res = led_strip_fill( &theStrip, (size_t)0, (size_t)theStrip.length, color );
    if ( _res != ESP_OK )
        return _res;
    if ( show )
        _res = led_strip_flush( &theStrip );
    return _res;
}

esp_err_t LiteLED::fill( crgb_t color, bool show ) {
    return LiteLED::fill( rgb_from_code( color ), show );
}

esp_err_t LiteLED::clear( bool show ) {
    esp_err_t _res = led_strip_fill( &theStrip, (size_t)0, (size_t)theStrip.length, rgb_from_code( 0x000000 ) );
    if ( _res != ESP_OK )
        return _res;
    if ( show )
        _res = led_strip_flush( &theStrip );
    return _res;
}

esp_err_t LiteLED::brightness( uint8_t bright, bool show ) {
    theStrip.brightness = bright;
    if (show)
        return led_strip_flush( &theStrip );
    else
        return ESP_OK;
}


//  --- EOF ---
