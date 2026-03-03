//
/*
    LiteLED PARLIO Driver

    Umbrella header for the PARLIO-backed LED strip driver components:
        ll_led_timings.h    - Shared RMT + PARLIO timing definitions
        ll_parlio_core.h    - PARLIO strip lifecycle (init / install / flush / free)

    Including this header on a target without SOC_PARLIO_SUPPORTED will
    produce an explicit compile-time error.
*/

#ifndef __LLPARLIO_H__
    #define __LLPARLIO_H__

    #ifndef SOC_PARLIO_SUPPORTED
        #define SOC_PARLIO_SUPPORTED 0
    #endif

    #if !SOC_PARLIO_SUPPORTED
        #error "LiteLEDpio / llparlio.h: This target does not have a PARLIO peripheral (SOC_PARLIO_SUPPORTED is not set). Use LiteLED with the RMT driver instead."
    #endif

    #include "ll_led_timings.h"
    #include "ll_parlio_core.h"

#endif /* __LLPARLIO_H__ */

//  --- EOF --- //
