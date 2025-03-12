//
/*
    ESP32 RMT-based driver for various types of RGB LED strips
    - MIT Licensed as described in the file LICENSE
*/

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000    /* 10 MHz resolution, 1 tick = 0.1us (led strip needs a high resolution) */
#define LL_MEM_BLOCK_SIZE_DEFAULT ( ( size_t)SOC_RMT_MEM_WORDS_PER_CHANNEL ) /* size of the dedicated memory block owned by the RMT channel if the SoC does not support RMT DMA */
#define LL_MEM_BLOCK_SIZE_DMA 1024              /* size of the dedicated memory block owned by the RMT channel if the SoC supports RMT DMA and DMA is enabled */
#define LL_ENCODER_MIN_CHUNK_SIZE 64            /* Minimum amount of free space, in RMT symbols, the encoder needs in order to guarantee it always returns non-zero. Defaults to 64 if zero or not given */

#define COLOR_SIZE( strip ) ( 3 + ( (strip)->is_rgbw != 0 ) )
#define PIXEL_SIZE( strip ) ( COLOR_SIZE( strip ) * (strip)->length )

static const rmt_symbol_word_t ws2812_zero = {
    static_cast<uint16_t>( 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T0H=0.3us
    1,                                                                      // .level0
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T0L=0.9us
    0                                                                       // .level1
};
static const rmt_symbol_word_t ws2812_one = {
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T1H=0.9us
    1,                                                                      // .level0
    static_cast<uint16_t>( 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T1L=0.3us
    0                                                                       // .level1
};
static const rmt_symbol_word_t ws2812_reset = {                                 /* WS2812 reset is 50uS */
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2 ),    //.duration0 = 25us
    0,                                                                          // .level0  - was 1
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2 ),    //.duration0 = 25us
    0                                                                           // .level1
};

