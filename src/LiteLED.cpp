#include "hal/rmt_types.h"
/*
 * @file LiteLED.cpp
 *
 * RMT-based ESP-IDF driver for WS2812B/SK6812/APA106/SM16703 LED strips
 *  - Copyright (c) 2020 Ruslan V. Uss <unclerus@gmail.com>
 *  - Copyright (c) 2023 Xylopyrographer <xylopyrographer@gmail.com>
 *  - MIT Licensed as described in the file LICENSE
 */
#include <cstddef>
#include "hal/gpio_types.h"
#include <Arduino.h>
#include "LiteLED.h"
#include "llrmt.h"

static const char *TAG = "LiteLED";

// Constructor1
LiteLED::LiteLED( led_strip_type_t led_type, bool rgbw ) {
    // populate the LED strip type structure
    theStrip.type = led_type;
    theStrip.is_rgbw = rgbw;
    theStrip.channel = RMT_CHANNEL_0;
    #ifdef LED_STRIP_BRIGHTNESS
        theStrip.brightness = 255;     // < Brightness 0..255, call ::show() after change.                                   
    #endif
    theStrip.buf = NULL;
}

// Constructor2
LiteLED::LiteLED( led_strip_type_t led_type, bool rgbw, rmt_channel_t channel) {
    // populate the LED strip type structure
    theStrip.type = led_type;
    theStrip.is_rgbw = rgbw;
    theStrip.channel = channel;
    #ifdef LED_STRIP_BRIGHTNESS
        theStrip.brightness = 255;     // < Brightness 0..255, call ::show() after change.                                   
    #endif
    theStrip.buf = NULL;
}

void LiteLED::led_strip_install() {
    float ratio = (float)APB_CLK_FREQ / LED_STRIP_RMT_CLK_DIV / 1e09f;

    for (size_t i = 0; i < LED_STRIP_TYPE_MAX; i++) {
        // 0 bit
        rmt_items[i].bit0.duration0 = (uint32_t)(ratio * led_params[i].t0h);
        rmt_items[i].bit0.level0 = 1;
        rmt_items[i].bit0.duration1 = (uint32_t)(ratio * led_params[i].t0l);
        rmt_items[i].bit0.level1 = 0;
        // 1 bit
        rmt_items[i].bit1.duration0 = (uint32_t)(ratio * led_params[i].t1h);
        rmt_items[i].bit1.level0 = 1;
        rmt_items[i].bit1.duration1 = (uint32_t)(ratio * led_params[i].t1l);
        rmt_items[i].bit1.level1 = 0;
    }
    return;
}

esp_err_t LiteLED::led_strip_init(led_strip_t *strip) {  
    CHECK_ARG(strip && strip->length > 0 && strip->type < LED_STRIP_TYPE_MAX);
    strip->buf = (uint8_t*) calloc(strip->length, COLOR_SIZE(strip) );
    // strip.buf = (uint8_t*) calloc(strip->length, COLOR_SIZE(strip) );
    if (!strip->buf) {
        ESP_LOGE(TAG, "Cannot init strip - insufficient memory.");
        return ESP_ERR_NO_MEM;
    }

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(strip->gpio, strip->channel);
    // rmt_config_t config = RMT_DEFAULT_CONFIG_TX(strip.gpio, strip.channel);
    config.clk_div = LED_STRIP_RMT_CLK_DIV;

    CHECK(rmt_config(&config));
    CHECK(rmt_driver_install(config.channel, 0, 0));

    CHECK(rmt_translator_init(config.channel, led_params[strip->type].adapter));
    #ifdef LED_STRIP_BRIGHTNESS
        // No support for translator context prior to ESP-IDF 4.3
        CHECK(rmt_translator_set_context(config.channel, strip));
    #endif

    return ESP_OK;
}

esp_err_t LiteLED::begin( uint8_t data_pin, size_t length ) {
    // add the data pin & length to the structure
    theStrip.length = length;
    theStrip.gpio = (gpio_num_t)data_pin;
    LiteLED::led_strip_install();
    return LiteLED::led_strip_init(&theStrip);
}

