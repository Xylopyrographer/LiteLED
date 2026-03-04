/*
    rainbow_parlio.ino — LiteLED PARLIO driver example

    Displays a scrolling rainbow across a strip of WS2812-compatible LEDs
    using the LiteLEDpio class, which is backed by the ESP32 PARLIO
    (Parallel IO) peripheral instead of the RMT peripheral.

    Supported chips
    ---------------
    The PARLIO peripheral is available on:
      • ESP32-C6
      • ESP32-H2
      • ESP32-P4

    The PARLIO peripheral is NOT available on the original ESP32, ESP32-S2,
    ESP32-S3, or ESP32-C3. Use the standard LiteLED class on those chips.

    Hardware
    --------
    - One data pin wired to the DIN of your LED strip.
    - LED_GPIO and LED_COUNT below to match your wiring and strip length.

    Usage notes
    -----------
    LiteLEDpio has the same API as LiteLED so you can swap one for the
    other with a single-line change. The only difference is the class name:

        LiteLED    strip(LED_STRIP_WS2812, false);   // RMT  — all ESP32 chips
        LiteLEDpio strip(LED_STRIP_WS2812, false);   // PARLIO — C6 / H2 / P4

    Only one LiteLEDpio instance may be active at a time because the ESP32-C6
    and ESP32-H2 each have a single PARLIO TX unit.
*/

#include <LiteLED.h>

// ---------------------------------------------------------------------------
// Configuration — adjust to match your hardware
// ---------------------------------------------------------------------------
#define LED_GPIO    21          // data pin connected to strip DIN
#define LED_COUNT   30          // number of LEDs in your strip

// ---------------------------------------------------------------------------
// Create the PARLIO-backed strip
// Replace LiteLEDpio with LiteLED to switch to the RMT driver
// ---------------------------------------------------------------------------
LiteLEDpio strip( LED_STRIP_WS2812, false );

// ---------------------------------------------------------------------------
// HSV → RGB helper
// ---------------------------------------------------------------------------
crgb_t HSVtoRGB( uint8_t h, uint8_t s, uint8_t v ) {
    if ( s == 0 ) {
        return ( ( uint32_t )v << 16 ) | ( ( uint32_t )v << 8 ) | v;
    }

    uint8_t region    = h / 43;
    uint8_t remainder = ( h - ( region * 43 ) ) * 6;
    uint8_t p = ( v * ( 255 - s ) ) >> 8;
    uint8_t q = ( v * ( 255 - ( ( s * remainder ) >> 8 ) ) ) >> 8;
    uint8_t t = ( v * ( 255 - ( ( s * ( 255 - remainder ) ) >> 8 ) ) ) >> 8;
    uint8_t r, g, b;

    switch ( region ) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        default:
            r = v;
            g = p;
            b = q;
            break;
    }

    return ( ( uint32_t )r << 16 ) | ( ( uint32_t )g << 8 ) | b;
}

// ---------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin( 115200 );

    esp_err_t err = strip.begin( LED_GPIO, LED_COUNT );
    if ( err != ESP_OK ) {
        Serial.printf( "LiteLEDpio begin() failed: %s\n", esp_err_to_name( err ) );
        while ( true ) {
            delay( 1000 );
        }
    }

    strip.brightness( 50 );
    strip.clear( true );
}

// ---------------------------------------------------------------------------
// loop — scrolling rainbow
// ---------------------------------------------------------------------------
void loop() {
    static uint8_t hue = 0;

    for ( size_t i = 0; i < LED_COUNT; i++ ) {
        uint8_t pixelHue = hue + ( i * 255 / LED_COUNT );
        strip.setPixel( i, HSVtoRGB( pixelHue, 255, 255 ) );
    }

    strip.show();
    hue += 2;
    delay( 20 );
}
