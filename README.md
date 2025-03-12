# LiteLED

## v2.0.2

## What is it?

An arduino-esp32 library for the Espressif ESP32 series of SoC's for controlling WS2812B, SK6812, APA106 and SM16703 intelligent "clockless" RGB colour LED's.

It is a "light weight" library targeted for applications where simple colours or patterns on a LED strip or matrix panel are all that is required, such as driving the one colour RGB LED found on many ESP32 development boards.


## Features

**Global brightness**

- LiteLED lets the intensity of all LED's be set at once. This is non-destructive to the colour value of the LED.
- Makes flashing the LED strip very easy.


**Works with WiFi**

- When used with a dual core ESP32 SoC, LiteLED is compatible with concurrent use of the WiFi system.
- Requires that LiteLED and the WiFi system be run on different cores.


**Multi string capable**

- Multiple LED strings can be concurrently driven.
- Limited by available memory and RMT channels.


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

### Compatibility with arduino-esp32 Core Versions

There have been a number of "interesting" changes with regard to how the RMT peripheral is supported across the arduino-esp32 core starting with core version 3.0.0.

Creating a single library that supports core versions from 3.0.0 to 3.0.7 proved to be a bit of a challenge so the decision was made to exclude a limited number of core releases from the version 3.0.x stream. The TLDR version of this is LiteLED requires methods that are not available in the excluded versions. Fire up a note in the library GitHub Discussions page if you're curious.

The table below summarizes compatibility with arduino-esp32 core versions from 2.0.0 onward.

| **core version** | **Compatible <br>LiteLED<br> Version** | **Note** |
|:----------------:|:------------------------------------:|:--------:|
|      < 2.0.3     |                 None                 |     1    |
|  2.0.3 to 2.0.17 |                 1.2.1                |          |
|  3.0.0 to 3.0.2  |                 None                 |     1    |
|  3.0.3 to 3.0.7  |                 1.2.1                |     2    |
|  3.1.0 and later |                 2.0.0                |     3    |

**Notes:**

