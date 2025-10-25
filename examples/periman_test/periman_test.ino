/*
    Example demonstrating LiteLED with Peripheral Manager integration
    Shows pin conflict detection and handling

    Note: It is not necessary to attach a display to run the example.
          Just ensure the assigned pins do not conflict with other hardware on your board.
*/

#include <LiteLED.h>

// GPIO pins for LED strips - modify these for your hardware setup
#define STRIP1_GPIO_PIN     5               // GPIO pin for first LED strip
#define STRIP2_GPIO_PIN    18               // GPIO pin for second LED strip
#define CONFLICT_TEST_PIN  STRIP1_GPIO_PIN  // GPIO pin to test conflict detection (same as STRIP1)

// LED strip configuration - modify these for your hardware setup
#define STRIP1_LED_TYPE     LED_STRIP_WS2812        // Type of LED strip for strip 1 (WS2812, APA106, etc.)
#define STRIP2_LED_TYPE     LED_STRIP_WS2812_RGB    // Type of LED strip for strip 2 (WS2812, APA106, etc.)
#define STRIP1_NUM_LEDS     10                      // Number of LEDs in first strip
#define STRIP2_NUM_LEDS     20                      // Number of LEDs in second strip

LiteLED strip1( STRIP1_LED_TYPE, false );
LiteLED strip2( STRIP2_LED_TYPE, false );

void setup() {
    Serial.begin( 115200 );
    delay( 1000 );

    Serial.println( "LiteLED Peripheral Manager Integration Test" );
    Serial.println( "=============================================" );

    // Show initial GPIO availability
    Serial.printf( "GPIO %d available: %s\n", STRIP1_GPIO_PIN, LiteLED::isGpioAvailable( STRIP1_GPIO_PIN ) ? "YES" : "NO" );
    Serial.printf( "GPIO %d available: %s\n", STRIP2_GPIO_PIN, LiteLED::isGpioAvailable( STRIP2_GPIO_PIN ) ? "YES" : "NO" );
    Serial.printf( "Active instances: %d\n", LiteLED::getActiveInstanceCount() );

    Serial.printf( "\n--- Creating first strip on GPIO %d ---\n", STRIP1_GPIO_PIN );
    esp_err_t result1 = strip1.begin( STRIP1_GPIO_PIN, STRIP1_NUM_LEDS );
    if ( result1 == ESP_OK ) {
        Serial.println( "Strip 1 initialized successfully" );
    }
    else {
        Serial.printf( "Strip 1 failed: %s\n", esp_err_to_name( result1 ) );
    }

    Serial.printf( "GPIO %d available: %s\n", STRIP1_GPIO_PIN, LiteLED::isGpioAvailable( STRIP1_GPIO_PIN ) ? "YES" : "NO" );
    Serial.printf( "Active instances: %d\n", LiteLED::getActiveInstanceCount() );

    Serial.printf( "\n--- Attempting to create second strip on same GPIO %d ---\n", CONFLICT_TEST_PIN );
    esp_err_t result2 = strip2.begin( CONFLICT_TEST_PIN, STRIP2_NUM_LEDS ); // This should fail
    if ( result2 == ESP_OK ) {
        Serial.println( "Strip 2 initialized successfully" );
    }
    else {
        Serial.printf( "Strip 2 failed (expected): %s\n", esp_err_to_name( result2 ) );
    }

    Serial.printf( "\n--- Creating second strip on GPIO %d ---\n", STRIP2_GPIO_PIN );
    result2 = strip2.begin( STRIP2_GPIO_PIN, STRIP2_NUM_LEDS );
    if ( result2 == ESP_OK ) {
        Serial.println( "Strip 2 initialized successfully" );
    }
    else {
        Serial.printf( "Strip 2 failed: %s\n", esp_err_to_name( result2 ) );
    }

    Serial.printf( "GPIO %d available: %s\n", STRIP1_GPIO_PIN, LiteLED::isGpioAvailable( STRIP1_GPIO_PIN ) ? "YES" : "NO" );
    Serial.printf( "GPIO %d available: %s\n", STRIP2_GPIO_PIN, LiteLED::isGpioAvailable( STRIP2_GPIO_PIN ) ? "YES" : "NO" );
    Serial.printf( "Active instances: %d\n", LiteLED::getActiveInstanceCount() );

    Serial.println( "\n--- Testing strip operations ---" );

    // Test strip 1
    if ( strip1.isValid() ) {
        Serial.println( "Strip 1 is valid, setting colors..." );
        strip1.fill( {255, 0, 0}, false ); // Red
        strip1.show();
        Serial.printf( "Strip 1 GPIO: %d\n", strip1.getGpioPin() );
    }

    // Test strip 2
    if ( strip2.isValid() ) {
        Serial.println( "Strip 2 is valid, setting colors..." );
        strip2.fill( {0, 255, 0}, false ); // Green
        strip2.show();
        Serial.printf( "Strip 2 GPIO: %d\n", strip2.getGpioPin() );
    }

    Serial.println( "\nSetup complete!" );
}

void loop() {
    // Cycle colors on both strips if they're valid
    static uint32_t lastUpdate = 0;
    static uint8_t colorIndex = 0;

    if ( millis() - lastUpdate > 1000 ) {
        lastUpdate = millis();

        rgb_t colors[] = {
            {255, 0, 0},    // Red
            {0, 255, 0},    // Green
            {0, 0, 255},    // Blue
            {255, 255, 0},  // Yellow
            {255, 0, 255},  // Magenta
            {0, 255, 255}   // Cyan
        };

        if ( strip1.isValid() ) {
            strip1.fill( colors[ colorIndex ], true );
        }

        if ( strip2.isValid() ) {
            strip2.fill( colors[ ( colorIndex + 3 ) % 6 ], true ); // Offset colors
        }

        colorIndex = ( colorIndex + 1 ) % 6;

        // Periodically show status
        if ( colorIndex == 0 ) {
            Serial.printf( "Status - Active instances: %d, Strip1 valid: %s, Strip2 valid: %s\n",
                           LiteLED::getActiveInstanceCount(),
                           strip1.isValid() ? "YES" : "NO",
                           strip2.isValid() ? "YES" : "NO" );
        }
    }
}