// custom_timing example
// Using the LiteLED library
// - drives an LED type that is not one of the built-in LED_STRIP_* types by
//   supplying your own bit timing (T0H, T0L, T1H, T1L) and reset time.
// - works with the RMT driver (LiteLED) on any ESP32 and with the PARLIO
//   driver (LiteLEDpio) on SoCs that have a PARLIO peripheral.
//
// Take the timing values from the LED's datasheet. All times are in nanoseconds
// except reset_us, which is in microseconds.

#include <LiteLED.h>

#define LED_GPIO 27     // change this number to be the GPIO pin connected to the LED strip
#define LED_COUNT 25    // number of LEDs in the strip
#define LED_BRIGHT 20   // 0 is off; 255 is full brightness

// Choose the driver. Comment out to use the RMT driver.
// #define USE_PARLIO

static const crgb_t L_RED = 0xff0000;
static const crgb_t L_GREEN = 0x00ff00;
static const crgb_t L_BLUE = 0x0000ff;

#if defined( USE_PARLIO ) && SOC_PARLIO_SUPPORTED
    #define STRIP_CLASS LiteLEDpio
#else
    #define STRIP_CLASS LiteLED
#endif

// The timing for the LED. Take the values from the LED's datasheet. These are
// example values: a 0 bit is 325 ns high then 925 ns low, a 1 bit is 925 ns
// high then 325 ns low, 1250 ns per bit. All times are in ns except reset_us.
//
// Define this above the strip object so it is initialised first. The members
// must be listed in this order. Members left off the end keep their defaults.
static const ll_custom_timing_t timing = {
    .t0h_ns   = 325,
    .t0l_ns   = 925,
    .t1h_ns   = 925,
    .t1l_ns   = 325,
    .reset_us = 280
    // Optional, add after reset_us:
    //
    // RMT driver only. The RMT tick length is 1 / resolution_hz. The default is
    // 40 MHz (a 25 ns tick). Each time above is rounded to the nearest tick.
    // , .resolution_hz = 40000000
    //
    // PARLIO driver only. The driver picks the lowest PARLIO clock at which each of
    // T0H, T0L, T1H and T1L is within this many ns of the value requested. A smaller
    // number selects a faster clock (more accurate, but uses more DMA memory).
    // , .parlio_tolerance_ns = 25
};

// Custom timing needs the constructor that takes the timing, the colour order and the RGBW flag
STRIP_CLASS myLED( timing, ORDER_GRB, false );

void setup() {
    Serial.begin( 115200 );
    delay( 500 );

    // begin() returns ESP_ERR_INVALID_ARG if the timing can't be produced,
    // for example a time that rounds to zero ticks, or (PARLIO) no clock
    // that meets the tolerance.
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
    static const crgb_t colours[] = { L_RED, L_GREEN, L_BLUE };
    static uint8_t i = 0;

    myLED.fill( colours[ i ], true );
    i = ( i + 1 ) % 3;
    delay( 1000 );
}
