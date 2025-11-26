//
/*
    LiteLED Minimal Registry - Leverages Peripheral Manager

    Delegates GPIO tracking to the Peripheral Manager.
    We only maintain a minimal mapping needed for the deinit callback:
    RMT channel -> LiteLED instance pointer (for invalidation on GPIO reassignment)
*/

#include <Arduino.h>
#include "ll_registry.h"
#include "esp32-hal-log.h"
#include <string.h>

// Minimal tracking: only RMT channel -> instance mapping for deinit callback
typedef struct {
    rmt_channel_handle_t channel;
    LiteLED *instance;
} ll_channel_map_t;

static ll_channel_map_t g_channel_map[ LL_MAX_INSTANCES ];
static bool g_ll_periman_initialized = false;

#if !CONFIG_DISABLE_HAL_LOCKS
static SemaphoreHandle_t g_channel_map_mutex = NULL;
#define CHANNEL_MAP_LOCK() \
    do { \
        if (g_channel_map_mutex != NULL) { \
            while (xSemaphoreTake(g_channel_map_mutex, portMAX_DELAY) != pdPASS) {} \
        } \
    } while(0)
#define CHANNEL_MAP_UNLOCK() \
    do { \
        if (g_channel_map_mutex != NULL) { \
            xSemaphoreGive(g_channel_map_mutex); \
        } \
    } while(0)
#else
#define CHANNEL_MAP_LOCK()
#define CHANNEL_MAP_UNLOCK()
#endif

// Initialize LiteLED's integration with Peripheral Manager
esp_err_t ll_registry_init( void ) {
    if ( g_ll_periman_initialized ) {
        return ESP_OK;
    }

    // Initialize channel map
    memset( g_channel_map, 0, sizeof( g_channel_map ) );

#if !CONFIG_DISABLE_HAL_LOCKS
    g_channel_map_mutex = xSemaphoreCreateMutex();
    if ( g_channel_map_mutex == NULL ) {
        log_d( "LiteLED: Failed to create channel map mutex" );
        return ESP_ERR_NO_MEM;
    }
#endif

    // Set flag BEFORE registering callback to prevent re-entry issues
    g_ll_periman_initialized = true;

    // Register our deinit callback with Peripheral Manager
    perimanSetBusDeinit( ESP32_BUS_TYPE_RMT_TX, ll_periman_deinit_callback );

    log_d( "LiteLED: Peripheral Manager integration initialized" );
    return ESP_OK;
}

// Internal: Register channel -> instance mapping (called from LiteLED::begin)
esp_err_t ll_register_channel_instance( rmt_channel_handle_t channel, LiteLED* instance ) {
    if ( !channel || !instance ) {
        return ESP_ERR_INVALID_ARG;
    }

    // Ensure registry is initialized (registers deinit callback with Peripheral Manager)
    esp_err_t init_result = ll_registry_init();
    if ( init_result != ESP_OK ) {
        return init_result;
    }

    CHANNEL_MAP_LOCK();

    // Find empty slot
    for ( int i = 0; i < LL_MAX_INSTANCES; i++ ) {
        if ( g_channel_map[i].channel == NULL ) {
            g_channel_map[i].channel = channel;
            g_channel_map[i].instance = instance;
            CHANNEL_MAP_UNLOCK();
            return ESP_OK;
        }
    }

    CHANNEL_MAP_UNLOCK();
    log_d( "LiteLED: Channel map full" );
    return ESP_ERR_NO_MEM;
}

// Internal: Unregister channel mapping (called from LiteLED::free)
void ll_unregister_channel_instance( rmt_channel_handle_t channel ) {
    if ( !channel ) {
        return;
    }

    CHANNEL_MAP_LOCK();

    for ( int i = 0; i < LL_MAX_INSTANCES; i++ ) {
        if ( g_channel_map[i].channel == channel ) {
            g_channel_map[i].channel = NULL;
            g_channel_map[i].instance = NULL;
            break;
        }
    }

    CHANNEL_MAP_UNLOCK();
}

