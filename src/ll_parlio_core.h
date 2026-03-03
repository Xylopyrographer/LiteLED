//
/*
    LiteLED PARLIO Core Operations

    Provides the install / flush / free lifecycle for the PARLIO-backed
    LED strip driver.  The pixel colour buffer is owned by the calling
    led_strip_t (same struct as the RMT path); only the hardware config
    and DMA bitstream buffer live here.

    Design notes
    ------------
    * Clock: 2.5 MHz (400 ns per PARLIO sample), 3 samples per LED bit.
    * Bit encoding:
         LED data 0  →  PARLIO samples [H, L, L]  (pattern 0b100)
         LED data 1  →  PARLIO samples [H, H, L]  (pattern 0b110)
      Each input byte (8 bits) expands exactly to 3 output bytes; alignment
      is always on a byte boundary.
    * Reset: PARLIO_RESET_BYTES zero-bytes appended to every DMA transfer
      (400 µs LOW), satisfying all currently supported LED types.
    * Brightness scaling is applied when encoding (at flush time), matching
      the RMT encoder callback behaviour.
    * DMA buffer is allocated from internal DMA-capable RAM regardless of
      the PSRAM preference for the pixel colour buffer.
*/

#ifndef __LL_PARLIO_CORE_H__
    #define __LL_PARLIO_CORE_H__

    #if SOC_PARLIO_SUPPORTED

        #include "LiteLED.h"
        #include "ll_led_timings.h"
        #include "esp32-hal-log.h"

        // -------------------------------------------------------------------------
        // Function declarations
        // -------------------------------------------------------------------------

        // Validate arguments and zero the parlio_strip_cfg_t handles.
        esp_err_t parlio_strip_init( led_strip_t *strip, parlio_strip_cfg_t *cfg );

        // Allocate pixel buffer (respects strip->use_psram) and DMA bitstream
        // buffer, create and enable the PARLIO TX channel.
        esp_err_t parlio_strip_install( led_strip_t *strip, parlio_strip_cfg_t *cfg );

        // Wait for any in-progress transfer to finish, disable and delete the
        // PARLIO TX channel, and free both buffers.
        esp_err_t parlio_strip_free( led_strip_t *strip, parlio_strip_cfg_t *cfg );

        // Encode pixel colour buffer → DMA bitstream (applying brightness), then
        // send via PARLIO DMA and block until the transfer (including reset) is done.
        esp_err_t parlio_strip_flush( led_strip_t *strip, parlio_strip_cfg_t *cfg );

        // Dump PARLIO strip configuration to the debug log.
        void parlio_strip_debug_dump( led_strip_t *strip, parlio_strip_cfg_t *cfg );

    #endif /* SOC_PARLIO_SUPPORTED */

#endif /* __LL_PARLIO_CORE_H__ */

//  --- EOF --- //
