# LiteLED

## What is it?

An arduino-esp32 library for controlling WS2812B, SK6812, APA106 and SM16703 RGB colour LED's with the ESP32 series of SoC's.

It is a "light weight" library targeted for applications where simple colours or patterns on a LED strip or matrix panel are all that is required, such as driving the one colour LED found on many ESP32 development boards.


## Features

**•  Global brightness**<br>
LiteLED lets the intensity of all LED's be set at once. This is non-destructive to the colour value of the LED.

Makes flashing the LED's very easy.

**•  Works with WiFi**<br>
When used with a dual core ESP32 SoC, LiteLED is compatible with concurrent use of the WiFi system.

Requires that LiteLED and the WiFi system be run on different cores.

**• Thread Safe**<br>
Though not extensively tested, LiteLED should also be thread-safe.


## Compatibility

LiteLED has been tested on the following SoC's:

* ESP32
* ESP32-C3
* ESP32-S2
* ESP32-S3

LiteLED requires at minimum arduino-esp32 core version 2.0.3. It will probably break under the pending arduino-esp32 core version 3.

LiteLED uses the RMT peripheral of the ESP32 to send data to the LED strip. The RMT channel number is selectable.


##Colour Representation

The intensity and colour of an LED is defined by setting a value for each of its red, blue and green channels. Values range between `0` and `255`, where `0` is off and `255` is full on. By adjusting the values of each channel, different colours and intensities result.

With LiteLED, colours are defined in two ways:

**As an RGB colour structure**

In this way colours are defined as a structure of type `rgb_t` where a member of the structure represents the intensity of the red, blue and green channels for a particular LED. Members can be accessed using either `.r, .b, .g` or `.red, .blue, .green` notation.

**Example:**

Define a colour:

```rgb_t myColour = { .r = 47, .g = 26, .b = 167 };```

Set the green channel of a colour variable:

```myColour.green = 76;```


**As an RGB colour code**

In this way colours are defined as type `crgb_t` where the colour is represented by a 24-bit value within which eight bits are assigned for the intensity of the red, blue and green channels for a particular LED in the form `0xRRGGBB`.

**Example:**

Define a colour:

```crgb_t myOtherColour = 0xff0000;     // pure red```

```crbg_t yetAnotherColour = 0xafafaf;  // white-ish```


**Notes:**

1. Though not required, hex notation is typically used when defining `crgb_t` colours as it makes the values for each of the channels easier to see.

2. Once defined, a colour cannot be accessed as the other type. For example, 

```
crgb_t myOtherColour = 0xff0000;
myotherColour.blue = 123;    // oops - no can do
```
will produce an error at line 2 as `myOtherColour` is defined as type `crgb_t` and the statement is attempting to change the blue channel using `rgb_t` notation.

See also the *Kibbles and Bits* section below.

### Regarding RGBW Strips

LiteLED can drive RGBW strips like SK6812 RGBW types however there is no direct method for setting the value of the W channel. By default LiteLED will automatically set the value of the W channel based on some behind the scenes magic derived from the R, G and B values for that LED.

This behaviour can be disabled when initializing the strip in the `begin()` method. When disabled, the value of the W channel is set to 0 and the white LED will not illuminate. Given that RGBW strips are available with many choices for the colour temperature of the W LED, give it a shot both ways and pick the one that looks good to you.

LiteLED does not support RGBWW type strips.


## Constructor

Create a strip object.

**`LiteLED( led_type, rgbw, channel );`**

Where:

**`led_type`**

    One of the four possible LED types supported. Must be one of: 

        `LED_STRIP_WS2812`

        `LED_STRIP_SK6812`

        `LED_STRIP_APA106`

        `LED_STRIP_SM16703` 

**`rgbw`**

    A boolean set to `true` if the `led_type` is of  RGBW type, `false` if not.

**`channel`**

    Optional parameter used to specify which RMT channel number LiteLED should use. Default is `RMT_CHANNEL_0`.

    If specified, must be one of:    

        `RMT_CHANNEL_0`

        `RMT_CHANNEL_1`

        `RMT_CHANNEL_2`

        `RMT_CHANNEL_3`

        `RMT_CHANNEL_4`

        `RMT_CHANNEL_5`

        `RMT_CHANNEL_6`

        `RMT_CHANNEL_7`

    **Note:** Not all SoC's have the same number of RMT channels available. Confirm with the data sheet for the target SoC.

**Examples:**

    `LiteLED myStrip( LED_STRIP_WS2812, 0 );`

    Creates a LiteLED strip object named `myStrip` made up of WS2812 LED's of type RGB using the default RMT channel.


    `LiteLED strip2( LED_STRIP_SK6812, 1, RMT_CHANNEL_2 );`

    Creates a LiteLED strip object named `strip2` made up of SK6812 RGBW LED's using RMT channel 2.


