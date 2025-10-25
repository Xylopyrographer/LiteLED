//
/*
    LiteLED Registry - Leverages Peripheral Manager

    This module provides tracking that Peripheral Manager doesn't handle:
    - RMT channel -> LiteLED instance mapping (for deinit callback)

    GPIO/bus tracking is handled directly by Peripheral Manager.
*/

#ifndef __LL_REGISTRY_H__
#define __LL_REGISTRY_H__

#include "LiteLED.h"
#include "esp32-hal-periman.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of concurrent LiteLED instances = max RMT channels
#define LL_MAX_INSTANCES 8

// Initialize the LiteLED integration with Peripheral Manager
esp_err_t ll_registry_init( void );

// Internal: Register channel -> instance mapping (called from LiteLED::begin)
esp_err_t ll_register_channel_instance( rmt_channel_handle_t channel, LiteLED* instance );

// Internal: Unregister channel mapping (called from LiteLED::free)
void ll_unregister_channel_instance( rmt_channel_handle_t channel );

// Get LiteLED instance from GPIO pin (queries periman + channel map)
LiteLED* ll_get_instance_by_gpio( uint8_t gpio );

// Get active instance count (queries periman)
uint8_t ll_registry_get_active_count( void );

// Peripheral Manager deinit callback - called when GPIO is reassigned
bool ll_periman_deinit_callback( void *bus_handle );

#ifdef __cplusplus
}
#endif

#endif /* __LL_REGISTRY_H__ */


//  --- EOF --- //
