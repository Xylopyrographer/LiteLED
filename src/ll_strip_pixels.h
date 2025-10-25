//
/*
    LiteLED Pixel Operations

    LED strip pixel manipulation functions:
    - Set/get individual pixels
    - Set multiple pixels
    - Fill operations
    - Clear operations
    - Brightness control
    - Color order management
*/

#ifndef __LL_STRIP_PIXELS_H__
#define __LL_STRIP_PIXELS_H__

#include "LiteLED.h"
#include "llrgb.h"
#include "ll_led_timings.h"
#include "ll_strip_core.h"
#include "esp32-hal-log.h"

// Set global brightness for all LEDs (0-255)
esp_err_t led_strip_set_brightness( led_strip_t *strip, uint8_t num );

// Get current brightness value
uint8_t led_strip_get_brightness( led_strip_t *strip );

// Set individual pixel color (respects color order)
esp_err_t led_strip_set_pixel( led_strip_t *strip, size_t num, rgb_t color );

// Get individual pixel color (respects color order)
rgb_t led_strip_get_pixel( led_strip_t *strip, size_t num );

// Set multiple pixels from rgb_t array
esp_err_t led_strip_set_pixels( led_strip_t *strip, size_t start, size_t len, rgb_t* data );

// Set multiple pixels from crgb_t (color code) array
esp_err_t led_strip_set_pixels_c( led_strip_t *strip, size_t start, size_t len, crgb_t* data );

// Fill entire strip with single color
esp_err_t led_strip_fill( led_strip_t *strip, rgb_t color );

// Fill strip with random colors
esp_err_t led_strip_fill_random( led_strip_t *strip );

// Clear strip (set all to black)
esp_err_t led_strip_clear( led_strip_t *strip, size_t num_bytes );

// Set custom color order for the strip
esp_err_t led_strip_set_color_order( led_strip_t *strip, color_order_t led_order );

// Reset to default color order based on LED type
esp_err_t led_strip_set_default_color_order( led_strip_t *strip );

#endif /* __LL_STRIP_PIXELS_H__ */

//  --- EOF --- //
