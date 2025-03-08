//
/*
    ESP32 RMT-based driver for various types of RGB LED strips
    MIT Licensed as described in the file LICENSE
*/

#include "driver/rmt_tx.h"

#ifndef __LITELED_H__
#define __LITELED_H__

// check for ESP32
#if !ARDUINO_ARCH_ESP32
    #error "LiteLED: This library is only for use with ESP32 boards."
#endif

// check for arduino-esp32 core compatibility
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL( 2, 0, 3 ) ||    \
    ( ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL( 3, 0, 0 ) && \
    ESP_ARDUINO_VERSION <= ESP_ARDUINO_VERSION_VAL( 3, 0, 2 ) )
#error "LiteLED: This library is not compatible with this version of the arduino-esp32 core. See the library documentation for options."
#elif ( ( ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL( 2, 0, 3 ) ) && ( ESP_ARDUINO_VERSION <= ESP_ARDUINO_VERSION_VAL( 2, 0, 17 ) ) ) || \
      ( ( ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL( 3, 0, 3 ) && ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL( 3, 1, 0 ) ) )
#error "LiteLED: LiteLED version 1.2.1 is required for this version of the arduino-esp32 core. See the LiteLED library documentation for options."
#endif

// check for RMT support
#ifndef SOC_RMT_SUPPORTED
    #if !SOC_RMT_SUPPORTED
        #error "LiteLED: Use of this library requires an ESP32 with an RMT peripheral."
    #endif
#endif

// check if the RMT supports DMA
#ifdef SOC_RMT_SUPPORT_DMA
    #define LL_DMA_SUPPORT SOC_RMT_SUPPORT_DMA
#else
    #define LL_DMA_SUPPORT 0
#endif
#if !LL_DMA_SUPPORT
    // #warning "LiteLED: Selected ESP32 model does not support RMT DMA access. Use of RMT DMA will be disabled."
    #pragma message "LiteLED: Selected ESP32 model does not support RMT DMA access. Use of RMT DMA will be disabled."
#endif

#include "llrgb.h"

enum led_strip_type_t {
    /* note: if this enum is modified, must also
    change led_type[] in 'llrmt.h' to match */
    LED_STRIP_WS2812 = 0,
    LED_STRIP_WS2812_RGB,
    LED_STRIP_SK6812,
    LED_STRIP_APA106,
    LED_STRIP_SM16703,
    LED_STRIP_TYPE_MAX
};

enum color_order_t {
    /* note: if this enum is modified, must also
    change col_ord[] in 'llrmt.h' to match */
    ORDER_RGB = 0,
    ORDER_RBG,
    ORDER_GRB,
    ORDER_GBR,
    ORDER_BRG,
    ORDER_BGR,
    ORDER_MAX   // not a valid colour order, used to mark the end of the enum
};

typedef struct {
    rmt_tx_channel_config_t     led_chan_config;    /* RMT channel configuration for the LED strip. */
    rmt_transmit_config_t       led_tx_config;      /* RMT transmit configuration */
    rmt_channel_handle_t        led_chan = NULL;    /* RMT channel allocated by the RMT driver */
    rmt_simple_encoder_config_t led_encoder_cfg;    /* RMT encoder configuration */
    rmt_encoder_handle_t        led_encoder = NULL; /* RMT encoder handle */
} led_strip_cfg_t;

typedef struct {
    rmt_symbol_word_t led_0;
    rmt_symbol_word_t led_1;
    rmt_symbol_word_t led_reset;
    color_order_t order;
} led_params_t;

typedef struct {
    uint8_t *buf;
    size_t length;
    uint8_t brightness;
    uint8_t bright_act;
    uint8_t gpio;
    uint8_t type;
    bool is_rgbw;
    bool auto_w;
    led_strip_cfg_t stripCfg;
} led_strip_t;


// defines for setting the led encoder DMA usage
enum ll_dma_t : uint32_t {
    DMA_ON = 1,
    DMA_OFF = 0,
    DMA_DEFAULT = DMA_OFF
};

// defines for setting the led encoder callback interrupt priority level
enum ll_priority_t : int {
    PRIORITY_DEFAULT = 0,
    PRIORITY_HIGH = 1,
    PRIORITY_MED = 2,
    PRIORITY_LOW = 3
};

class LiteLED {
  public:
    // @brief Class constructor. Set the LED parameters for the RMT driver
    // @param led_type Enumerated value for the type of LED's in the strip
    // @param rgbw Set true if the strip is RGBW type
    // @return 'ESP_OK' on success
    LiteLED( led_strip_type_t led_type, bool rgbw );
    ~LiteLED();

