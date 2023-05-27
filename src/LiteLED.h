/*
 * RMT-based ESP-IDF driver for WS2812B/SK6812/APA106/SM16703 LED strips
 *
 *  Copyright (c) 2020 Ruslan V. Uss <unclerus@gmail.com>
 *  Copyright (c) 2023 Xylopyrographer <xylopyrographer@gmail.com>
 *  MIT Licensed as described in the file LICENSE
*/

#include "hal/rmt_types.h"

#pragma once
 
#ifndef __LITELED_H__
#define __LITELED_H__

// ??? Move the next 3 lines to LiteLED.cpp ???
#include <driver/gpio.h>
#include <esp_err.h>
#include <driver/rmt.h>
#include "llrgb.h"

#define LED_STRIP_FLUSH_TIMEOUT  1000
#define LED_STRIP_PAUSE_LENGTH   50
#define LED_STRIP_SHOW_TIMEOUT   1000

// enum class led_strip_type_t {
//     LED_STRIP_WS2812 = 0,
//     LED_STRIP_SK6812,
//     LED_STRIP_APA106,
//     LED_STRIP_SM16703,
//     LED_STRIP_TYPE_MAX
// };

enum led_strip_type_t {
    LED_STRIP_WS2812 = 0,
    LED_STRIP_SK6812, 
    LED_STRIP_APA106,
    LED_STRIP_SM16703,
    LED_STRIP_TYPE_MAX
};

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
    #define LED_STRIP_BRIGHTNESS
#endif

// #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 3, 0)
//     #error "LiteLED requires ESP-IDF Version 4.3.0 or greater."
// #endif

// LED strip descriptor
// Note: .brightness is supported only for ESP-IDF version >= 4.3
//       and for ESP-IDF version < 5 
typedef struct {
    led_strip_type_t type;      // < LED type
    bool is_rgbw;               // < true for RGBW strips
    #ifdef LED_STRIP_BRIGHTNESS
        uint8_t brightness;     // < Brightness 0..255, call ::show() after change.                                   
    #endif
    size_t length;              // < Number of LEDs in strip
    gpio_num_t gpio;            // < Data GPIO pin
    rmt_channel_t channel;      // < RMT channel
    uint8_t *buf;               // < Xylopyrographer: pointer to the buffer that contains the LED data 
} led_strip_t;


class LiteLED {
    public:
        // Constructor1
        LiteLED( led_strip_type_t led_type, bool rgbw);

        // Constructor2
        LiteLED( led_strip_type_t led_type, bool rgbw, rmt_channel_t channel);
    
        // Destructor??
        // LiteLED::~LiteLED();

        // #ifdef __cplusplus
        //     extern "C" {
        // #endif

        /*
         * @brief Install driver and initialize LED strip
         *
         * This method must be called after the constructor and before any other LiteLED methods
        */        
        esp_err_t begin(uint8_t data_pin, size_t length );

        /*
         * @brief Wait until RMT peripherals is free to send buffer to LEDs
         *
         * @param strip Descriptor of LED strip
         * @param timeout Timeout in RTOS ticks
         * @return `ESP_OK` on success
        */
        // esp_err_t led_strip_wait(led_strip_t *strip, TickType_t timeout);
        esp_err_t show();

        /*
         * @brief Set color of single LED in strip
         *
         * This function does not actually change colors of the LEDs.
         * Call ::led_strip_flush() to send buffer to the LEDs.
         *
         * @param strip Descriptor of LED strip
         * @param num LED number, 0...strip length - 1
         * @param color RGB color
         * @return `ESP_OK` on success
        */
        // esp_err_t led_strip_set_pixel(led_strip_t *strip, size_t num, rgb_t color);
        esp_err_t setPixel( size_t num, rgb_t color);

