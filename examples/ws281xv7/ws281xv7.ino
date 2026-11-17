// ws281xv7 example
// Using the LiteLED library
// - drives a WorldSemi WS281x strip (WS2811, WS2812B, WS2813, WS2815 and the
//   RGBW variants of the WS2813 and WS2815) with LED_STRIP_WS281XV7.
// - the timing is the one recommended by WorldSemi, with T1 and reset values
//   taken from the WorldSemi data sheets. See "WorldSemi WS281x" in Using LiteLED.md.
// - works with the RMT driver (LiteLED) on any ESP32 and with the PARLIO
//   driver (LiteLEDpio) on SoCs that have a PARLIO peripheral.

#include <LiteLED.h>

#define LED_GPIO 27     // change this number to be the GPIO pin connected to the LED strip
#define LED_COUNT 25    // number of LEDs in the strip
#define LED_BRIGHT 20   // 0 is off; 255 is full brightness
#define LED_RGBW false  // set true for the WS2813B-RGBW and WS2815B-RGBW

// Choose the driver. Comment out to use the RMT driver.
// #define USE_PARLIO

// The WS2811 and WS2811C send their colour data in RGB order. Everything else
// listed above sends GRB (and GRBW). Uncomment for a WS2811 or WS2811C.
// #define WS2811_RGB_ORDER

static const crgb_t L_RED = 0xff0000;
static const crgb_t L_GREEN = 0x00ff00;
static const crgb_t L_BLUE = 0x0000ff;

#if defined( USE_PARLIO ) && SOC_PARLIO_SUPPORTED
    #define STRIP_CLASS LiteLEDpio
#else
    #define STRIP_CLASS LiteLED
#endif

STRIP_CLASS myLED( LED_STRIP_WS281XV7, LED_RGBW );

void setup() {
    Serial.begin( 115200 );
    delay( 500 );

#ifdef WS2811_RGB_ORDER
    myLED.setOrder( ORDER_RGB );
#endif

    esp_err_t res = myLED.begin( LED_GPIO, LED_COUNT );
    if ( res != ESP_OK ) {
        Serial.printf( "begin() failed: %s\n", esp_err_to_name( res ) );
        while ( true ) {
            delay( 1000 );
        }
    }
    myLED.brightness( LED_BRIGHT );
}

void loop() {
    // Red, green and blue in turn. If the colours come out in the wrong order
    // for your strip, check its data sheet for the colour order and use setOrder().
    static const crgb_t colours[] = { L_RED, L_GREEN, L_BLUE };
    static uint8_t i = 0;

    myLED.fill( colours[ i ], true );
    i = ( i + 1 ) % 3;
    delay( 1000 );
}
