#include <LiteLED.h>

#define LED_GPIO    14
#define LED_COUNT   60

LiteLED strip(LED_STRIP_WS2812, false);

void setup() {
    strip.begin(LED_GPIO, LED_COUNT);
    strip.brightness(50);
}

void loop() {
    static uint8_t hue = 0;
    
    for (size_t i = 0; i < LED_COUNT; i++) {
        // Create rainbow effect
        uint8_t pixelHue = hue + (i * 255 / LED_COUNT);
        strip.setPixel(i, HSVtoRGB(pixelHue, 255, 255));
    }
    
    strip.show();
    hue += 2;
    delay(20);
}

// Helper function to convert HSV to RGB
crgb_t HSVtoRGB(uint8_t h, uint8_t s, uint8_t v) {
    uint8_t region, remainder, p, q, t;
    uint8_t r, g, b;
    
    if (s == 0) {
        return ((uint32_t)v << 16) | ((uint32_t)v << 8) | v;
    }
    
    region = h / 43;
    remainder = (h - (region * 43)) * 6;
    
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    
    switch (region) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}