static const rmt_symbol_word_t apa106_zero = {
    static_cast<uint16_t>( 0.35 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T0H=0.35us
    1,                                                                       // .level0
    static_cast<uint16_t>( 1.36 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T0L=1.36us
    0                                                                        // .level1
};
static const rmt_symbol_word_t apa106_one = {
    static_cast<uint16_t>( 1.36 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T1H=1.36us
    1,                                                                       // .level0
    static_cast<uint16_t>( 0.35 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T1L=0.35us
    0                                                                        // .level1
};
static const rmt_symbol_word_t apa106_reset = {                                 /* APA106 reset is 50uS */
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2 ),    //.duration0 = 25us
    0,                                                                          // .level0 - was 1
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2 ),    //.duration0 = 25us
    0                                                                           // .level1
};

static const rmt_symbol_word_t sm16703_zero = {
    static_cast<uint16_t>( 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T0H=0.3us
    1,                                                                      // .level0
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T0L=0.9us
    0                                                                       // .level1
};
static const rmt_symbol_word_t sm16703_one = {
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T1H=0.9us
    1,                                                                      // .level0
    static_cast<uint16_t>( 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration1 = T1L=0.3us
    0                                                                       // .level1
};
static const rmt_symbol_word_t sm16703_reset = {                                /* SM16703 reset is 210uS */
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 210 / 2 ),   //.duration0 = 105us
    0,                                                                          // .level0 - was 1
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 210 / 2 ),   //.duration0 = 105us
    0                                                                           // .level1
};

static const rmt_symbol_word_t sk6812_zero = {
    static_cast<uint16_t>( 0.32 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T0H=0.32us
    1,                                                                       // .level0
    static_cast<uint16_t>( 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),    // .duration1 = T0L=0.9us
    0                                                                        // .level1
};
static const rmt_symbol_word_t sk6812_one = {
    static_cast<uint16_t>( 0.64 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),   // .duration0 = T1H=0.64us
    1,                                                                       // .level0
    static_cast<uint16_t>( 0.4 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000 ),    // .duration1 = T1L=0.4us
    0                                                                        // .level1
};
static const rmt_symbol_word_t sk6812_reset = {                                 /* SK6812 reset is 90uS */
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 90 / 2 ),    //.duration0 = 45us
    0,                                                                          // .level0 - was 1
    static_cast<uint16_t>( RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 90 / 2 ),    //.duration0 = 45us
    0                                                                           // .level1
};

static const led_params_t led_ws2812 = {
    .led_0 = ws2812_zero,
    .led_1 = ws2812_one,
    .led_reset = ws2812_reset,
    .order = ORDER_GRB
};
static const led_params_t led_ws2812_rgb = {
    /* seems there are some "WS2812" LEDs with non-standard RGB colour order! */
    .led_0 = ws2812_zero,
    .led_1 = ws2812_one,
    .led_reset = ws2812_reset,
    .order = ORDER_RGB
};
static const led_params_t led_apa106 = {
    .led_0 = apa106_zero,
    .led_1 = apa106_one,
    .led_reset = apa106_reset,
    .order = ORDER_RGB
};
static const led_params_t led_sm16703 = {
    .led_0 = sm16703_zero,
    .led_1 = sm16703_one,
    .led_reset = sm16703_reset,
    .order = ORDER_RGB
};
static const led_params_t led_sk6812 = {
    .led_0 = sk6812_zero,
    .led_1 = sm16703_one,
    .led_reset = sm16703_reset,
    .order = ORDER_GRB
};

static const led_params_t led_params[] = {
    [ LED_STRIP_WS2812 ]  = led_ws2812,
    [ LED_STRIP_WS2812_RGB ] = led_ws2812_rgb,
    [ LED_STRIP_SK6812 ]  = led_sk6812,
    [ LED_STRIP_APA106 ]  = led_apa106,
    [ LED_STRIP_SM16703 ] = led_sm16703
};

// led_strip_cfg_t stripCfg;
color_order_t custom_color_order = ORDER_GRB;
bool use_custom_color_order = false;
char *col_ord[] = { "order_rgb", "order_rbg", "order_grb", "order_gbr", "order_brg", "order_bgr", "order_max" };
char *led_type[] = { "ws2812", "ws2812_rgb", "sk6812", "apa106", "sm167O3" };

void led_strip_debug_dump( led_strip_t *strip );

/*
    The encoder_callback() function is called by the RMT driver when it needs more symbols to send.
    The encoder_callback() pulls byte of data from the LED data buffer and encodes it into the RMT symbols,
    which are then stored into the allocated RMT memory block. The RMT driver then pulls the symbols
    from the memory block and sends them to the GPIO pin via the RMT hardware peripheral.

    static size_t encoder_callback(
                  const void *data, // [in] - provided via the "simple_encoder" handle
                  size_t data_size, // [in] - provided via the "simple_encoder" handle
                  size_t symbols_written, // [in] - provided by the RMT driver
                  size_t symbols_free, // [in] - provided by the RMT driver
                  rmt_symbol_word_t* symbols, // [in] - symbol timing is defined by the user and provided via the "simple_encoder" handle
                  bool *done, [out] - // provided by "encoder_callback" code, used by the RMT driver to indicate the end of the transaction
                  void *arg [in] - // provided by the "simple_encoder" handle
                  )
*/
IRAM_ATTR static size_t led_encoder_cb( const void* data, size_t data_size,
                                        size_t symbols_written, size_t symbols_free,
                                        rmt_symbol_word_t *symbols, bool *done, void *arg ) {

    // We need a minimum of 8 symbol spaces to encode a byte. We only
    // need one to encode a reset, but it's simpler to simply demand that
    // there are 8 symbol spaces free to write anything.
    if ( symbols_free < 8 ) {
        return 0;
    }

    led_strip_t *strip = ( led_strip_t* )arg; // Cast arg to led_strip_t*
    uint8_t *data_bytes = ( uint8_t* )data;
    uint8_t photons = strip->brightness;      // Get the brightness value

    // Retrieve the current position from enc_pos
    size_t data_pos = strip->stripCfg.enc_pos;

    if ( data_pos < data_size ) {
        uint8_t currentByte = scale8_video( data_bytes[ data_pos ], photons ); // Set the brightness of the colour value
        // Encode a byte, as in, convert the byte to an RMT symbol
        size_t symbol_pos = 0;
        for ( int bitmask = 0x80; bitmask != 0; bitmask >>= 1 ) {
            symbols[ symbol_pos++ ] = ( currentByte & bitmask ) ? led_params[ strip->type ].led_1 : led_params[ strip->type ].led_0;
        }
        // Update the current position in the buffer
        strip->stripCfg.enc_pos++;
        // We're done; we should have written 8 symbols.
        return symbol_pos;
    }
    else {
        // All bytes already are encoded.
        // Encode the reset, and we're done.
        symbols[ 0 ] = led_params[ strip->type ].led_reset;
        strip->stripCfg.enc_pos = 0;    // reset the position in the buffer
        *done = 1;                      // Indicate end of the transaction.
        return 1;                       // We only wrote one symbol
    }
}

esp_err_t led_strip_init( led_strip_t *strip ) {
    /* initializes all structures and variables required for the library */
    esp_err_t res = ESP_OK;
    if ( !( strip && strip->length > 0 && strip->type < LED_STRIP_TYPE_MAX ) ) {
        log_e( "led_strip_init(): invalid aguments." );
        return ESP_ERR_INVALID_ARG;
    }
    // populate the stripCfg struct with the necessary values
    strip->stripCfg.led_chan = NULL;
    strip->stripCfg.led_encoder = NULL;
    strip->stripCfg.led_chan_config = {
        .gpio_num = ( gpio_num_t )strip->gpio       /* ( gpio_num_t )RMT_LED_STRIP_GPIO_NUM */,
        .clk_src = RMT_CLK_SRC_DEFAULT,             /* select source clock */
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .mem_block_symbols = LL_MEM_BLOCK_SIZE_DEFAULT,
        .trans_queue_depth = 4,                     /* set the number of transactions that can be pending in the background */
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL( 5, 1, 2 )
        .intr_priority = 0,                         /* callback interrupt priority, default is 0 - gaurd is for compatibility with earlier versions of esp32-idf */
#endif
        .flags = {
            .invert_out = 0,            /* do not invert output */
            .with_dma = DMA_DEFAULT,    /* do not use DMA by default */
            .io_loop_back = 0,          /* no loopback */
            .io_od_mode = 0,            /* open drain */
        }
    };
    log_d( "Defining the led encoder configuration." );
    strip->stripCfg.led_encoder_cfg = {
        led_encoder_cb,                         /* the led encoder function */
        strip,                                  /* arg's for the led encoder function */
        ( size_t )LL_ENCODER_MIN_CHUNK_SIZE,    /* explcitly set - default is 64 */
    };
    log_d( "Setting the RMT transmit configuration." );
    strip->stripCfg.led_tx_config = {
        .loop_count = 0,
        .flags = {
            .eot_level = 0,
            .queue_nonblocking = 0,
        }
    };
    log_d( "RMT driver configuration complete. Debug dump..." );
    led_strip_debug_dump( strip );
    return ESP_OK;
}

esp_err_t led_strip_init_modify( led_strip_t *strip, ll_dma_t use_dma, ll_priority_t priority ) {
    /* sets user defined DMA and interrupt priority settings for the LED strip */
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL( 5, 1, 2 )
    strip->stripCfg.led_chan_config.intr_priority = priority;    /* callback interrupt priority, default is 0 - gaurd is for compatibility with earlier versions of esp32-idf */
    log_d( "led_strip_init_modify(): Setting the RMT interrupt priority to %d.", priority );
#endif
#if LL_DMA_SUPPORT
    strip->stripCfg.led_chan_config.flags.with_dma = use_dma;    /* use DMA (or not) */
    log_d( "led_strip_init_modify(): Setting the RMT DMA usage to %s.", use_dma ? "ON" : "OFF" );
    if ( use_dma ) {
        strip->stripCfg.led_chan_config.mem_block_symbols = ( size_t )LL_MEM_BLOCK_SIZE_DMA;   /* RMT DMA lets us use a large block size */
        log_d( "led_strip_init_modify(): Setting the RMT DMA memory block size to %d.", strip->stripCfg.led_chan_config.mem_block_symbols );
    }
#else
    strip->stripCfg.led_chan_config.flags.with_dma = DMA_DEFAULT;  /* turn off RMT DMA on ESP32 models that don't support it  */
    strip->stripCfg.led_chan_config.mem_block_symbols = LL_MEM_BLOCK_SIZE_DEFAULT; /* set the default memory block size */
    log_e( "led_strip_init_modify(): Attempt to set RMT DMA - not supported on this ESP32 model. Defaults used." );
#endif
    return ESP_OK;
}

esp_err_t led_strip_install( led_strip_t *strip ) {
    /* installs the LED strip and allocates the necessary resources */
    strip->buf = ( uint8_t* )calloc( strip->length, COLOR_SIZE( strip ) );
    if ( !strip->buf ) {
        log_e( "led_strip_install(): failed to allocate buffer - ESP_ERR_NO_MEM." );
        return ESP_ERR_NO_MEM;
    }
    esp_err_t res = ESP_OK;
    if ( ( res = rmt_new_tx_channel( &strip->stripCfg.led_chan_config, &strip->stripCfg.led_chan ) ) != ESP_OK ) {
        log_e( "rmt_new_tx_channel(): failed to create RMT TX channel - %s.", esp_err_to_name( res ) );
        return res;
    }
    if ( ( res = rmt_new_simple_encoder( &strip->stripCfg.led_encoder_cfg, &strip->stripCfg.led_encoder ) ) != ESP_OK ) {
        log_e( "rmt_new_simple_encode(): failed to create LED encoder - %s.", esp_err_to_name( res ) );
        return res;
    }
    log_d( "Enabling the RMT TX channel." );
    if ( ( res = rmt_enable( strip->stripCfg.led_chan ) ) != ESP_OK ) {
        log_e( "rmt_enable(): failed to enable RMT TX channel - %s.", esp_err_to_name( res ) );
        return res;
    }
    log_d( "LED strip installed. Debug dump..." );
    led_strip_debug_dump( strip );
    return res;
}

esp_err_t led_strip_free( led_strip_t *strip ) {
    /* deletes all resources used by the strip */
    if ( !( strip && strip->buf ) ) {
        log_e( "led_strip_free(): strip not initialized." );
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t res = ESP_OK;
    if ( ( res = rmt_tx_wait_all_done( strip->stripCfg.led_chan, portMAX_DELAY ) ) != ESP_OK ) {
        log_e( "led_strip_free(): fail on wait for RMT TX to finish - %s.", esp_err_to_name( res ) );
        return res;
    }
    if ( ( res = rmt_disable( strip->stripCfg.led_chan ) ) != ESP_OK ) {
        log_e( "led_strip_free(): fail on disable RMT TX channel - %s.", esp_err_to_name( res ) );
        return res;
    }
    if ( ( rmt_del_channel( strip->stripCfg.led_chan ) ) != ESP_OK ) {
        log_e( "led_strip_free(): fail on delete RMT TX channel - %s.", esp_err_to_name( res ) );
        return res;
    }
    if ( ( rmt_del_encoder( strip->stripCfg.led_encoder ) ) != ESP_OK ) {
        log_e( "led_strip_free(): fail on delete RMT encoder - %s.", esp_err_to_name( res ) );
        return res;
    }
    free( strip->buf );
    strip->buf = NULL;
    return res;
}

esp_err_t led_strip_flush( led_strip_t *strip ) {
    /* pushes all data from the LED buffer to the LED strip */
    esp_err_t res = ESP_OK;
    if ( ( res = rmt_transmit( strip->stripCfg.led_chan, strip->stripCfg.led_encoder, strip->buf, PIXEL_SIZE( strip ), &strip->stripCfg.led_tx_config ) ) != ESP_OK ) {
        log_e( "led_strip_flush(): Fail on 'rmt_transmit()'. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    if ( ( res = rmt_tx_wait_all_done( strip->stripCfg.led_chan, portMAX_DELAY ) ) != ESP_OK ) {
        log_e( "led_strip_flush(): Fail on 'rmt_tx_wait_all_done()'. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    return res;
}

esp_err_t led_strip_set_brightness( led_strip_t *strip, uint8_t num ) {
    /* sets the intensity of all LED's in the strip */
    if ( !( strip && strip->buf ) ) {
        log_e( "led_strip_set_brightness(): strip not initialized." );
        return ESP_ERR_INVALID_ARG;
    }
    strip->brightness = num;
    return ESP_OK;
}

esp_err_t led_strip_set_pixel( led_strip_t *strip, size_t num, rgb_t color ) {
    /* sets the color of in individual LED in the strip as per the specified LED color order */
    if ( !( strip && strip->buf && num <= strip->length ) ) {
        log_e( "led_strip_set_pixel(): strip not initialized or LED number out of bounds." );
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
            log_e( "led_strip_set_pixel(): invalid color order specifier." );
            return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

esp_err_t led_strip_set_pixels( led_strip_t *strip, size_t start, size_t len, rgb_t* data ) {
    /* sets a range of pixels to colors defined in a buffer */
    if ( !( strip && strip->buf && len && start + len <= strip->length && data ) ) {
        log_e( "led_strip_set_pixels(): strip not initialized or LED number out of bounds." );
        return ESP_ERR_INVALID_ARG;
    }
    for ( size_t i = 0; i < len; i++ ) {
        if ( esp_err_t res = ( led_strip_set_pixel( strip, i + start, data[ i ] ) ) != ESP_OK ) {
            log_e( "led_strip_set_pixels(): failed to set pixel %d - %s.", i, esp_err_to_name( res ) );
            return res;
        }
    }
    return ESP_OK;
}

esp_err_t led_strip_set_pixels_c( led_strip_t *strip, size_t start, size_t len, crgb_t* data ) {
    /* sets a range of pixels to colors defined in a buffer */
    if ( !( strip && strip->buf && len && start + len <= strip->length ) ) {
        log_e( "led_strip_set_pixels_c(): strip not initialized or LED number out of bounds." );
        return ESP_ERR_INVALID_ARG;
    }
    for ( size_t i = 0; i < len; i++ ) {
        if ( esp_err_t res = led_strip_set_pixel( strip, i + start, rgb_from_code( data[ i ] ) ) != ESP_OK ) {
            log_e( "led_strip_set_pixels_c(): failed to set pixel %d - %s.", i, esp_err_to_name( res ) );
            return res;
        }
    }
    return ESP_OK;
}

esp_err_t led_strip_fill( led_strip_t *strip, rgb_t color ) {
    /* sets all LED's in the strip to a single color */
    if ( !( strip && strip->buf ) ) {
        log_e( "led_strip_fill(): strip not initialized." );
        return ESP_ERR_INVALID_ARG;
    }
    size_t num_pixels = strip->length;
    esp_err_t res = ESP_OK;
    for ( size_t i = 0; i < num_pixels; i++ ) {
        res = led_strip_set_pixel( strip, i, color );
        if ( res != ESP_OK ) {
            log_e( "led_strip_fill(): failed to set pixel %d - %s.", i, esp_err_to_name( res ) );
            break;
        }
    }
    return res;
}

esp_err_t led_strip_fill_random( led_strip_t *strip ) {
    /* fills the LED strip with random colors */
    if ( !( strip && strip->buf ) ) {
        log_e( "led_strip_fill_random(): strip not initialized." );
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t res = ESP_OK;
    for ( size_t i = 0; i < strip->length; i++ ) {
        res = led_strip_set_pixel( strip, i, rgb_from_code( ( esp_random() & 0xFFFFFF ) ) );
        if ( res != ESP_OK ) {
            log_e( "led_strip_fill_random(): failed to set pixel %d - %s.", i, esp_err_to_name( res ) );
            return res;
        }
    }
    return res;
}

esp_err_t led_strip_clear( led_strip_t *strip, size_t num_bytes ) {
    /* sets the color of all LED's in the strip to black */
    if ( !( strip && num_bytes <= PIXEL_SIZE( strip ) ) ) {
        log_e( "led_strip_clear(): strip not initialized or buffer size out of bounds." );
        return ESP_ERR_INVALID_ARG;
    }
    memset( strip->buf, 0, num_bytes );
    return ESP_OK;
}

uint8_t led_strip_get_brightness( led_strip_t *strip ) {
    /* returns the LED strip brightness value */
    if ( !( strip && strip->buf ) ) {
        log_e( "led_strip_get_brightness(): strip not initialized." );
        return 0;
    }
    return strip->bright_act;
}

rgb_t led_strip_get_pixel( led_strip_t *strip, size_t num ) {
    /* gets the color value of a specified LED in the strip */
    rgb_t res;
    if ( !( strip && strip->buf && num <= strip->length ) ) {
        log_e( "getPixel(): invalid argument or strip not initialized." );
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
            log_e( "led_strip_get_pixel(): invalid colour order specifier." );
            res.r = res.g = res.b = 0;
    }
    return res;
}

esp_err_t led_strip_set_color_order( led_strip_t *strip, color_order_t led_order ) {
    /* sets the color order of the LED's in the strip */
    custom_color_order = led_order;
    use_custom_color_order = true; // Set the custom colour order flag
    log_d( "led_strip_set_color_order(): Setting custom color order to %s.", col_ord[ custom_color_order ] );
    log_d( "led_strip_set_color_order(): use_custom_color_order set to %s.", use_custom_color_order ? "true" : "false" );
    return ESP_OK;
}

esp_err_t led_strip_set_default_color_order( led_strip_t *strip ) {
    /* sets the color order of the LED's in the strip to the value defined by the LED type paramaters */
    custom_color_order = led_params[ strip->type ].order;
    use_custom_color_order = false; // reset the custom colour order flag
    log_d( "led_strip_set_default_color_order(): Setting custom color order to %s.", col_ord[ custom_color_order ] );
    log_d( "led_strip_set_default_color_order(): use_custom_color_order set to %s.", use_custom_color_order ? "true" : "false" );
    return ESP_OK;
}

void led_strip_debug_dump( led_strip_t *strip ) {
    /* dumps the LED strip configuration data to the serial monitor */
    if ( strip ) {
        log_d( "" );
        log_d( "=============================================" );
        log_d( "LED strip object at: %p", strip );
        log_d( "    type: %s", led_type[ strip->type ] );
        log_d( "    is_rgbw: %d", strip->is_rgbw );
        log_d( "    auto_w: %d", strip->auto_w );
        log_d( "    brightness: %d", strip->brightness );
        log_d( "    bright_act: %d", strip->bright_act );
        log_d( "    length: %d", strip->length );
        log_d( "    gpio: %d", strip->gpio );
        log_d( "    buf: %p", strip->buf );
        log_d( "    buf color size: %d", COLOR_SIZE( strip ) );
        log_d( "    buf bytes: %d", PIXEL_SIZE( strip ) );
        log_d( "    led_chan_config at: %p", &strip->stripCfg.led_chan_config );
        log_d( "        .gpio_num: %d", strip->stripCfg.led_chan_config.gpio_num );
        log_d( "        .clk_src: %d", strip->stripCfg.led_chan_config.clk_src );
        log_d( "        .resolution_hz: %d", strip->stripCfg.led_chan_config.resolution_hz );
        log_d( "        .mem_block_symbols: %d", strip->stripCfg.led_chan_config.mem_block_symbols );
        log_d( "        .trans_queue_depth: %d", strip->stripCfg.led_chan_config.trans_queue_depth );
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 2)
        log_d( "        .intr_priority: %d", strip->stripCfg.led_chan_config.intr_priority );
#endif
        log_d( "        .flags" );
        log_d( "            .invert_out: %d", strip->stripCfg.led_chan_config.flags.invert_out );
        log_d( "            .with_dma: %d", strip->stripCfg.led_chan_config.flags.with_dma );
        log_d( "            .io_loop_back: %d", strip->stripCfg.led_chan_config.flags.io_loop_back );
        log_d( "            .io_od_mode: %d", strip->stripCfg.led_chan_config.flags.io_od_mode );
        log_d( "    led_tx_config:" );
        log_d( "        .loop_count: %d", strip->stripCfg.led_tx_config.loop_count );
        log_d( "        .flags" );
        log_d( "            .queue_nonblocking: %d", strip->stripCfg.led_tx_config.flags.queue_nonblocking );
        log_d( "            .eot_level: %d", strip->stripCfg.led_tx_config.flags.eot_level );
        log_d( "    led_chan: %p", strip->stripCfg.led_chan );
        log_d( "    led_encoder_cfg: %p", strip->stripCfg.led_encoder_cfg );
        log_d( "    led_encoder: %p", strip->stripCfg.led_encoder );
        log_d( "    led_encoder chunk size: %d", strip->stripCfg.led_encoder_cfg.min_chunk_size );
        log_d( "    -----------------------------------------" );
        log_d( "    led color order: %s", col_ord[ led_params[ strip->type ].order ] );
        log_d( "    led custom color order: %s", col_ord[ custom_color_order ] );
        log_d( "    led use custom color order: %s", use_custom_color_order ? "true" : "false" );
        log_d( "LED strip object dump complete" );
        log_d( "=============================================" );
        log_d( "" );
    }
    else {
        log_d( "LED strip object is NULL." );
    }
}


//  --- EOF --- //
