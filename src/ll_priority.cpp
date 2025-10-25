//
/*
    LiteLED RMT Interrupt Priority Management Implementation
*/

#include "ll_priority.h"

// Priority fallback order: DEFAULT first (most compatible), then others
const int ll_priority_fallbacks[ LL_MAX_PRIORITY_ATTEMPTS ] = { 0, 1, 2, 3 };

// Priority tracking state
bool ll_priority_used[ LL_MAX_PRIORITY_ATTEMPTS ] = { false, false, false, false };
uint8_t ll_active_channels = 0;

bool ll_is_priority_available( int priority ) {
    /* Checks if a specific interrupt priority is available */
    if ( priority < 0 || priority >= LL_MAX_PRIORITY_ATTEMPTS ) {
        return false;
    }
    return !ll_priority_used[ priority ];
}

int ll_find_best_available_priority( int requested_priority ) {
    /* Finds the best available priority, preferring the requested one */
    // First check if requested priority is available
    if ( ll_is_priority_available( requested_priority ) ) {
        return requested_priority;
    }

    // Try fallback priorities in order
    for ( int i = 0; i < LL_MAX_PRIORITY_ATTEMPTS; i++ ) {
        if ( ll_is_priority_available( ll_priority_fallbacks[ i ] ) ) {
            return ll_priority_fallbacks[ i ];
        }
    }

    // No priority available
    return -1;
}

void ll_mark_priority_used( int priority ) {
    /* Marks a priority as used */
    if ( priority >= 0 && priority < LL_MAX_PRIORITY_ATTEMPTS ) {
        ll_priority_used[ priority ] = true;
        ll_active_channels++;
        log_d( "Priority %s (%d) marked as used. Active channels: %d",
               ll_priority_to_string( priority ), priority, ll_active_channels );
    }
}

void ll_mark_priority_free( int priority ) {
    /* Marks a priority as free */
    if ( priority >= 0 && priority < LL_MAX_PRIORITY_ATTEMPTS && ll_priority_used[ priority ] ) {
        ll_priority_used[ priority ] = false;
        if ( ll_active_channels > 0 ) {
            ll_active_channels--;
        }
        log_d( "Priority %s (%d) marked as free. Active channels: %d",
               ll_priority_to_string( priority ), priority, ll_active_channels );
    }
}

void ll_reset_priority_tracking( void ) {
    /* Resets all priority tracking - useful for debugging or error recovery */
    for ( int i = 0; i < LL_MAX_PRIORITY_ATTEMPTS; i++ ) {
        ll_priority_used[ i ] = false;
    }
    ll_active_channels = 0;
    log_d( "Priority tracking reset. All priorities now available." );
}

const char *ll_priority_to_string( int priority ) {
    /* Converts interrupt priority number to human-readable string */
    switch ( priority ) {
        case 0:
            return "DEFAULT";
        case 1:
            return "HIGH";
        case 2:
            return "MEDIUM";
        case 3:
            return "LOW";
        default:
            return "UNKNOWN";
    }
}

//  --- EOF --- //
