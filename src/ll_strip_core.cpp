//
/*
    LiteLED Core Strip Operations Implementation
*/

#include "ll_strip_core.h"

esp_err_t led_strip_init( led_strip_t *strip ) {
    /* Initializes all structures and variables required for the library */
    if ( !( strip && strip->length > 0 && strip->type < LED_STRIP_TYPE_MAX ) ) {
        log_d( "Error: Invalid arguments." );
        return ESP_ERR_INVALID_ARG;
    }

    // Populate the stripCfg struct with the necessary values
    strip->stripCfg.led_chan = NULL;
    strip->stripCfg.led_encoder = NULL;
    strip->stripCfg.led_chan_config = {
        .gpio_num = ( gpio_num_t )strip->gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .mem_block_symbols = LL_MEM_BLOCK_SIZE_DEFAULT,
        .trans_queue_depth = 4,  /* Number of transactions that can be pending in the background */
#if LL_INT_PRIORITY_SUPPORT
        .intr_priority = 0,  /* Callback interrupt priority, default is 0 */
#endif
        .flags = {
            .invert_out = 0,            /* Do not invert output */
            .with_dma = DMA_DEFAULT,    /* Do not use DMA by default */
            .io_loop_back = 0,          /* No loopback */
            .io_od_mode = 0,            /* Open drain */
        }
    };

    log_d( "Defining the led encoder configuration." );
    strip->stripCfg.led_encoder_cfg = {
        led_encoder_cb,                         /* The led encoder function */
        strip,                                  /* Args for the led encoder function */
        ( size_t )LL_ENCODER_MIN_CHUNK_SIZE,    /* Explicitly set - default is 64 */
    };

    log_d( "Setting the RMT transmit configuration." );
    strip->stripCfg.led_tx_config = {
        .loop_count = 0,
        .flags = {
            .eot_level = 0,
            .queue_nonblocking = 0,  // Use blocking mode for reliability
        }
    };

    log_d( "RMT driver configuration successful." );
    return ESP_OK;
}

esp_err_t led_strip_init_modify( led_strip_t *strip, ll_dma_t use_dma, ll_priority_t priority ) {
    /* Sets user defined DMA and interrupt priority settings for the LED strip */
    esp_err_t res = ESP_OK;

#if LL_INT_PRIORITY_SUPPORT
    strip->stripCfg.led_chan_config.intr_priority = priority;
    log_d( "Setting the RMT interrupt priority to %d.", priority );
#else
    // Interrupt priority setting not supported - use default (0)
    // This is not an error condition, just a limitation
    log_d( "Setting RMT interrupt priority not supported with this core version. Using default." );
#endif

#if LL_DMA_SUPPORT
    strip->stripCfg.led_chan_config.flags.with_dma = use_dma;
    log_d( "Setting the RMT DMA usage to %s.", use_dma ? "ON" : "OFF" );
    if ( use_dma ) {
        strip->stripCfg.led_chan_config.mem_block_symbols = ( size_t )LL_MEM_BLOCK_SIZE_DMA;
        log_d( "RMT DMA enabled. Setting the memory block size to %d.", strip->stripCfg.led_chan_config.mem_block_symbols );
    }
    else {
        strip->stripCfg.led_chan_config.mem_block_symbols = ( size_t )LL_MEM_BLOCK_SIZE_DEFAULT;
        log_d( "RMT DMA disabled. Setting the memory block size to %d.", strip->stripCfg.led_chan_config.mem_block_symbols );
    }
#else
    strip->stripCfg.led_chan_config.flags.with_dma = DMA_OFF;
    strip->stripCfg.led_chan_config.mem_block_symbols = LL_MEM_BLOCK_SIZE_DEFAULT;
    log_d( "RMT DMA not supported on this ESP32 model. Disabling DMA." );
#endif

    return res;
}

