# LiteLED

## v3.0.0

## What is it?

LiteLED is an arduino-esp32 library for the Espressif ESP32 series of SoC's for controlling WS2812B, SK6812, APA106 and SM16703 intelligent "clockless" RGB colour LED's.

It provides hardware-accelerated LED control with support for driving multiple LED strips, arbitrary colour orders, DMA transfers, interrupt priority, and buffer allocation to either internal RAM or PSRAM.


## Features

**Global brightness**

- The intensity of all LED's in a strip be set at once. This is non-destructive to the colour value in the LED buffer.
- Makes flashing the LED strip very easy.
- Allows for auto dimming applications.


**Works with WiFi**

- When used with a dual core ESP32 SoC, LiteLED is compatible with concurrent use of the WiFi system.


**Multi-Display Support**

- Up to eight LED strips can be driven.

**Configurable Color Order**

- Though the RGB colour order for the supported LED types is supposed to be standard, some are not. The library lets you set any R, G, and B colour order for any LED type to manage those cases. No need to modify colour definitions in the sketch.

**DMA Support**

- On supported ESP32 models, can optionally use RMT DMA for driving the LED strip.

**Interrupt Priority Support**

- The priority of the routines that service the LED strip can be modified for performance tuning.

**PSRAM Support**

- The LED buffer can optionally be placed in PSRAM.

**Robust Error Checking and Fallbacks**

The library is quite fault tolerant, providing:

- Automatic conflict resolution.
- Intelligent interrupt priority management with fallback mechanisms.
- Pre-flight conflict detection to prevent initialization failures.
- Integration with ESP32 Peripheral Manager for safe GPIO usage.
- Resource tracking and monitoring in multi-display applications.

**Thread Safe**

- Though not extensively tested, LiteLED should also be thread-safe.

## Compatibility

### SoC Models

LiteLED has been tested on the following SoC's:

* ESP32
* ESP32-C3
* ESP32-S2
* ESP32-S3

Would appreciate feedback on use of this library on other models.

LiteLED uses the RMT peripheral of the ESP32 to send data to the LED strip and therefore the target device must have an RMT peripheral. As the family of ESP32 models grows, this is not always the case.

Among SoC's that do have an RMT peripheral, the implementation also varies. This may place restrictions on the availability and combinations of configurations available when using the library. Note that these restrictions are fundamental to how the RMT peripheral is implemented in the SoC and are not a restriction of the library. 

### Compatibility with arduino-esp32 Core Versions

There have been a number of "interesting" changes with regard to how the RMT peripheral is supported across the arduino-esp32 core, starting with core version 3.0.0.

Creating a single library that supports core versions from 3.0.0 to 3.0.7 proved to be a bit of a challenge so the decision was made to exclude a limited number of core releases from the 3.0.x stream. The TLDR version of this is LiteLED requires methods that are not available in the excluded versions. Fire up a note in the library GitHub Discussions page if you're curious.

The table below summarizes compatibility with arduino-esp32 core versions from 2.0.0 onward.

| **core version** | **Compatible<br>LiteLED<br>Version** | **Note** |
|:----------------:|:------------------------------------:|:--------:|
|      < 2.0.3     |                 None                 |     1    |
|  2.0.3 to 2.0.17 |                 1.2.1                |          |
|  3.0.0 to 3.0.2  |                 None                 |     1    |
|  3.0.3 to 3.0.7  |                 1.2.1                |     2    |
|  3.1.0 and later |       2.0.0 and greater              |     3    |

**Notes:**

1. Not compatible.
2. Requires the `-DESP32_ARDUINO_NO_RGB_BUILTIN` workaround discussed on the arduino-esp32 GitHub site [here](https://github.com/espressif/arduino-esp32/pull/9941) and as shown in the example [here](https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/RMT/Legacy_RMT_Driver_Compatible).
3. Fully compatible. **But highly recommend using v3+**.

### Breaking Change

As the underlying method of using the RMT peripheral was completely redone starting with arduino-esp32 core version 3.0.0, this has caused a breaking change in the method used to declare the LiteLED object starting at LiteLED version 2.0.0.

## API Document

The API information has been moved to a new *Using LiteLED.md* document.

It can be found in the same place as this README.md file.

---

## Acknowledgement

A good chunk of LiteLED is based on the `led_strip` driver from the Uncle Rus [esp-idf-lib](https://github.com/UncleRus/esp-idf-lib). Full credit and recognition to the team that supplies and supports this incredible resource.

Starting with library version 2, the RMT driver is based on the espressif-idf example found [here](https://github.com/espressif/esp-idf/tree/a6c3a9cb/examples/peripherals/rmt/led_strip_simple_encoder).

And a tip of the hat to those who submitted PR's and suggestions for improvement.


## Support and Contributions

For issues, feature requests, or contributions, please visit the library repository.

---

## Version History

### v3.0.0

- Feat: Add support for RMT DMA on compatible chips.
- Feat: Add support for using PSRAM for the LED buffer.
- Feat: Add ability to set the RMT interrupt priority where supported by the arduino-esp32 core.
- Feat: Add integration with the arduino-esp32 Peripheral Manager.
- Feat: Add multiple initialization options with `begin()` overloads
- Feat: Add enhanced error handling and validation.
- Feat: Add static utility methods for GPIO checking
- Fix. Greatly improved handling of multiple strips.
- Fix. Debug logging levels are a bit less aggressive. Refer to the *Logging* section in the *Using LiteLED.md* document.
- Fix. The usual kind of fixes applied after spending too many late hours working on previous versions.
- Fix: gobs of architectural changes to the files that make up the library.
- Doc: Create separate *Using LiteLED.md* document.
- Doc: Update this document.

### v2.0.2

- Improved performance when concurrently driving multiple strips.
- The `library.properties` file is correctly updated to reflect this version. Missed doing that last time so v2.0.1 was never found by most everyone. My bad.

### v2.0.1

- Bug fix. Driving multiple strips now works as expected.

### v2.0.0

- Significant rewrite bringing compatibility with arduino-esp32 core version 3.1+. Refer to the *Compatibility* section above.
- **There is a breaking change with this release.** Refer to the *Breaking Change* section above.
- Added new `begin()` method to:
    - support RMT DMA access if available in the ESP32 model.
    - set the interrupt priority for the RMT driver.
- Added `fillRandom()` method to fill the strip with random colours.
- Added `setOrder()` method to set a custom LED colour order.
- Added `resetOrder()` method to reset the LED colour order to its default.
- Improved error checking and reporting.

### v1.2.1

- Bug fix: `setPixels()`, with `crgb_t` data, would not flush the buffer to the LED's when the parameter `show` was `true`.
- Bug fix: `setPixels()`, with `rgb_t` data, would always start at `0` regardless of the `start` value provided.

### v1.2.0

- add method to get the brightness value of the strip.

### v1.1.0

- add methods to get the colour of a LED in the strip.

### v1.0.0
- initial release.

---

## License

LiteLED is provided under the terms of the MIT license:

**MIT License**

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

**THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.**

<!-- //  --- EOF --- // -->
