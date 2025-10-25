//
/*
    LiteLED Pixel Operations Implementation
*/

#include "ll_strip_pixels.h"
#include <string.h>

esp_err_t led_strip_set_brightness( led_strip_t *strip, uint8_t num ) {
    /* Sets the intensity of all LED's in the strip */
    if ( !( strip && strip->buf ) ) {
        log_d( "Error: Strip not initialized." );
        return ESP_ERR_INVALID_ARG;
    }
    strip->brightness = num;
    return ESP_OK;
}

uint8_t led_strip_get_brightness( led_strip_t *strip ) {
    /* Returns the LED strip brightness value */
    if ( !( strip && strip->buf ) ) {
        log_d( "Error: Strip not initialized." );
        return 0;
    }
    return strip->bright_act;
}

esp_err_t led_strip_set_pixel( led_strip_t *strip, size_t num, rgb_t color ) {
    /* Sets the color of an individual LED in the strip as per the specified LED color order */
    if ( !( strip && strip->buf && num <= strip->length ) ) {
        log_d( "Error: Strip not initialized or LED number out of bounds." );
        return ESP_ERR_INVALID_ARG;
    }

    color_order_t order = use_custom_color_order ? custom_color_order : led_params[ strip->type ].order;
    size_t idx = num * COLOR_SIZE( strip );

    switch ( order ) {
        case ORDER_RGB:
            strip->buf[ idx ] = color.r;
            strip->buf[ idx + 1 ] = color.g;
            strip->buf[ idx + 2 ] = color.b;
            if ( strip->is_rgbw ) {
                strip->buf[ idx + 3 ] = strip->auto_w ? rgb_luma( color ) : 0;
            }
            break;
        case ORDER_RBG:
            strip->buf[ idx ] = color.r;
            strip->buf[ idx + 1 ] = color.b;
            strip->buf[ idx + 2 ] = color.g;
            if ( strip->is_rgbw ) {
                strip->buf[ idx + 3 ] = strip->auto_w ? rgb_luma( color ) : 0;
            }
            break;
        case ORDER_GRB:
            strip->buf[ idx ] = color.g;
            strip->buf[ idx + 1 ] = color.r;
            strip->buf[ idx + 2 ] = color.b;
            if ( strip->is_rgbw ) {
                strip->buf[ idx + 3 ] = strip->auto_w ? rgb_luma( color ) : 0;
            }
            break;
        case ORDER_GBR:
            strip->buf[ idx ] = color.g;
            strip->buf[ idx + 1 ] = color.b;
            strip->buf[ idx + 2 ] = color.r;
            if ( strip->is_rgbw ) {
                strip->buf[ idx + 3 ] = strip->auto_w ? rgb_luma( color ) : 0;
            }
            break;
        case ORDER_BRG:
            strip->buf[ idx ] = color.b;
            strip->buf[ idx + 1 ] = color.r;
            strip->buf[ idx + 2 ] = color.g;
            if ( strip->is_rgbw ) {
                strip->buf[ idx + 3 ] = strip->auto_w ? rgb_luma( color ) : 0;
            }
            break;
        case ORDER_BGR:
            strip->buf[ idx ] = color.b;
            strip->buf[ idx + 1 ] = color.g;
            strip->buf[ idx + 2 ] = color.r;
            if ( strip->is_rgbw ) {
                strip->buf[ idx + 3 ] = strip->auto_w ? rgb_luma( color ) : 0;
            }
            break;
        default:
            /* Default is to set RGB colour order */
            strip->buf[ idx ] = color.r;
            strip->buf[ idx + 1 ] = color.g;
            strip->buf[ idx + 2 ] = color.b;
            if ( strip->is_rgbw ) {
                strip->buf[ idx + 3 ] = strip->auto_w ? rgb_luma( color ) : 0;
            }
            log_d( "Error: Invalid color order specifier. Default RGB order set." );
            return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

rgb_t led_strip_get_pixel( led_strip_t *strip, size_t num ) {
    /* Gets the color value of a specified LED in the strip */
    rgb_t res;
    if ( !( strip && strip->buf && num <= strip->length ) ) {
        log_d( "Error: Invalid argument or strip not initialized." );
        res.r = res.g = res.b = 0;
        return res;
    }

    size_t idx = num * COLOR_SIZE( strip );
    color_order_t order = use_custom_color_order ? custom_color_order : led_params[ strip->type ].order;

    switch ( order ) {
        case ORDER_RGB:
            res.r = strip->buf[ idx ];
            res.g = strip->buf[ idx + 1 ];
            res.b = strip->buf[ idx + 2 ];
            break;
        case ORDER_RBG:
            res.r = strip->buf[ idx ];
            res.b = strip->buf[ idx + 1 ];
            res.g = strip->buf[ idx + 2 ];
            break;
        case ORDER_GRB:
            res.g = strip->buf[ idx ];
            res.r = strip->buf[ idx + 1 ];
            res.b = strip->buf[ idx + 2 ];
            break;
        case ORDER_GBR:
            res.g = strip->buf[ idx ];
            res.b = strip->buf[ idx + 1 ];
            res.r = strip->buf[ idx + 2 ];
            break;
        case ORDER_BRG:
            res.b = strip->buf[ idx ];
            res.r = strip->buf[ idx + 1 ];
            res.g = strip->buf[ idx + 2 ];
            break;
        case ORDER_BGR:
            res.b = strip->buf[ idx ];
            res.g = strip->buf[ idx + 1 ];
            res.r = strip->buf[ idx + 2 ];
            break;
        default:
            log_d( "Error: Invalid colour order specifier. Defaults used." );
            res.r = res.g = res.b = 0;
    }
    return res;
}

esp_err_t led_strip_set_pixels( led_strip_t *strip, size_t start, size_t len, rgb_t* data ) {
    /* Sets a range of pixels to colors defined in a buffer */
    if ( !( strip && strip->buf && len && start + len <= strip->length && data ) ) {
        log_d( "Error: Strip not initialized or LED number out of bounds." );
        return ESP_ERR_INVALID_ARG;
    }
    for ( size_t i = 0; i < len; i++ ) {
        if ( esp_err_t res = ( led_strip_set_pixel( strip, i + start, data[ i ] ) ) != ESP_OK ) {
            log_d( "Error: Failed to set pixel %d - %s.", i, esp_err_to_name( res ) );
            return res;
        }
    }
    return ESP_OK;
}

esp_err_t led_strip_set_pixels_c( led_strip_t *strip, size_t start, size_t len, crgb_t* data ) {
    /* Sets a range of pixels to colors defined in a buffer (color code format) */
    if ( !( strip && strip->buf && len && start + len <= strip->length ) ) {
        log_d( "Error: Strip not initialized or LED number out of bounds." );
        return ESP_ERR_INVALID_ARG;
    }
    for ( size_t i = 0; i < len; i++ ) {
        if ( esp_err_t res = led_strip_set_pixel( strip, i + start, rgb_from_code( data[ i ] ) ) != ESP_OK ) {
            log_d( "Error: Failed to set pixel %d - %s.", i, esp_err_to_name( res ) );
            return res;
        }
    }
    return ESP_OK;
}

esp_err_t led_strip_fill( led_strip_t *strip, rgb_t color ) {
    /* Sets all LED's in the strip to a single color */
    if ( !( strip && strip->buf ) ) {
        log_d( "Error: Strip not initialized." );
        return ESP_ERR_INVALID_ARG;
    }
    size_t num_pixels = strip->length;
    esp_err_t res = ESP_OK;
    for ( size_t i = 0; i < num_pixels; i++ ) {
        res = led_strip_set_pixel( strip, i, color );
        if ( res != ESP_OK ) {
            log_d( "Error: Failed to set pixel %d - %s.", i, esp_err_to_name( res ) );
            break;
        }
    }
    return res;
}

esp_err_t led_strip_fill_random( led_strip_t *strip ) {
    /* Fills the LED strip with random colors */
    if ( !( strip && strip->buf ) ) {
        log_d( "Error: Strip not initialized." );
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t res = ESP_OK;
    for ( size_t i = 0; i < strip->length; i++ ) {
        res = led_strip_set_pixel( strip, i, rgb_from_code( ( esp_random() & 0xFFFFFF ) ) );
        if ( res != ESP_OK ) {
            log_d( "Error: Failed to set pixel %d - %s.", i, esp_err_to_name( res ) );
            return res;
        }
    }
    return ESP_OK;
}

esp_err_t led_strip_clear( led_strip_t *strip, size_t num_bytes ) {
    /* Sets the color of all LED's in the strip to black */
    if ( !( strip && num_bytes <= PIXEL_SIZE( strip ) ) ) {
        log_d( "Error: Strip not initialized or buffer size out of bounds." );
        return ESP_ERR_INVALID_ARG;
    }
    memset( strip->buf, 0, num_bytes );
    return ESP_OK;
}

esp_err_t led_strip_set_color_order( led_strip_t *strip, color_order_t led_order ) {
    /* Sets the color order of the LED's in the strip */
    custom_color_order = led_order;
    use_custom_color_order = true;
    log_d( "Setting a custom color order to: %s.", col_ord[ custom_color_order ] );
    log_d( "Custom color order now set to: %s.", use_custom_color_order ? "true" : "false" );
    return ESP_OK;
}

esp_err_t led_strip_set_default_color_order( led_strip_t *strip ) {
    /* Sets the color order of the LED's in the strip to the value defined by the LED type parameters */
    custom_color_order = led_params[ strip->type ].order;
    use_custom_color_order = false;
    log_d( "Setting the color order to its default: %s.", col_ord[ custom_color_order ] );
    log_d( "Custom color order now set to: %s.", use_custom_color_order ? "true" : "false" );
    return ESP_OK;
}

//  --- EOF --- //