// Internal: Find instance by channel (for deinit callback)
static LiteLED* ll_find_instance_by_channel( rmt_channel_handle_t channel ) {
    if ( !channel ) {
        return NULL;
    }

    CHANNEL_MAP_LOCK();

    for ( int i = 0; i < LL_MAX_INSTANCES; i++ ) {
        if ( g_channel_map[i].channel == channel ) {
            LiteLED* instance = g_channel_map[i].instance;
            CHANNEL_MAP_UNLOCK();
            return instance;
        }
    }

    CHANNEL_MAP_UNLOCK();
    return NULL;
}

// Get LiteLED instance pointer from GPIO (queries periman)
LiteLED* ll_get_instance_by_gpio( uint8_t gpio ) {
    // Check if pin is assigned to RMT_TX with LiteLED
    if ( perimanGetPinBusType( gpio ) != ESP32_BUS_TYPE_RMT_TX ) {
        return NULL;
    }

    // Check the extra_type to verify it's LiteLED
    const char* extra_type = perimanGetPinBusExtraType( gpio );
    if ( extra_type == NULL || strcmp( extra_type, "LiteLED" ) != 0 ) {
        return NULL;
    }

    // Get the RMT channel handle from periman
    rmt_channel_handle_t channel = ( rmt_channel_handle_t )perimanGetPinBus( gpio, ESP32_BUS_TYPE_RMT_TX );
    if ( !channel ) {
        return NULL;
    }

    // Look up instance in our minimal channel map
    return ll_find_instance_by_channel( channel );
}

// Get count of active LiteLED instances (queries periman)
uint8_t ll_registry_get_active_count( void ) {
    uint8_t count = 0;

    // Iterate through all possible GPIO pins
    for ( int gpio = 0; gpio < SOC_GPIO_PIN_COUNT; gpio++ ) {
        if ( perimanGetPinBusType( gpio ) == ESP32_BUS_TYPE_RMT_TX ) {
            const char* extra_type = perimanGetPinBusExtraType( gpio );
            if ( extra_type != NULL && strcmp( extra_type, "LiteLED" ) == 0 ) {
                count++;
            }
        }
    }

    return count;
}

// Peripheral Manager deinit callback - called when GPIO is being reassigned
bool ll_periman_deinit_callback( void *bus_handle ) {
    if ( !bus_handle ) {
        log_d( "LiteLED: Deinit callback called with NULL handle" );
        return false;
    }

    rmt_channel_handle_t channel = ( rmt_channel_handle_t )bus_handle;

    // Find the LiteLED instance for this channel
    LiteLED* instance = ll_find_instance_by_channel( channel );
    if ( !instance ) {
        // Return true to allow periman to proceed - this might be an RMT channel
        // that wasn't created by LiteLED (e.g., from Arduino's internal RMT usage)
        log_d( "LiteLED: Deinit callback: No LiteLED instance found for RMT channel %p", channel );
        return true;
    }

    // Find which GPIO this channel is assigned to (for logging)
    uint8_t gpio = GPIO_NUM_NC;
    for ( int pin = 0; pin < SOC_GPIO_PIN_COUNT; pin++ ) {
        if ( perimanGetPinBusType( pin ) == ESP32_BUS_TYPE_RMT_TX ) {
            void* pin_bus_handle = perimanGetPinBus( pin, ESP32_BUS_TYPE_RMT_TX );
            if ( pin_bus_handle == bus_handle ) {
                gpio = pin;
                break;
            }
        }
    }

    log_d( "LiteLED: GPIO %u being forcibly reassigned - invalidating instance %p", gpio, instance );

    // Mark the LiteLED instance as invalid (prevents further operations)
    instance->invalidate();

    // Remove from our channel map
    ll_unregister_channel_instance( channel );

    // Note: The RMT channel cleanup will be handled by the peripheral that's
    // taking over the GPIO. We just mark our instance as invalid.

    log_d( "LiteLED: Cleanup completed for GPIO %u", gpio );
    return true;
}

//  --- EOF --- //