esp_err_t led_strip_install( led_strip_t *strip ) {
    /* Installs the LED strip and allocates the necessary resources */
    size_t buffer_size = strip->length * COLOR_SIZE( strip );

    // Allocate buffer based on PSRAM preference
    if ( strip->use_psram ) {
#if CONFIG_SPIRAM
        // Check if PSRAM is actually available at runtime
        if ( psramFound() ) {
            // Try to allocate in PSRAM first
            strip->buf = ( uint8_t* )heap_caps_calloc( strip->length, COLOR_SIZE( strip ), MALLOC_CAP_SPIRAM );
            if ( strip->buf ) {
                log_d( "LED buffer allocated in PSRAM (%d bytes)", buffer_size );
            }
            else {
                // Fall back to internal RAM if PSRAM allocation fails
                log_d( "PSRAM allocation failed, falling back to internal RAM" );
                strip->buf = ( uint8_t* )calloc( strip->length, COLOR_SIZE( strip ) );
                if ( strip->buf ) {
                    log_d( "LED buffer allocated in internal RAM (%d bytes)", buffer_size );
                }
            }
        }
        else {
            // PSRAM compiled in but not available at runtime
            log_d( "PSRAM requested but not found at runtime, using internal RAM" );
            strip->buf = ( uint8_t* )calloc( strip->length, COLOR_SIZE( strip ) );
            if ( strip->buf ) {
                log_d( "LED buffer allocated in internal RAM (%d bytes)", buffer_size );
            }
        }
#else
        // PSRAM not compiled in
        log_d( "PSRAM requested but support not compiled in, using internal RAM" );
        strip->buf = ( uint8_t* )calloc( strip->length, COLOR_SIZE( strip ) );
        if ( strip->buf ) {
            log_d( "LED buffer allocated in internal RAM (%d bytes)", buffer_size );
        }
#endif
    }
    else {
        // Use regular internal RAM allocation
        strip->buf = ( uint8_t* )calloc( strip->length, COLOR_SIZE( strip ) );
        if ( strip->buf ) {
            log_d( "LED buffer allocated in internal RAM (%d bytes)", buffer_size );
        }
    }

    if ( !strip->buf ) {
        log_d( "Error: Failed to allocate buffer - ESP_ERR_NO_MEM." );
        return ESP_ERR_NO_MEM;
    }

    esp_err_t res = ESP_OK;

#if LL_INT_PRIORITY_SUPPORT
    int original_priority = strip->stripCfg.led_chan_config.intr_priority;

    // Pre-flight conflict detection
    log_d( "Checking interrupt priority availability for priority %s (%d)",
           ll_priority_to_string( original_priority ), original_priority );

    int best_priority = ll_find_best_available_priority( original_priority );
    if ( best_priority < 0 ) {
        log_d( "Error: No interrupt priorities available! Maximum number of RMT channels may be exceeded" );
        log_d( "       Active channels: %d. May need to reduce the number of concurrent displays or reduce RMT DMA usage", ll_active_channels );
        return ESP_ERR_NO_MEM;
    }

    if ( best_priority != original_priority ) {
        log_d( "Requested priority %s (%d) not available, using %s (%d) instead",
               ll_priority_to_string( original_priority ), original_priority,
               ll_priority_to_string( best_priority ), best_priority );
        strip->stripCfg.led_chan_config.intr_priority = best_priority;
    }
    else {
        log_d( "Requested priority %s (%d) is available",
               ll_priority_to_string( original_priority ), original_priority );
    }
#endif

    // Try to create RMT TX channel with the selected priority
    res = rmt_new_tx_channel( &strip->stripCfg.led_chan_config, &strip->stripCfg.led_chan );

    // If we still get a conflict despite pre-flight checks, use the original fallback mechanism
    if ( res == ESP_ERR_INVALID_ARG ) {
#if LL_INT_PRIORITY_SUPPORT
        log_d( "Error: Unexpected priority conflict after pre-flight checks. Attempting to fallback..." );

        for ( int i = 0; i < LL_MAX_PRIORITY_ATTEMPTS; i++ ) {
            if ( ll_priority_fallbacks[ i ] != strip->stripCfg.led_chan_config.intr_priority ) {
                strip->stripCfg.led_chan_config.intr_priority = ll_priority_fallbacks[ i ];
                res = rmt_new_tx_channel( &strip->stripCfg.led_chan_config, &strip->stripCfg.led_chan );

                if ( res == ESP_OK ) {
                    log_d( "Fallback successful with priority %s (%d)",
                           ll_priority_to_string( ll_priority_fallbacks[ i ] ), ll_priority_fallbacks[ i ] );
                    break;
                }
                else if ( res != ESP_ERR_INVALID_ARG ) {
                    log_d( "Error: Fallback priority %s (%d) failed with error: %s",
                           ll_priority_to_string( ll_priority_fallbacks[ i ] ), ll_priority_fallbacks[ i ],
                           esp_err_to_name( res ) );
                    break;
                }
            }
        }
#endif
    }

    // Mark the priority as used if successful
    if ( res == ESP_OK ) {
#if LL_INT_PRIORITY_SUPPORT
        ll_mark_priority_used( strip->stripCfg.led_chan_config.intr_priority );
#endif
        log_d( "RMT TX channel created successfully with priority %s (%d)",
               ll_priority_to_string( strip->stripCfg.led_chan_config.intr_priority ),
               strip->stripCfg.led_chan_config.intr_priority );
    }

    if ( res != ESP_OK ) {
        if ( res == ESP_ERR_INVALID_ARG ) {
            log_d( "Error: Failed to create RMT TX channel - unable to resolve interrupt priority conflict." );
            log_d( "       Solutions: 1) Use PRIORITY_DEFAULT for all displays, 2) Reduce number of concurrent displays, 3) Use simple begin() method" );
        }
        else {
            log_d( "Error: Failed to create RMT TX channel - %s.", esp_err_to_name( res ) );
        }
        return res;
    }

    if ( ( res = rmt_new_simple_encoder( &strip->stripCfg.led_encoder_cfg, &strip->stripCfg.led_encoder ) ) != ESP_OK ) {
        log_d( "Error: Failed to create LED encoder - %s.", esp_err_to_name( res ) );
        return res;
    }

    log_d( "Enabling the RMT TX channel." );
    if ( ( res = rmt_enable( strip->stripCfg.led_chan ) ) != ESP_OK ) {
        log_d( "Error: Failed to enable RMT TX channel - %s.", esp_err_to_name( res ) );
        return res;
    }

    log_d( "LED strip sucessfully configured and installed." );

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
    led_strip_debug_dump( strip );
#endif

    return res;
}