// esp_err_t LiteLED::led_strip_free(led_strip_t *strip) {
//     CHECK_ARG(strip && strip->buf);
//     free(strip->buf);
//     CHECK(rmt_driver_uninstall(strip->channel));
//     return ESP_OK;
// }

// esp_err_t LiteLED::free() {
//      return led_strip_free( &theStrip );
// }

esp_err_t LiteLED::led_strip_flush(led_strip_t *strip) {
    CHECK_ARG(strip && strip->buf);
    CHECK(rmt_wait_tx_done(strip->channel, pdMS_TO_TICKS(LED_STRIP_FLUSH_TIMEOUT)));
    ets_delay_us(LED_STRIP_PAUSE_LENGTH);
    return rmt_write_sample(strip->channel, strip->buf,
                            strip->length * COLOR_SIZE(strip), false);
}

bool LiteLED::led_strip_busy(led_strip_t *strip) {
    if (!strip) return false;
    return rmt_wait_tx_done(strip->channel, 0) == ESP_ERR_TIMEOUT;
}

esp_err_t LiteLED::led_strip_wait(led_strip_t *strip, TickType_t timeout) {
    CHECK_ARG(strip);
    return rmt_wait_tx_done( strip->channel, pdMS_TO_TICKS( timeout ) );
}

esp_err_t LiteLED::show() {
    return LiteLED::led_strip_flush( &theStrip );
}

esp_err_t LiteLED::led_strip_set_pixel(led_strip_t *strip, size_t num, rgb_t color) {
    CHECK_ARG(strip && strip->buf && num <= strip->length);
    size_t idx = num * COLOR_SIZE(strip);
    switch (led_params[strip->type].order) {
        case ORDER_GRB:
            strip->buf[idx] = color.g;
            strip->buf[idx + 1] = color.r;
            strip->buf[idx + 2] = color.b;
            if (strip->is_rgbw)
                strip->buf[idx + 3] = rgb_luma(color);
            break;
        case ORDER_RGB:
            strip->buf[idx] = color.r;
            strip->buf[idx + 1] = color.g;
            strip->buf[idx + 2] = color.b;
            if (strip->is_rgbw)
                strip->buf[idx + 3] = rgb_luma(color);
            break;
    }
    return ESP_OK;
}

esp_err_t LiteLED::setPixel( size_t num, rgb_t color ) {
    return LiteLED::led_strip_set_pixel( &theStrip, num, color );
}

esp_err_t LiteLED::led_strip_set_pixels(led_strip_t *strip, size_t start, size_t len, rgb_t *data)
{
    CHECK_ARG(strip && strip->buf && len && start + len <= strip->length);
    for (size_t i = 0; i < len; i++)
        CHECK(LiteLED::led_strip_set_pixel(strip, i + start, data[i]));
    return ESP_OK;
}

esp_err_t LiteLED::setPixels( size_t start, size_t len, rgb_t *data) {
    return LiteLED::led_strip_set_pixels( &theStrip, (size_t)0, (size_t)theStrip.length, data );
}

esp_err_t LiteLED::led_strip_fill(led_strip_t *strip, size_t start, size_t len, rgb_t color)
{
    CHECK_ARG(strip && strip->buf && len && start + len <= strip->length);

    for (size_t i = start; i < start + len; i++)
        CHECK(LiteLED::led_strip_set_pixel(strip, i, color));
    return ESP_OK;
}


esp_err_t LiteLED::fill( rgb_t color ) {
    return LiteLED::led_strip_fill( &theStrip, (size_t)0, (size_t)theStrip.length, color );
}

esp_err_t LiteLED::clear() {
    esp_err_t _res = LiteLED::led_strip_fill( &theStrip, (size_t)0, (size_t)theStrip.length, rgb_from_code( 0x000000 ) );
    if (_res != ESP_OK )
        return _res;
    return LiteLED::led_strip_flush(&theStrip);
}

esp_err_t LiteLED::brightness( uint8_t bright ) {
    #ifdef LED_STRIP_BRIGHTNESS
        theStrip.brightness = bright;
        return LiteLED::led_strip_flush(&theStrip);
    #else
        return ESP_ERR_INVALID_ARG;
    #endif
}


//  --- EOF ---
