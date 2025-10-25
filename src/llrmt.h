//
/*
    ESP32 RMT-based driver for various types of RGB LED strips

    Header that includes the modular components:
    - ll_led_timings.h   : LED timing definitions for various strip types
    - ll_priority.h      : Interrupt priority management
    - ll_encoder.h       : RMT encoder callback
    - ll_strip_core.h    : Core strip initialization and lifecycle
    - ll_strip_pixels.h  : Pixel manipulation operations
*/

#ifndef __LLRMT_H__
#define __LLRMT_H__

// Include all modular components
#include "ll_led_timings.h"
#include "ll_priority.h"
#include "ll_encoder.h"
#include "ll_strip_core.h"
#include "ll_strip_pixels.h"

/*
    All functionality is now provided through the modular headers above.
    This file maintains backward compatibility for existing code that includes llrmt.h
*/

#endif /* __LLRMT_H__ */

//  --- EOF --- //
