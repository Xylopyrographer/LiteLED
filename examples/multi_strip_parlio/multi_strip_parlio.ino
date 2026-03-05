/*
    multi_strip_parlio.ino — LiteLEDpioGroup example

    Drives two independent LED strips simultaneously from a single PARLIO TX
    unit using LiteLEDpioGroup. Every show() encodes both lane pixel buffers
    into one DMA bitstream and transmits them in perfect lock-step from a
    single GDMA transfer.

    Hardware
    --------
    Two WS2812 strips connected to the XIAO ESP32-C6 (or any
    SOC_PARLIO_SUPPORTED device):
        Strip A DIN → GPIO 21
        Strip B DIN → GPIO 19
    Both strips share a common 5V supply and GND.

    The sketch cycles through four animations, alternating colours on the
    two strips to demonstrate that each lane is independently controllable.

    Requires
    --------
    - arduino-esp32 3.0.0 or later (IDF 5.x PARLIO TX API)
    - LiteLED library 3.1.0 or later
    - ESP32 SoC with SOC_PARLIO_SUPPORTED (e.g., ESP32-C6, ESP32-H2)
*/

#include <Arduino.h>
#include <LiteLED.h>

// ---------------------------------------------------------------------------
// Configuration — adjust to your hardware
// ---------------------------------------------------------------------------
#define LED_TYPE            LED_STRIP_WS2812
#define LED_IS_RGBW         false

#define GPIO_STRIP_A        21
#define GPIO_STRIP_B        19

#define NUM_LEDS            64          // LEDs per strip
#define BRIGHTNESS          20          // 0–255

// ---------------------------------------------------------------------------
// LiteLEDpioGroup — one PARLIO TX unit, two independent bit-lane outputs
//
//  Constructor: LiteLEDpioGroup(led_type, length_per_strip, rgbw)
//
//  All strips added to the group must share the same LED type, length, and
//  RGBW setting.
// ---------------------------------------------------------------------------
LiteLEDpioGroup strips( LED_TYPE, NUM_LEDS, LED_IS_RGBW );

// Lane handles are set during setup() after addStrip() calls.
// LiteLEDpioLane exposes the same pixel/brightness API as LiteLED/LiteLEDpio.
LiteLEDpioLane *stripA = nullptr;
LiteLEDpioLane *stripB = nullptr;

// ---------------------------------------------------------------------------
// Colour definitions
// ---------------------------------------------------------------------------
static const crgb_t BLACK  = 0x000000;
static const crgb_t RED    = 0x200000;
static const crgb_t GREEN  = 0x002000;
static const crgb_t BLUE   = 0x000020;
static const crgb_t CYAN   = 0x001818;
static const crgb_t YELLOW = 0x181800;

// ---------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin( 115200 );
    delay( 500 );

    Serial.println( "LiteLEDpioGroup — dual-strip PARLIO example" );

    // Register each strip on a bit-lane before calling begin().
    // addStrip() assigns lanes sequentially: first call → lane 0,
    // second call → lane 1, and so on up to PARLIO_TX_UNIT_MAX_DATA_WIDTH.
    //
    // For explicit lane assignment use the template form:
    //   strips.addStrip<0>( GPIO_STRIP_A );
    //   strips.addStrip<1>( GPIO_STRIP_B );
    stripA = &strips.addStrip( GPIO_STRIP_A );
    stripB = &strips.addStrip( GPIO_STRIP_B );

    // begin() allocates per-lane pixel buffers and the shared DMA bitstream
    // buffer, creates the PARLIO TX unit, and registers both GPIOs with the
    // ESP32 Peripheral Manager.
    //
    // Optional PSRAM flag:  strips.begin( PSRAM_ENABLE );
    //   PSRAM applies to the pixel colour buffers only.
    //   The DMA bitstream buffer is always in internal DMA-capable RAM.
    esp_err_t err = strips.begin();
    if ( err != ESP_OK ) {
        Serial.printf( "FATAL: strips.begin() failed: %s\n", esp_err_to_name( err ) );
        while ( true ) {
            delay( 1000 );
        }
    }

    strips.brightness( BRIGHTNESS );

    // Clear both strips and push the empty frame.
    stripA->clear();
    stripB->clear();
    strips.show();

    Serial.println( "Ready.\n" );
}

// ---------------------------------------------------------------------------
// loop — four animation demos, repeating
// ---------------------------------------------------------------------------
void loop() {

    // -----------------------------------------------------------------------
    // Demo 1: Solid fill — strips show independent colours
    // -----------------------------------------------------------------------
    Serial.println( "Demo 1: independent solid fills" );

    stripA->fill( RED );
    stripB->fill( BLUE );
    strips.show();          // Both strips updated simultaneously
    delay( 1500 );

    stripA->fill( GREEN );
    stripB->fill( YELLOW );
    strips.show();
    delay( 1500 );

    stripA->clear();
    stripB->clear();
    strips.show();
    delay( 300 );

    // -----------------------------------------------------------------------
    // Demo 2: Opposing row sweeps
    //   Strip A — rows 0→7 (top-to-bottom) in red
    //   Strip B — rows 7→0 (bottom-to-top) in blue
    // Assumes 8×8 matrix in row-major order; adjust MATRIX_W for other sizes.
    // -----------------------------------------------------------------------
    Serial.println( "Demo 2: opposing row sweeps" );
    const int MATRIX_W = 8;
    const int MATRIX_H = NUM_LEDS / MATRIX_W;

    for ( int row = 0; row < MATRIX_H; row++ ) {
        stripA->clear();
        stripB->clear();
        for ( int c = 0; c < MATRIX_W; c++ ) {
            stripA->setPixel( row * MATRIX_W + c, RED );
            stripB->setPixel( ( MATRIX_H - 1 - row ) * MATRIX_W + c, BLUE );
        }
        strips.show();
        delay( 100 );
    }
    delay( 600 );

    stripA->clear();
    stripB->clear();
    strips.show();
    delay( 300 );

    // -----------------------------------------------------------------------
    // Demo 3: Synchronised pixel walk — same pixel index, different colours
    // -----------------------------------------------------------------------
    Serial.println( "Demo 3: synchronised pixel walk" );

    for ( size_t px = 0; px < NUM_LEDS; px++ ) {
        stripA->setPixel( px, CYAN );
        stripB->setPixel( px, YELLOW );
        strips.show();
        delay( 40 );
        stripA->setPixel( px, BLACK );
        stripB->setPixel( px, BLACK );
    }
    strips.show();
    delay( 300 );

    // -----------------------------------------------------------------------
    // Demo 4: Brightness pulse — group brightness affects both strips
    // -----------------------------------------------------------------------
    Serial.println( "Demo 4: brightness pulse" );

    stripA->fill( RED );
    stripB->fill( GREEN );
    strips.show();
    delay( 300 );

    for ( int rep = 0; rep < 3; rep++ ) {
        strips.brightness( 0, true );
        delay( 200 );
        strips.brightness( BRIGHTNESS, true );
        delay( 200 );
    }

    stripA->clear();
    stripB->clear();
    strips.show();
    delay( 800 );
}

//  --- EOF --- //
