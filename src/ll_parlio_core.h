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

// Include LiteLED.h unconditionally so that soc_caps.h is brought into scope
// (via Arduino.h) and SOC_PARLIO_SUPPORTED is resolved BEFORE the #if check
// below.  Without this, ll_parlio_core.cpp sees SOC_PARLIO_SUPPORTED=0 when
// this is its first include, and the entire implementation compiles to nothing.
#include "LiteLED.h"
#include "ll_led_timings.h"
#include "esp32-hal-log.h"

#if SOC_PARLIO_SUPPORTED

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

// ---- Multi-strip group functions (used by LiteLEDpioGroup) ----------

// Allocate per-lane pixel buffers and the shared DMA bitstream buffer,
// then create and enable the PARLIO TX unit with all assigned lane GPIOs.
// cfg->lanes[n].assigned and cfg->lanes[n].strip must be pre-populated
// by LiteLEDpioGroup before calling this.
esp_err_t parlio_group_install( parlio_group_cfg_t *cfg );

// Encode all assigned lane pixel buffers into the shared DMA buffer
// (applying per-lane brightness), then transmit and block until done.
esp_err_t parlio_group_flush( parlio_group_cfg_t *cfg );

// Wait for any in-progress transfer, disable and delete the PARLIO TX
// unit, and free all per-lane pixel buffers and the DMA buffer.
esp_err_t parlio_group_free( parlio_group_cfg_t *cfg );

// ---- PARLIO Peripheral Manager integration -------------------------

// Resolved bus type token for periman.
// ESP32_BUS_TYPE_PARLIO_TX is not yet defined in any released
// arduino-esp32 version (confirmed through 3.3.8).  When it is added
// upstream, the dedicated path becomes active automatically.
#ifdef ESP32_BUS_TYPE_PARLIO_TX
    #define LL_PARLIO_BUS_TYPE  ESP32_BUS_TYPE_PARLIO_TX
#else
    #define LL_PARLIO_BUS_TYPE  ESP32_BUS_TYPE_GPIO
#endif

// Returns the bus handle to pass to perimanSetPinBus().
//
// When a dedicated PARLIO bus type exists, we store the channel handle
// so the registered deinit callback can identify the owning instance.
//
// When falling back to ESP32_BUS_TYPE_GPIO, we store NULL.  periman
// only invokes the deinit callback when the stored handle is non-NULL
// (see perimanSetPinBus implementation), so passing NULL avoids the
// need to register any deinit callback at all — which prevents LiteLED
// from overwriting the GPIO layer's own gpioDetachBus callback.
static inline void *ll_parlio_bus_handle( parlio_tx_unit_handle_t chan ) {
    #ifdef ESP32_BUS_TYPE_PARLIO_TX
    return ( void * )chan;
    #else
    ( void )chan;
    return NULL;
    #endif
}

// Maximum number of concurrently active PARLIO TX instances tracked
// by the deinit registry.  Only used on the dedicated-type path.
// Current hardware: ESP32-C6 / H2 = 1, ESP32-P4 = 2.  4 is a safe ceiling.
#define LL_PARLIO_MAX_INSTANCES  4

// Called from begin() after a successful perimanSetPinBus().
// On the dedicated-type path: registers the periman deinit callback
// (idempotent) and adds chan → valid_flag to the instance registry.
// On the GPIO-fallback path: no-op (NULL bus handle means periman
// will never invoke a deinit callback, so no registration is needed
// and gpioDetachBus is left undisturbed).
void ll_parlio_periman_begin( parlio_tx_unit_handle_t chan,
                              bool *valid_flag );

// Called from free() / _free() as a cleanup safety net.
// On the dedicated-type path: removes the registry entry.
// On the GPIO-fallback path: no-op.
void ll_parlio_periman_end( parlio_tx_unit_handle_t chan );

// ---- Internal helpers (used only within ll_parlio_core.cpp) --------

void ll_parlio_register_instance( parlio_tx_unit_handle_t chan,
                                  bool *valid_flag );
void ll_parlio_unregister_instance( parlio_tx_unit_handle_t chan );
bool ll_parlio_deinit_cb( void *bus_handle );

#endif /* SOC_PARLIO_SUPPORTED */

#endif /* __LL_PARLIO_CORE_H__ */

//  --- EOF --- //
