/**
 * LiteLED Debug Test Sketch
 * 
 * Use this file to create test cases for Issue #20 investigation
 * The LiteLED library source is linked from the parent directory,
 * so any changes you make to the library will be compiled directly.
 */

#include <Arduino.h>
#include <LiteLED.h>

// Configure your LED setup here
#define LED_GPIO        8       // GPIO pin connected to LEDs
#define LED_COUNT       1       // Number of LEDs in the strip
#define LED_TYPE        LED_STRIP_WS2812  // LED type

// Create LiteLED instance
LiteLED myLED( LED_TYPE, 0 );   // 0 = RMT channel

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n--- LiteLED Issue #20 Debug ---");
    Serial.println("Starting LED initialization...");
    
    // Initialize the LED strip
    if ( myLED.begin( LED_GPIO, LED_COUNT ) ) {
        Serial.println("LED strip initialized successfully");
    } else {
        Serial.println("ERROR: LED strip initialization failed!");
    }
    
    // Set brightness
    myLED.brightness( 50 );  // 50% brightness
    
    Serial.println("Setup complete!");
}

void loop() {
    // Test pattern - add your debug code here
    Serial.println("Setting RED...");
    myLED.fill( LiteLED::RED );
    myLED.show();
    delay(1000);
    
    Serial.println("Setting GREEN...");
    myLED.fill( LiteLED::GREEN );
    myLED.show();
    delay(1000);
    
    Serial.println("Setting BLUE...");
    myLED.fill( LiteLED::BLUE );
    myLED.show();
    delay(1000);
}