        /*
         * @brief Set colors of multiple LEDs
         *
         * This function does not actually change colors of the LEDs.
         * Call ::led_strip_flush() to send buffer to the LEDs.
         *
         * @param strip Descriptor of LED strip
         * @param start First LED index, 0-based
         * @param len Number of LEDs
         * @param data Pointer to RGB data
         * @return `ESP_OK` on success
        */
        // esp_err_t led_strip_set_pixels(led_strip_t *strip, size_t start, size_t len, rgb_t *data);
        esp_err_t setPixels( size_t start, size_t len, rgb_t *data);

        /*
         * @brief Set multiple LEDs to the one color
         *
         * This function does not actually change colors of the LEDs.
         * Call ::led_strip_flush() to send buffer to the LEDs.
         *
         * @param strip Descriptor of LED strip
         * @param start First LED index, 0-based
         * @param len Number of LEDs
         * @param color RGB color
         * @return `ESP_OK` on success
        */
        // esp_err_t led_strip_fill(led_strip_t *strip, size_t start, size_t len, rgb_t color);
        esp_err_t fill( rgb_t color );

        /*
         * @brief Set all LEDS to colour 0
         *
         * This function does not actually change colors of the LEDs.
         * Call ::show() to send buffer to the LEDs.
         *
         * @param strip Descriptor of LED strip
         * @return `ESP_OK` on success
        */
        esp_err_t clear();
        
        /*
         * @brief Set the intensity of the LEDs
         *
         * This function does not actually change intensity of the LEDs.
         * Call ::show() to send buffer to the LEDs.
         *
         * @param strip Descriptor of LED strip
         * @return `ESP_OK` on success
        */
        esp_err_t brightness( uint8_t bright );


    private:

        /*
         * @brief Setup library
         *
         * This method must be called before any other led_strip methods
         * RJL - called via ::begin()
        */
        // void led_strip_install();
        void led_strip_install();

        /*
         * @brief Initialize LED strip and allocate buffer memory
         *
         * @param strip Descriptor of LED strip
         * @return `ESP_OK` on success
         * RJL - called via ::begin()
        */
        // esp_err_t led_strip_init(led_strip_t *strip);
        esp_err_t led_strip_init( led_strip_t *strip );

        /*
         * @brief Set color of single LED in strip
         *
         * This function does not actually change colors of the LEDs.
         * Call ::led_strip_flush() to send buffer to the LEDs.
         *
         * @param strip Descriptor of LED strip
         * @param num LED number, 0...strip length - 1
         * @param color RGB color
         * @return `ESP_OK` on success
        */
        esp_err_t led_strip_set_pixel(led_strip_t *strip, size_t num, rgb_t color);

        esp_err_t led_strip_set_pixels(led_strip_t *strip, size_t start, size_t len, rgb_t *data);

        esp_err_t led_strip_fill(led_strip_t *strip, size_t start, size_t len, rgb_t color);

        /*
         * @brief Deallocate buffer memory and release RMT channel
         *
         * @param strip Descriptor of LED strip
         * @return `ESP_OK` on success
        */
        // esp_err_t led_strip_free(led_strip_t *strip);
        // esp_err_t free(led_strip_t *strip);

        /*
         * @brief Send strip buffer to LEDs
         *
         * @param strip Descriptor of LED strip
         * @return `ESP_OK` on success
        */
        esp_err_t led_strip_flush(led_strip_t *strip);

        esp_err_t led_strip_wait(led_strip_t *strip, TickType_t timeout);

        /*
         * @brief Check if associated RMT channel is busy
         *
         * @param strip Descriptor of LED strip
         * @return true if RMT peripherals is busy
        */
        bool led_strip_busy(led_strip_t *strip);


        // #ifdef __cplusplus
        //     }
        // #endif

        /**@}*/

        led_strip_t theStrip;
//         led_strip_type_t _led_type;
//         bool _rgbw;
//         uint8_t _data_pin;
//         size_t _length;
//         rmt_channel_t _channel;
        
};   // class LiteLED

#endif /* __LITELED_H__ */