1. Not compatible.
2. Requires the `-DESP32_ARDUINO_NO_RGB_BUILTIN` workaround discussed on the arduino-esp32 GitHub site [here](https://github.com/espressif/arduino-esp32/pull/9941) and as shown in the example [here](https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/RMT/Legacy_RMT_Driver_Compatible).
3. Fully compatible.

### Breaking Change

As the underlying method of using the RMT peripheral was completely redone starting with  arduino-esp32 core version 3.0.0, this has caused a breaking change in the method used to declare the LiteLED object starting at LiteLED version 2.0.0.

Refer to *Constructor* under the *Using the Library* section below for details.

### New Features

LiteLED version 2 introduces additional functionality.

- `begin()` adds a new method to support RMT DMA and RMT interrupt priority.
- Methods are provided to set and reset the LED colour order.
- Support for "WS2812" LED's with RGB colour order added.
- A method to fill the strip with random colours had been added.

# Using the Library

## Colour Representation

The intensity and colour of a LED is defined by setting a value for each of its red, blue and green channels. Values range between `0` and `255`, where `0` is off and `255` is full on. By adjusting the values of each channel, different colours and intensities result.

With LiteLED, colours are defined in two ways:

_**As an RGB colour structure**_

In this way colours are defined as a structure of type `rgb_t` where a member of the structure represents the intensity of the red, blue and green channels for a particular LED. Members can be accessed using either `.r, .b, .g` or `.red, .blue, .green` notation.

**Example:**

Define a colour:

`rgb_t myColour = { .r = 47, .g = 26, .b = 167 };`

Set the green channel of a colour variable:

`myColour.green = 76;`


_**As an RGB colour code**_

In this way colours are defined as type `crgb_t` where the colour is represented by a 24-bit value within which eight bits are assigned for the intensity of the red, blue and green channels for a particular LED in the form `0xRRGGBB`.

**Example:**

Define a colour:

`crgb_t myOtherColour = 0xff0000;    // pure red`

`crbg_t yetAnotherColour = 0xafafaf;  // white-ish`


**Notes:**

1. Though not required, hex notation is typically used when defining `crgb_t` colours as it makes the values for each of the channels easier to see.

2. Once defined, a colour cannot be accessed as the other type. For example,

```
crgb_t myOtherColour = 0xff0000;
myOtherColour.blue = 123;         // oops - no can do
```
will produce an error at line 2 as `myOtherColour` is defined as type `crgb_t` and the statement is attempting to change the blue channel using `rgb_t` notation.

See also the *Kibbles and Bits* section below.

### Regarding RGBW Strips

LiteLED can drive RGBW strips like SK6812 RGBW types however there is no direct method for setting the value of the W channel. By default LiteLED will automatically set the value of the W channel based on some behind the scenes magic derived from the R, G, and B values for that LED. Thus by default the R, G, B, and W LED's will illuminate based on the values set. 

This behaviour can be disabled when initializing the strip in the `begin()` method. When disabled, the value of the W channel is set to 0 and the white LED will not illuminate. Given that RGBW strips are available with many choices for the colour temperature of the W LED, give it a shot both ways and pick the one that looks good to you.

LiteLED does not support RGBWW (dual white channel) type strips.

## _

## Constructor

Create a strip object.

**`LiteLED( led_type, rgbw );`**

Where:

**`led_type`**

    One of the five supported LED types. Must be one of:<br><br>
        `LED_STRIP_WS2812`<br>
        `LED_STRIP_WS2812_RGB`<br>
        `LED_STRIP_SK6812`<br>
        `LED_STRIP_APA106`<br>
        `LED_STRIP_SM16703`<br>

**`rgbw`**

    A boolean set to `true` if the `led_type` is of  RGBW type, `false` if not.


**Notes**

1. **BREAKING CHANGE**. With earlier versions of the library, you could optionally specify an RMT channel when creating the strip object. As a result of changes in the underlying code starting with arduino-esp32 core 3.0.0, the RMT channel is now automatically assigned. It is no longer possible to specify the RMT channel. Your code will not compile if that parameter is specified. The code will need to be modified by removing the RMT channel specifier if this option was used.

1. If driving multiple strings be aware that not all ESP32 SoC's have the same number of RMT channels. Confirm with the data sheet for the target device.  
   
1. If driving more than one LED string, create an object for each string. This is subject to the number of available RMT channels for the SoC. Strings can be of different LED types, but you cannot mix types within a string.

2. The standard colour order for WS2812 LED's is GRB. However it's been seen that there are LED's stated as WS2812 but have a colour order of RGB. The `LED_STRIP_WS2812_RGB` type was added as a result. If the colours of your WS2812 strip are odd, try using this type.

**Examples**

    `LiteLED myStrip( LED_STRIP_WS2812, 0 );`

    Creates a LiteLED strip object named `myStrip` made up of WS2812 LED's.

    `LiteLED strip2( LED_STRIP_SK6812, 1 );`

    Creates a LiteLED strip object named `strip2` made up of SK6812 RGBW LED's.


## Methods

#### `begin( data_pin, length, dma_flag, priority, auto_w );`

#### Description:

After calling the constructor, and before using any other LiteLED library methods, the LED object must be initialized by a `begin()` method.

#### Parameters:

**`data_pin`**

The ESP32 GPIO number connected to the `DATA` or `DIN` pin of the LED's.

Type is `uint8_t`.


**`length`**

The number of physical LED's in the strip.

Type is `size_t`.


**`dma_flag`**

An enumerated value to enable use of DMA by the RMT peripheral.

Type is `ll_dma_t` and must one of:<br><br>
    `DMA_ON` - enables RMT DMA<br>
    `DMA_OFF` - disables RMT DMA<br>
    `DMA_DEFAULT` - equivalent to `DMA_OFF`<br>

     **Note:** Not all ESP32 models support RMT DMA. If this is set to `DMA_ON` on unsupported ESP32 models the library will set this to `DMA_OFF` and throw a warning message during compilation.


**`priority`**

An enumerated value that selects the interrupt priority of the RMT driver.

Type is `ll_priority_t ` and must one of:<br><br>
    `PRIORITY_DEFAULT` - the RMT driver sets the priority<br>
    `PRIORITY_HIGH`<br>
    `PRIORITY_MED`<br>
    `PRIORITY_LOW`<br>

     **Note:** Recommendation is to use `PRIORITY_DEFAULT`, however there may be certain applications where increasing the priority improves performance.


**`auto_w`**

An optional boolean parameter that when set to `false` will disable the automatic setting of the W channel for RGBW strips.

Default if omitted is `true`.

See also *Regarding RGBW Strips* under the *Colour Representation* section above.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.
    
#### Note:
Memory is required for each string buffer. It is recommended to check the return code to ensure the string buffers have been allocated.


## _

#### `begin( data_pin, length, auto_w )`

#### Description:

Simplified version of the `begin()` method.

Parameters are as per the full `begin()` method form described above.

With this method, **`dma_flag`** is set to `DMA_DEFAULT` and **`priority`** is set to `PRIORITY_DEFAULT`.


## _

#### `show()`

#### Description:

Send the LED buffer data to the strip.

LiteLED maintains a buffer in memory that holds the colour data for each of the LED's in the strip.

This data does not affect the colour of the LED's until a `show()` or a method that calls `show()` is used.

#### Parameters:

None

#### Returns:

`esp_err_t` code `ESP_OK` if successful.

#### Note:

`show()` and other methods that call `show()` always send the entire buffer to the strip. Thus, if you're making multiple changes to the LED colours, make all changes before using `show()` as that will execute much faster and without flickering or other artefacts.

## _

#### `setPixel( num, colour, show )`

#### Description:

Set the colour of a single LED in the strip.

#### Parameters:

**`num`**

The number of the LED in the strip to set the colour.

Type is `size_t`.

The first LED in the strip is `0`.

**`color`**

The colour to set the LED to.

Type is either `rgb_t` or `crgb_t`.

**`show`**

An optional parameter of type `bool` that if set `true` will send the LED buffer data to the strip after the colour of the LED is set. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


## _
#### `setPixels( start, length, *data, show )`


#### Description:

Set colours of multiple consecutive LED's in the strip.

`setPixels` reads LED colour data from a user-specified buffer in memory and writes that data to the internal LED strip buffer starting at position `start` for `len` number of LED's.

#### Parameters:

**`start`**

The starting position of the LED in the strip where the colour data is to be set.

Type is `size_t`.

The first LED in the strip is `0`.

**`length`**

The number of LED's to set the colours, inclusive of the `start` LED.

Type is `size_t`.

**`data`**

Pointer to the buffer containing the colour data.

The buffer must be large enough to contain `length` elements of either type `rgb_t` or `crgb_t`.

See also the *rgb_t structure definition* and *crgb_t definition* blurbs under the  *Kibbles and Bits* section below.

**`show`**

An optional parameter of type `bool` that if set `true` will send the LED buffer data to the strip after the colours of the LED's are set. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


## _
#### `fill( color, show )`

#### Description:

Set all LEDs to a single colour.

#### Parameters:

**`color`**

The colour to set the LED's to.

Type is either `rgb_t` or `crgb_t`.

**`show`**

An optional parameter of type `bool` that if set `true` will send the LED buffer data to the strip after the colour of the LED's is set. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


## _
#### `clear( show )`

#### Description:

Set all LED's to colour black.

#### Parameters:

**`show`**

An optional parameter of type `bool` that if set `true` will send the LED buffer data to the strip. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


## _
#### `brightness( bright, show )`

#### Description:

Set the intensity of all LED's in the strip.

Brightness is a global parameter for the entire strip. It does not change the colour value of any LED in the strip buffer.

It is not required to be set as LiteLED defaults the brightness to 255 (full on) when initialized using the `begin` method.

#### Parameters:

**`bright`**

Value to set the strip brightness to.
    
Type is `uint8_t`.
    
Range is `0` to `255`.

**`show`**

An optional parameter of type `bool` that if set `true` will send the strip buffer to the strip after setting the brightness. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.

#### Note:

A change in brightness does not take effect until after a `show()` or another method that calls `show()` is used.

## _
#### `getBrightness( )`

#### Description:

Get the intensity of the strip.

#### Parameters:

None.

#### Returns:

The brightness value of the strip.

Type is `uint8_t`.

#### Note:

The method returns the actual operating intensity value of the strip. That is, it returns the brightness value used the last time a `show()` or method that calls `show()` was called. 

If `getBrightness()` is used after using `brightness()` but before a `show()` or method that calls `show()` was used, the return value will be what was set with the `brightness()` method before the last call to `show()`.

See also the **Note:** under `brightness()`.

If an error occurs, a message is sent to the serial port via the esp32 `log_e` facility and the method returns `0`.


## _
#### `getPixel( num )`

#### Description:

Get the colour of a LED in the strip in `rgb_t` format.

#### Parameters:

**`num`**

The number of the LED in the strip to get the colour of.

The first LED in the strip is `0`.

Type is `size_t`.

#### Returns:

The colour of the LED at position `num` in the strip.

Type is `rgb_t`.

If the strip is of RGBW type, the method returns only the R, G, and B values. The W value is not available.

#### Notes:

1. The method returns the colour value for LED `num` from its internal buffer. If the buffer has not yet been sent to the strip using `show()` or a method that calls `show()`, the colour of the LED may not match the returned value.

1. If an error occurs, a message is sent to the serial port via the esp32 `log_e` facility and the method returns R, G, and B values set to `0`.

## _


#### `getPixelC( num )`

#### Description:

Get the colour of a LED in the strip in `crgb_t` format.

#### Parameters:

**`num`**

The number of the LED in the strip to get the colour of.

The first LED in the strip is `0`.

Type is `size_t`.

#### Returns:

The colour of the LED at position `num` in the strip.

Type is `crgb_t`.

If the strip is of RGBW type, the method returns only the R, G, and B values. The W value is not available.

#### Notes:

1. The method returns the colour value for LED `num` from its internal buffer. If the buffer has not yet been sent to the strip using `show()` or a method that calls `show()`, the colour of the LED may not match the returned value.

1. If an error occurs, a message is sent to the serial port via the esp32 `log_e` facility and the method returns `0x000000`.

## _

#### `fillRandom( show )`

#### Description:

Fill the strip with random colours.

#### Parameters:

**`show`**

An optional parameter of type `bool` that if set `true` will send the LED buffer data to the strip. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


#### Notes:

1. Each colour channel of the LED is set independently to a random value between 5 and 255. Thus when using this method, the strip can be quite bright. The `brightness()` method can be used beforehand to lower the strip intensity.


## _

#### `setOrder( color_order )`

#### Description:

Set the colour order of the LED's in the strip.

#### Parameters:

**`color_order`**

An enumerated value that sets the colour order of the LED's in the strip.

Type is `ll_order_t` and must be one of:<br><br>
    `ORDER_RGB`<br>
    `ORDER_RBG`<br>
    `ORDER_GRB`<br>
    `ORDER_GBR`<br>
    `ORDER_BRG`<br>
    `ORDER_BGR`<br>

#### Returns:

`esp_err_t` code `ESP_OK` if successful.

#### Notes:

1. This method overrides the order set by the LED strip type. As all LED strip types have a defined colour order it is not required to call this method. It was added to address LED's with non-standard colour orders.
2. Can be called any time after declaring the LiteLED object and takes effect after a call to `show()` or any call that invokes `show()` and remains in effect until another call to `setOrder()` or `resetOrder()` (see below) is made.
3. FWIW, originally intended to be a "one and done" call, multiple calls can be made for creative effect.


## _

#### `resetOrder()`

#### Description:

Restores the colour order of the LED's in the strip as defined by the LED strip type.

#### Parameters:

none

#### Returns:

`esp_err_t` code `ESP_OK` if successful.

#### Notes:

1. Can be called any time after declaring the LiteLED object and takes effect after a call to `show()` or any call that invokes `show()` and remains in effect until another call to `setOrder()` or `resetOrder()` is made.


## _


## Kibbles and Bits

### `rgb_t` structure definition

LiteLED stores all colour data internally as an `rgb_t` structure. The definition of that structure is:

```
typedef struct {
    union {
        uint8_t r;
        uint8_t red;
    };
    union {
        uint8_t g;
        uint8_t green;
    };
    union {
        uint8_t b;
        uint8_t blue;
    };
} rgb_t;

```

Thus, if looking use the `setPixels()` method to copy a user-specified buffer into a strip, if the data is in `rgb_t` format, the buffer size must be, in `uint8_t` terms, at least `3 * length` where `length` is the number of LED's in the strip you are changing with the `setPixels` method.

For RGBW strips, the W channel value is set internally by the library so no extra room in the buffer is required. See *Regarding RGBW Strips* under the *Colour Representation* section above.

### `crgb_t` definition

`crgb_t` data is defined as:

```typedef uint32_t crgb_t;```

Thus, if looking use the `setPixels()` method to copy a user-specified buffer into a strip, if the data is in `cgb_t` format, the buffer size must be, in `uint8_t` terms, at least `4 * length` where `length` is the number of LED's in the strip you are changing with the `setPixels()` method.

For RGBW strips, the W channel value is set internally by the library so no extra room in the buffer is required. See *Regarding RGBW Strips* under the *Colour Representation* section above.


### Return Status Codes

The LiteLED methods that write to the string buffer or set/reset the colour order (everything else except the constructor and the `get...` methods) return a status code of `esp_err_t ` type on completion. Checking this code and taking action is optional and is an exercise left to the developer.

If things go as normal, the return code is `ESP_OK` which is of type `int` with a value of `0`. So a quick check would be, if the return code is anything other than `0`, something went amok.

Full description of these codes can be found on the Espressif ESP-IDF site [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/error-codes.html?highlight=error%20handling).

If you're really interested in diving deeper, head over to the Espressif ESP-IDF Error Handling docs [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/error-handling.html?highlight=error%20handling).

To assist in debugging, a number of error messages are sent to the serial port via the esp32 `log_e` facility. This is enabled in the Arduino IDE by selecting *Error* in the *Core Debug Level* from the *Tools* menu. If using PlatformIO or pioarduino, add `-DCORE_DEBUG_LEVEL=1` to `build_flags` section the `platform.ini` file.

### Acknowledgement

A good chunk of LiteLED is based on the `led_strip` driver from the Uncle Rus [esp-idf-lib](https://github.com/UncleRus/esp-idf-lib). Full credit and recognition to the team that supplies and supports this incredible resource.

Starting with library version 2, the RMT driver is based on the espressif-idf example found [here](https://github.com/espressif/esp-idf/tree/a6c3a9cb/examples/peripherals/rmt/led_strip_simple_encoder).

## _

## Revision History


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

- Bug fix: `setPixels()`, with `crgb_t` data, would not flush the buffer to the LED's when the parameter `show` was `true`.<br>
- Bug fix: `setPixels()`, with `rgb_t` data, would always start at `0` regardless of the `start` value provided.

### v1.2.0

- add method to get the brightness value of the strip.

### v1.1.0

- add methods to get the colour of a LED in the strip.

### v1.0.0
- initial release.

## -

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
