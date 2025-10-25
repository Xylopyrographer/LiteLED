//
/*
    LiteLED Core Strip Operations

    Core LED strip initialization, configuration, and lifecycle management:
    - Initialization and configuration
    - Installation and resource allocation
    - Cleanup and resource deallocation
    - Data transmission (flush)
    - Debug output
*/

#ifndef __LL_STRIP_CORE_H__
#define __LL_STRIP_CORE_H__

#include "LiteLED.h"
#include "ll_led_timings.h"
#include "ll_priority.h"
#include "ll_encoder.h"
#include "esp32-hal-log.h"

// Configuration constants
#define LL_MEM_BLOCK_SIZE_DEFAULT ( ( size_t)SOC_RMT_MEM_WORDS_PER_CHANNEL )  /* Size of memory block if no DMA */
#define LL_MEM_BLOCK_SIZE_DMA 1024                                             /* Size of memory block with DMA */
#define LL_ENCODER_MIN_CHUNK_SIZE 64                                           /* Minimum RMT symbol space for encoder */

// Utility macros
#define COLOR_SIZE( strip ) ( 3 + ( (strip)->is_rgbw != 0 ) )
#define PIXEL_SIZE( strip ) ( COLOR_SIZE( strip ) * (strip)->length )

// Initialize LED strip configuration structures
esp_err_t led_strip_init( led_strip_t *strip );

// Modify strip configuration for DMA and interrupt priority
esp_err_t led_strip_init_modify( led_strip_t *strip, ll_dma_t use_dma, ll_priority_t priority );

// Install LED strip and allocate resources (buffer, RMT channel, encoder)
esp_err_t led_strip_install( led_strip_t *strip );

// Free all resources used by the strip
esp_err_t led_strip_free( led_strip_t *strip );

// Transmit LED buffer data to the strip
esp_err_t led_strip_flush( led_strip_t *strip );

// Dump strip configuration for debugging
void led_strip_debug_dump( led_strip_t *strip );

#endif /* __LL_STRIP_CORE_H__ */

//  --- EOF --- //
