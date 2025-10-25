//
/*
    LiteLED RMT Encoder Callback

    The encoder callback is called by the RMT driver when it needs more symbols to send.
    It pulls bytes from the LED data buffer and encodes them into RMT symbols with
    brightness scaling applied.
*/

#ifndef __LL_ENCODER_H__
#define __LL_ENCODER_H__

#include "LiteLED.h"
#include "llrgb.h"
#include "ll_led_timings.h"

/*
    The encoder_callback() function is called by the RMT driver when it needs more symbols to send.
    The encoder_callback() pulls bytes of data from the LED data buffer and encodes them into RMT symbols,
    which are then stored into the allocated RMT memory block. The RMT driver then pulls the symbols
    from the memory block and sends them to the GPIO pin via the RMT hardware peripheral.

    Parameters:
        data            - [in] LED buffer data (provided via the "simple_encoder" handle)
        data_size       - [in] Size of LED buffer (provided via the "simple_encoder" handle)
        symbols_written - [in] Number of symbols already written (provided by the RMT driver)
        symbols_free    - [in] Number of free symbol spaces available (provided by the RMT driver)
        symbols         - [out] Output buffer for RMT symbols
        done            - [out] Indicates end of transmission
        arg             - [in] User argument (led_strip_t pointer, provided via the "simple_encoder" handle)

    Returns:
        Number of symbols written to the output buffer
*/
IRAM_ATTR size_t led_encoder_cb( const void* data, size_t data_size,
                                  size_t symbols_written, size_t symbols_free,
                                  rmt_symbol_word_t *symbols, bool *done, void *arg );

#endif /* __LL_ENCODER_H__ */

//  --- EOF --- //