## LiteLED Library Methods

####`begin( data_pin, length, auto_w )`

#### Description:

After calling the constructor, and before using any other LiteLED library methods, the LED object must be initialized by calling this function.

#### Parameters:

**`data_pin`**

The GPIO pin number of the SoC connected to the `DATA` or `DIN` pin of the LED's. Type is `uint8_t`.

**`length`**

The number of LED's in the strip. Type is `size_t`.

**`auto_w`**

An optional boolean parameter that when set to `false` will disable the automatic setting of the W channel for RGBW strips. Default if omitted is `true`.

See also *Regarding RGBW Strips* under the *Colour Representation* section above.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


## 
#### `show()`

#### Description:

Send the LED colour buffer data to the strip.

LiteLED maintains a buffer in memory that holds the colour data for each of the LED's in the strip.

This data does not affect the colour of the LED's until a `show()` method is called. `show()` is the method that writes the data in its buffer to the LED strip.

#### Parameters:

None

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


## 
####`setPixel( num, color, show )`

#### Description:

Set the colour of a single LED in the strip.

#### Parameters:

**`num`**

The number of the LED in the strip to set the colour. Type is `size_t`.

The first LED in the strip is `0`.

**`color`**

The colour to set the LED to.

Type is either `rgb_t` or `crgb_t`.

**`show`**

An optional parameter of type `bool` that if set `true` will send the LED buffer data to the strip after the colours of the LED's are set. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


## 
#### `setPixels( start, length, data, show )`


#### Description:

Set colours of a multiple consecutive LED's in the strip.

`setPixels` reads LED colour data from a user-specified buffer in memory and writes that data to the internal LED strip buffer starting at position `start` for `len` number of LED's.

#### Parameters:

**`start`**

The starting position in the LED in the strip where the colour data is to be set.

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


## 
#### `fill( color, show )`

#### Description:

Set all LEDs to the one colour.

#### Parameters:

**`color`**

The colour to set the LED's to.

Type is either `rgb_t` or `crgb_t`.

**`show`**

An optional parameter of type `bool` that if set `true` will send the LED buffer data to the strip after the colours of the LED's are set. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


## 
#### `clear( show )`

#### Description:

Set all LED's to colour black.

#### Parameters:

**`show`**

An optional parameter of type `bool` that if set `true` will send the LED buffer data to the strip after the colours of the LED's are set. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.


## 
#### `brightness( bright, show )`

#### Description:

Set the intensity of all LED's in the strip.

The brightness is a global parameter for the entire strip. It does not change the colour value of any LED in the strip buffer.

It is not required to be set as LiteLED defaults the brightness to 255 when initialized using the `begin` method. 

#### Parameters:

**`bright`**

Value to set the strip brightness to.

Type is `uint8_t`.

Range is `0` to `255`.

**`show`**

An optional parameter of type `bool` that if set `true` will send the LED buffer data to the strip after the colours of the LED's are set. Default if omitted is `false`.

#### Returns:

`esp_err_t` code `ESP_OK` if successful.



## Kibbles and Bits

### Acknowledgement

LiteLED is based on the `led_strip` driver from the esp-idf-lib. The repository can be found on GitHub at:
[https://github.com/UncleRus/esp-idf-lib](https://github.com/UncleRus/esp-idf-lib)

Full credit and recognition to Uncle Rus and the team that supplies and supports this incredible resource.

### rgb_t structure definition

LiteLED stores all colour data internally as `rgb_t` data. The definition of that structure is:

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

### crgb_t definition

`crgb_t` data is defined as:

```
typedef uint32_t crgb_t;
```
Thus, if looking use the `setPixels()` method to copy a user-specified buffer into a strip, if the data is in `cgb_t` format, the buffer size must be, in `uint8_t` terms, at least `4 * length` where `length` is the number of LED's in the strip you are changing with the `setPixels` method.

For RGBW strips, the W channel value is set internally by the library so no extra room in the buffer is required. See *Regarding RGBW Strips* under the *Colour Representation* section above. 


### Return Status Codes

The LiteLED methods (excepting the constructor) return a status code of `esp_err_t ` type on completion. Checking this code and taking action is optional and is an exercise left to the developer.

If things go OK, the return code is `ESP_OK` which is of type `int` with a value of `0`. So a quick check would be, if the return code anything other than `0`, something went amok.

Full description of these codes can be found on the Espressif ESP-IDF site [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/error-codes.html?highlight=error%20handling).

If you're really interested in diving deeper, head over to the Espressif ESP-IDF Error Handling docs [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/error-handling.html?highlight=error%20handling).


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
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.**