esp_err_t led_strip_free( led_strip_t *strip ) {
    /* Deletes all resources used by the strip */
    if ( !( strip && strip->buf ) ) {
        log_d( "Error: Attempting to free uninitialized strip." );
        return ESP_ERR_INVALID_ARG;
    }

#if LL_INT_PRIORITY_SUPPORT
    // Release the interrupt priority before freeing resources
    int priority = strip->stripCfg.led_chan_config.intr_priority;
    ll_mark_priority_free( priority );
#endif

    esp_err_t res = ESP_OK;
    if ( ( res = rmt_tx_wait_all_done( strip->stripCfg.led_chan, portMAX_DELAY ) ) != ESP_OK ) {
        log_d( "Error: Fail on wait for RMT TX to finish - %s.", esp_err_to_name( res ) );
        return res;
    }
    if ( ( res = rmt_disable( strip->stripCfg.led_chan ) ) != ESP_OK ) {
        log_d( "Error: Fail on disable RMT TX channel - %s.", esp_err_to_name( res ) );
        return res;
    }
    if ( ( rmt_del_channel( strip->stripCfg.led_chan ) ) != ESP_OK ) {
        log_d( "Error: Fail on delete RMT TX channel - %s.", esp_err_to_name( res ) );
        return res;
    }
    if ( ( rmt_del_encoder( strip->stripCfg.led_encoder ) ) != ESP_OK ) {
        log_d( "Error: Fail on delete RMT encoder - %s.", esp_err_to_name( res ) );
        return res;
    }

    free( strip->buf );
    strip->buf = NULL;
    return res;
}