    // @brief Initialize the strip
    // @param data_pin GPIO pin connected to the DIN pin of the strip
    // @param length Number of LED's in the strip
    // @param auto_w Optional. Only used for RGBW strips. Set false to not use the automatic W channel value set by the library
    // @return 'ESP_OK' on success
    esp_err_t begin( uint8_t data_pin, size_t length, bool auto_w = true );

    // @brief Initialize the strip
    // @param data_pin GPIO pin connected to the DIN pin of the strip
    // @param length Number of LED's in the strip
    // @param dma_flag Enumerated value that sets the DMA usage of the led encoder
    // @param priority Enumerated value that sets the interrupt priority of led encoder callback
    // @param auto_w Optional. Only used for RGBW strips. Set false to not use the automatic W channel value set by the library
    // @return 'ESP_OK' on success
    esp_err_t begin( uint8_t data_pin, size_t length, ll_dma_t dma_flag, ll_priority_t priority, bool auto_w = true );

    // @brief Flush the the LED buffer to the strip
    esp_err_t show();

    // @brief Set color of single LED in strip, optionally flush the buffer to the strip
    // @param num Position of the LED in the strip, 0-based
    // @param color rgb_t or crgb_t Color to set the LED to
    // @param show Optional. Set true to flush the buffer to the strip before returning
    // @return 'ESP_OK' on success
    esp_err_t setPixel( size_t num, rgb_t color, bool show = false );
    esp_err_t setPixel( size_t num, crgb_t color, bool show = false );

    // @brief Set colors of multiple consecutive LEDs, optionally flush the buffer to the strip
    // @param start First LED index, 0-based
    // @param len The number of consecutive LEDs in the strip to which we are writing
    // @param data Pointer to data. Layout must match the color type
    // @param show Optional. Set true to flush the buffer to the strip before returning
    // @return 'ESP_OK' on success
    esp_err_t setPixels( size_t start, size_t len, rgb_t *data, bool show = false );
    esp_err_t setPixels( size_t start, size_t len, crgb_t *data, bool show = false );

    // @brief Set the entire strip to a color, optionally flush the buffer to the LEDs
    // @param color rgb_t or crgb_t Colour value to set the strip to
    // @param show Optional. Set true to flush the buffer to the strip before returning
    // @return 'ESP_OK' on success
    esp_err_t fill( rgb_t color, bool show = false );
    esp_err_t fill( crgb_t color, bool show = false );

    // @brief Clear the strip buffer, optionally flush the buffer to the strip
    // @param show Optional. Set true to flush the buffer to the strip before returning
    // @return 'ESP_OK' on success
    esp_err_t clear( bool show = 0 );

    // @brief Set the intensity of the LEDs, optionally flush the buffer to the strip
    // @param bright Brightness value, 0-255
    // @param show Optional. Set true to set strip intensity to 'bright' before returning
    // @return 'ESP_OK' on success
    esp_err_t brightness( uint8_t bright, bool show = false );

    // @brief Get the intensity value of the LEDs
    // @return The 'bright' value of the strip
    uint8_t getBrightness();

    // @brief Get, in rgb_t format, the color of a single LED in the strip
    // @param num Position of the LED in the strip, 0-based
    // @return The rgb_t color value of the LED
    rgb_t getPixel( size_t num );

    // @brief Get, in crgb_t format, the color of a single LED in strip
    // @param num Position of the LED in the strip, 0-based
    // @return The crgb_t color value of the LED
    crgb_t getPixelC( size_t num );

    // @brief Fill the strip buffer with random colors, optionally flush the buffer to the strip
    // @param show Optional. Set true to flush the buffer to the strip before returning. False if ommited.
    // @return 'ESP_OK' on success
    esp_err_t fillRandom( bool show = false );

    // @brief Set a custom order of the LED colors
    // @param led_order. Enumerated value of the color order of LED's in the strip.
    // @return 'ESP_OK' on success
    esp_err_t setOrder( color_order_t led_order = ORDER_GRB );

    // @brief Reset the color order of the LED's in the strip to its default value
    // @param None.
    // @return 'ESP_OK' on success
    esp_err_t resetOrder();

  private:
    led_strip_t theStrip;   // LED strip object for this class
    esp_err_t free();

};   // class LiteLED

#endif /* __LITELED_H__ */
