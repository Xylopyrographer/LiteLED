//
/*
    LiteLED Priority Management

    Manages RMT interrupt priority allocation for multiple concurrent displays.
    Provides fallback mechanism when requested priorities are unavailable.
*/

#ifndef __LL_PRIORITY_H__
#define __LL_PRIORITY_H__

#include <Arduino.h>
#include "esp32-hal-log.h"

// Maximum number of distinct interrupt priority levels
#define LL_MAX_PRIORITY_ATTEMPTS 4

// Priority fallback order: DEFAULT first (most compatible), then HIGH, MEDIUM, LOW
extern const int ll_priority_fallbacks[ LL_MAX_PRIORITY_ATTEMPTS ];

// Priority tracking state
extern bool ll_priority_used[ LL_MAX_PRIORITY_ATTEMPTS ];
extern uint8_t ll_active_channels;

// Check if a specific interrupt priority level is available
bool ll_is_priority_available( int priority );

// Find the best available priority, preferring the requested one
int ll_find_best_available_priority( int requested_priority );

// Mark a priority level as used (allocate)
void ll_mark_priority_used( int priority );

// Mark a priority level as free (release)
void ll_mark_priority_free( int priority );

// Reset all priority tracking (for debugging/recovery)
void ll_reset_priority_tracking( void );

// Convert priority number to human-readable string
const char *ll_priority_to_string( int priority );

#endif /* __LL_PRIORITY_H__ */

//  --- EOF --- //