esp_err_t led_strip_flush( led_strip_t *strip ) {
    /* Pushes all data from the LED buffer to the LED strip */
    esp_err_t res = ESP_OK;
    if ( ( res = rmt_transmit( strip->stripCfg.led_chan, strip->stripCfg.led_encoder, strip->buf, PIXEL_SIZE( strip ), &strip->stripCfg.led_tx_config ) ) != ESP_OK ) {
        log_d( "Error: Fail on 'rmt_transmit()'. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    if ( ( res = rmt_tx_wait_all_done( strip->stripCfg.led_chan, portMAX_DELAY ) ) != ESP_OK ) {
        log_d( "Error: Fail on 'rmt_tx_wait_all_done()'. Result = %s", esp_err_to_name( res ) );
        return res;
    }
    return res;
}

void led_strip_debug_dump( led_strip_t *strip ) {
    /* Dumps the LED strip configuration data to the debug monitor */
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
    if ( strip ) {
        log_printf( "\n" );
        log_printf( "============= LiteLED Debug Report =============\n" );
        log_printf( "LED strip object at: %p\n", strip );
        log_printf( "    type: %s\n", led_type[ strip->type ] );
        log_printf( "    is_rgbw: %d\n", strip->is_rgbw );
        log_printf( "    auto_w: %d\n", strip->auto_w );
        log_printf( "    brightness: %d\n", strip->brightness );
        log_printf( "    bright_act: %d\n", strip->bright_act );
        log_printf( "    length: %d\n", strip->length );
        log_printf( "    gpio: %d\n", strip->gpio );
        log_printf( "    buf: %p\n", strip->buf );
        log_printf( "    buf color size: %d\n", COLOR_SIZE( strip ) );
        log_printf( "    buf bytes: %d\n", PIXEL_SIZE( strip ) );
        // Check if buffer is in PSRAM or internal RAM
        if ( strip->buf ) {
#if defined(SOC_EXTRAM_DATA_LOW) && defined(SOC_EXTRAM_DATA_HIGH)
            bool is_psram = heap_caps_get_allocated_size( strip->buf ) > 0 &&
                            ( ( ( uint32_t )strip->buf >= SOC_EXTRAM_DATA_LOW ) &&
                              ( ( uint32_t )strip->buf < SOC_EXTRAM_DATA_HIGH ) );
            log_printf( "    buf location: %s\n", is_psram ? "PSRAM" : "Internal RAM" );
#else
            // ESP32-C3 and other variants without external RAM support
            log_printf( "    buf location: Internal RAM (no PSRAM support)\n" );
#endif
        }
        log_printf( "    led_chan_config at: %p\n", &strip->stripCfg.led_chan_config );
        log_printf( "        .gpio_num: %d\n", strip->stripCfg.led_chan_config.gpio_num );
        log_printf( "        .clk_src: %d\n", strip->stripCfg.led_chan_config.clk_src );
        log_printf( "        .resolution_hz: %d\n", strip->stripCfg.led_chan_config.resolution_hz );
        log_printf( "        .mem_block_symbols: %d\n", strip->stripCfg.led_chan_config.mem_block_symbols );
        log_printf( "        .trans_queue_depth: %d\n", strip->stripCfg.led_chan_config.trans_queue_depth );
#if LL_INT_PRIORITY_SUPPORT
        log_printf( "        .intr_priority: %d\n", strip->stripCfg.led_chan_config.intr_priority );
#endif
        log_printf( "        .flags\n" );
        log_printf( "            .invert_out: %d\n", strip->stripCfg.led_chan_config.flags.invert_out );
        log_printf( "            .with_dma: %d\n", strip->stripCfg.led_chan_config.flags.with_dma );
        log_printf( "            .io_loop_back: %d\n", strip->stripCfg.led_chan_config.flags.io_loop_back );
        log_printf( "            .io_od_mode: %d\n", strip->stripCfg.led_chan_config.flags.io_od_mode );
        log_printf( "    led_tx_config:\n" );
        log_printf( "        .loop_count: %d\n", strip->stripCfg.led_tx_config.loop_count );
        log_printf( "        .flags\n" );
        log_printf( "            .queue_nonblocking: %d\n", strip->stripCfg.led_tx_config.flags.queue_nonblocking );
        log_printf( "            .eot_level: %d\n", strip->stripCfg.led_tx_config.flags.eot_level );
        log_printf( "    led_chan: %p\n", strip->stripCfg.led_chan );
        log_printf( "    led_encoder_cfg: %p\n", &strip->stripCfg.led_encoder_cfg );
        log_printf( "    led_encoder: %p\n", strip->stripCfg.led_encoder );
        log_printf( "    led_encoder chunk size: %d\n", strip->stripCfg.led_encoder_cfg.min_chunk_size );
        log_printf( "    -----------------------------------------\n" );
        log_printf( "    led color order: %s\n", col_ord[ led_params[ strip->type ].order ] );
        log_printf( "    led custom color order: %s\n", col_ord[ custom_color_order ] );
        log_printf( "    led use custom color order: %s\n", use_custom_color_order ? "true" : "false" );
        log_printf( "    -----------------------------------------\n" );
        log_printf( "    interrupt priority status (total active: %d):\n", ll_active_channels );
        for ( int i = 0; i < LL_MAX_PRIORITY_ATTEMPTS; i++ ) {
            log_printf( "      %s (%d): %s\n",
                        ll_priority_to_string( i ), i,
                        ll_priority_used[ i ] ? "USED" : "FREE" );
        }
        log_printf( "================================================\n" );
        log_printf( "\n" );
    }
    else {
        log_printf( "Error: LED strip object is NULL." );
    }
#endif
}

//  --- EOF --- //
