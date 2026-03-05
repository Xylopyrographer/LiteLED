# Using LiteLED

## Table of Contents

- [Quick Start](#quick-start)
    * [Basic Example](#basic-example)
- [Colour Representation](#colour-representation)
    * [Regarding RGBW Strips](#regarding-rgbw-strips)
- [Classes and Types](#classes-and-types)
    * [LiteLED Class](#liteled-class)
    * [LiteLEDpio Class](#liteledpio-class)
        + [Constructor](#constructor)
        + [Destructor](#destructor)
        + [Initialization](#initialization)
            - [`begin(data_pin, length)`](#begindata_pin-length)
            - [`begin(data_pin, length, psram_flag)`](#begindata_pin-length-psram_flag)
            - [DMA flag and interrupt priority — not applicable](#dma-flag-and-interrupt-priority-not-applicable)
        + [RGBW Support](#rgbw-support)
        + [Driving Multiple Strips in Parallel](#driving-multiple-strips-in-parallel)
        + [Class Interface](#class-interface)

- [LiteLEDpioGroup Class](#liteledpiogroup-class)
    + [Constructor](#constructor-1)
    + [Destructor](#destructor-1)
    + [Strip Registration — `addStrip()`](#strip-registration-addstrip)
        - [Sequential lane assignment](#sequential-lane-assignment)
        - [Explicit lane assignment](#explicit-lane-assignment)
    + [`begin()`](#begin)
    + [`show()`](#show)
    + [`brightness()` / `getBrightness()`](#brightness-getbrightness)
    + [`operator`](#operatorlane)
    + [Class Interface](#class-interface-1)
    + [Quick Example](#quick-example)
- [LiteLEDpioLane Class](#liteledpiolane-class)
        + [`isValid()`](#isvalid)
        + [Class Interface](#class-interface-2)
- [Enumerations](#enumerations)
    * [`led_strip_type_t`](#led_strip_type_t)
    * [`color_order_t`](#color_order_t)
    * [`ll_dma_t`](#ll_dma_t)
    * [`ll_priority_t`](#ll_priority_t)
    * [`ll_psram_t`](#ll_psram_t)
- [Structures](#structures)
    * [`rgb_t`](#rgb_t)
    * [`crgb_t`](#crgb_t)
- [Library API](#library-api)
    * [Constructor](#constructor-2)
        + [`LiteLED()`](#liteled)
    * [Destructor](#destructor-2)
        + [`~LiteLED()`](#liteled-1)
    * [Initialization Methods](#initialization-methods)
        + [`begin()` - Basic](#begin-basic)
        + [`begin()` - PSRAM](#begin-psram)
        + [`begin()` - Full Configuration](#begin-full-configuration)
    * [Display Control Methods](#display-control-methods)
        + [`show()`](#show-1)
        + [`clear()`](#clear)
        + [`brightness()`](#brightness)
        + [`getBrightness()`](#getbrightness)
    * [Pixel Manipulation Methods](#pixel-manipulation-methods)
        + [`setPixel()` - `rgb_t`](#setpixel-rgb_t)
        + [`setPixel()` - `crgb_t`](#setpixel-crgb_t)
        + [`setPixels()` - `rgb_t` Array](#setpixels-rgb_t-array)
        + [`setPixels()` - `crgb_t` Array](#setpixels-crgb_t-array)
        + [`fill()` - `rgb_t`](#fill-rgb_t)
        + [`fill()` - `crgb_t`](#fill-crgb_t)
        + [`fillRandom()`](#fillrandom)
    * [Pixel Reading Methods](#pixel-reading-methods)
        + [`getPixel()`](#getpixel)
        + [`getPixelC()`](#getpixelc)
    * [Colour Order Methods](#colour-order-methods)
        + [`setOrder()`](#setorder)
        + [`resetOrder()`](#resetorder)
    * [Instance Management Methods](#instance-management-methods)
        + [`isValid()`](#isvalid-1)
        + [`getGpioPin()`](#getgpiopin)
        + [`isGpioAvailable()` - Static](#isgpioavailable-static)
        + [`getActiveInstanceCount()` - Static](#getactiveinstancecount-static)
- [Advanced Features](#advanced-features)
    * [Multi-Display Support](#multi-display-support)
        + [Simple Multi-Display Setup](#simple-multi-display-setup)
        + [Advanced Multi-Display Configuration](#advanced-multi-display-configuration)
        + [Automatic Priority Management](#automatic-priority-management)
            - [Priority Levels](#priority-levels)
        + [GPIO Management](#gpio-management)
        + [Best Practices for Multi-Display](#best-practices-for-multi-display)
    * [PSRAM for Large Arrays](#psram-for-large-arrays)
        + [Instance Validation](#instance-validation)
- [Utilities](#utilities)
    * [LiteLED_Utils](#liteled_utils)
        + [`isDmaSupported()`](#isdmasupported)
        + [`isPrioritySupported()`](#isprioritysupported)
        + [Complete Example](#complete-example)
        + [Integration with Existing Code](#integration-with-existing-code)
        + [Related Configuration](#related-configuration)
            - [`DMA_DEFAULT` Behaviour](#dma_default-behaviour)
    * [Colour Utility Functions](#colour-utility-functions)
        + [`rgb_from_code()`](#rgb_from_code)
        + [`rgb_from_values()`](#rgb_from_values)
        + [`rgb_to_code()`](#rgb_to_code)
        + [`rgb_is_zero()`](#rgb_is_zero)
        + [`rgb_luma()`](#rgb_luma)
        + [`scale8()` and `scale8_video()`](#scale8-and-scale8_video)
- [Performance Considerations](#performance-considerations)
    * [Transmission Time](#transmission-time)
    * [Memory Usage](#memory-usage)
        + [LiteLED (RMT) Memory](#liteled-rmt-memory)
        + [LiteLEDpio (PARLIO) Memory](#liteledpio-parlio-memory)
        + [LiteLEDpioGroup Memory](#liteledpiogroup-memory)
        + [Total Internal RAM Footprint](#total-internal-ram-footprint)
- [Optimization Tips](#optimization-tips)
- [Troubleshooting](#troubleshooting)
    * [No LEDs Light Up](#no-leds-light-up)
    * [Wrong Colours](#wrong-colours)
    * [Flickering or Glitches](#flickering-or-glitches)
    * [GPIO Already in Use Error](#gpio-already-in-use-error)
    * [Memory Allocation Failed](#memory-allocation-failed)
    * [Compile-Time Assertions](#compile-time-assertions)
    * [Compile-Time Warnings](#compile-time-warnings)
    * [Error Handling](#error-handling)
        + [Return Status Codes](#return-status-codes)
        + [Error Checking Pattern](#error-checking-pattern)
        + [Common Error Scenarios](#common-error-scenarios)
            - [GPIO Already in Use](#gpio-already-in-use)
            - [Out of Bounds LED Index](#out-of-bounds-led-index)
            - [Memory Allocation Failure](#memory-allocation-failure)
    * [Logging](#logging)
- [Usage Examples](#usage-examples)
    * [Basic Example - Solid Colour](#basic-example-solid-colour)
    * [Brightness Ramping](#brightness-ramping)
    * [Rainbow Pattern](#rainbow-pattern)
    * [Multiple Strips](#multiple-strips)
    * [RGBW Strip with Auto White](#rgbw-strip-with-auto-white)
    * [Large Array with PSRAM](#large-array-with-psram)
    * [High-Performance with DMA](#high-performance-with-dma)
- [Version History](#version-history)

<!-- TOC end -->

---

<a name="quick-start"></a>
# Quick Start

<a name="basic-example"></a>
## Basic Example

**Initialization Sequence**

The correct calling sequence for initialization is:

1. **Create LiteLED object** (constructor)
2. **Call a begin() method** to initialize hardware
3. **Optionally set brightness**
4. **Set pixel colours**
5. **Call show()** to update the LED strip

```cpp
#include <LiteLED.h>

// Define LED strip parameters
#define LED_TYPE    LED_STRIP_WS2812
#define LED_GPIO    14
#define LED_COUNT   30
#define LED_IS_RGBW 0

// Create LiteLED object
LiteLED strip(LED_TYPE, LED_IS_RGBW);

void setup() {
    // Initialize the strip
    strip.begin(LED_GPIO, LED_COUNT);
    
    // Set brightness (0-255)
    strip.brightness(50);
    
    // Set all LEDs to red and show
    strip.fill(rgb_from_code(0xFF0000), true);
}

void loop() {
    // Your animation code here
}
```

---

<a name="colour-representation"></a>
# Colour Representation

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

```c++
crgb_t myOtherColour = 0xff0000;
myOtherColour.blue = 123;         // oops - no can do
```
will produce an error at line 2 as `myOtherColour` is defined as type `crgb_t` and the statement is attempting to change the blue channel using `rgb_t` notation.

See also the *Kibbles and Bits* section below.

<a name="regarding-rgbw-strips"></a>
## Regarding RGBW Strips

LiteLED can drive RGBW strips like SK6812 RGBW types however there is no direct method for setting the value of the W channel. By default LiteLED will automatically set the value of the W channel based on some behind the scenes magic derived from the R, G, and B values for that LED. Thus by default the R, G, B, and W LED's will illuminate based on the values set. 

This behaviour can be disabled when initializing the strip in the `begin()` method. When disabled, the value of the W channel is set to 0 and the white LED will not illuminate. Given that RGBW strips are available with many choices for the colour temperature of the W LED, give it a shot both ways and pick the one that looks good to you.

LiteLED does not support RGBWW (dual white channel) type strips.

---

<a name="classes-and-types"></a>
# Classes and Types

<a name="liteled-class"></a>
## LiteLED Class

The main class for controlling LED strips.

```cpp
class LiteLED {
public:
    // Constructor
    LiteLED(led_strip_type_t led_type, bool rgbw);
    
    // Destructor
    ~LiteLED();
    
    // Initialization methods
    esp_err_t begin(uint8_t data_pin, size_t length, bool auto_w = true);
    esp_err_t begin(uint8_t data_pin, size_t length, ll_psram_t psram_flag, bool auto_w = true);
    esp_err_t begin(uint8_t data_pin, size_t length, ll_dma_t dma_flag, ll_priority_t priority, ll_psram_t psram_flag, bool auto_w = true);
    
    // Display control
    esp_err_t show();
    esp_err_t clear(bool show = false);
    esp_err_t brightness(uint8_t bright, bool show = false);
    uint8_t getBrightness();
    
    // Pixel manipulation
    esp_err_t setPixel(size_t num, rgb_t color, bool show = false);
    esp_err_t setPixel(size_t num, crgb_t color, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, rgb_t *data, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, crgb_t *data, bool show = false);
    esp_err_t fill(rgb_t color, bool show = false);
    esp_err_t fill(crgb_t color, bool show = false);
    esp_err_t fillRandom(bool show = false);
    
    // Pixel reading
    rgb_t getPixel(size_t num);
    crgb_t getPixelC(size_t num);
    
    // Color order control
    esp_err_t setOrder(color_order_t led_order = ORDER_GRB);
    esp_err_t resetOrder();
    
    // Instance management
    bool isValid() const;
    int getGpioPin() const;
    static bool isGpioAvailable(uint8_t gpio_pin);
    static uint8_t getActiveInstanceCount();
};
```

If you're curious about how the library is structured, take a look at the `LiteLED Architecture.md` file in the `docs` folder of the [library repository](https://github.com/Xylopyrographer/LiteLED).

---

<a name="liteledpio-class"></a>
## LiteLEDpio Class

API-compatible alternative to `LiteLED` that drives LED strips via the **PARLIO** (Parallel IO) peripheral instead of RMT.

**Availability:** Only compiled and available on SoCs that define `SOC_PARLIO_SUPPORTED` (e.g., ESP32-C6, ESP32-H2). The class declaration is wrapped in `#if SOC_PARLIO_SUPPORTED`.

**Hardware limits on ESP32-C6:** Only one `LiteLEDpio` instance can be active at a time (`SOC_PARLIO_TX_UNITS_PER_GROUP = 1`). Combine with `LiteLED` (up to 2 RMT TX channels on C6) for multi-strip applications.

<a name="constructor"></a>
### Constructor

```cpp
LiteLEDpio(led_strip_type_t led_type, bool rgbw);
```

Sets the strip type and colour format. No hardware is allocated until `begin()` is called.

| Parameter | Type | Description |
|-----------|------|-------------|
| `led_type` | `led_strip_type_t` | LED strip protocol (e.g., `LED_STRIP_WS2812`) |
| `rgbw` | `bool` | `true` for RGBW strips (e.g., SK6812 RGBW); `false` for RGB strips |

<a name="destructor"></a>
### Destructor

```cpp
~LiteLEDpio();
```

Waits for any in-progress GDMA transfer to complete, disables and deletes the PARLIO TX unit, and frees both the pixel colour buffer and the DMA bitstream buffer. Safe to call even if `begin()` was never called or failed.

<a name="initialization"></a>
### Initialization

<a name="begindata_pin-length"></a>
#### `begin(data_pin, length)`

```cpp
esp_err_t begin(uint8_t data_pin, size_t length, bool auto_w = true);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `data_pin` | `uint8_t` | — | GPIO pin connected to the strip DIN |
| `length` | `size_t` | — | Number of LEDs in the strip |
| `auto_w` | `bool` | `true` | RGBW strips only: `true` derives the W channel automatically from R/G/B values; `false` leaves W at 0 |

Allocates the pixel colour buffer in internal RAM, allocates the DMA bitstream buffer in internal DMA-capable RAM, creates and enables the PARLIO TX unit. Returns `ESP_OK` on success, or an `esp_err_t` error code if the GPIO is already in use or allocation fails.

<a name="begindata_pin-length-psram_flag"></a>
#### `begin(data_pin, length, psram_flag)`

```cpp
esp_err_t begin(uint8_t data_pin, size_t length, ll_psram_t psram_flag, bool auto_w = true);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `data_pin` | `uint8_t` | — | GPIO pin connected to the strip DIN |
| `length` | `size_t` | — | Number of LEDs in the strip |
| `psram_flag` | `ll_psram_t` | — | PSRAM preference for the pixel colour buffer (`PSRAM_ENABLE`, `PSRAM_DISABLE`, or `PSRAM_AUTO`) |
| `auto_w` | `bool` | `true` | RGBW strips only: see above |

**PSRAM and DMA buffer allocation**

The `psram_flag` controls allocation of the **pixel colour buffer** only. For large LED arrays (hundreds to thousands of LEDs), placing the colour buffer in PSRAM avoids consuming significant internal SRAM.

Regardless of `psram_flag`, the **DMA bitstream buffer** is always allocated from internal DMA-capable RAM (`MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL`). GDMA on C6/H2 cannot read PSRAM. The DMA buffer is approximately 87 bytes per LED (24 DMA bytes × 3 colour channels, plus reset padding); for 64 LEDs this is ~5.6 KB, for 144 LEDs ~12.4 KB.

<a name="dma-flag-and-interrupt-priority-not-applicable"></a>
#### DMA flag and interrupt priority — not applicable

`LiteLEDpio` exposes **no** `dma_flag` or `priority` parameters. The PARLIO peripheral transfers data exclusively via GDMA — DMA is always on and cannot be disabled. Because no CPU ISR is involved in the transmission, interrupt priority configuration has no meaning for `LiteLEDpio`.

The three-parameter overload `begin(pin, len, dma_flag, priority, psram_flag)` available on `LiteLED` (RMT) does not exist on `LiteLEDpio`. Attempting to pass `DMA_ON`, `DMA_OFF`, or any `ll_priority_t` value to a `LiteLEDpio` initializer will result in a compile error.

<a name="rgbw-support"></a>
### RGBW Support

`LiteLEDpio` supports RGBW LED types (e.g., `LED_STRIP_SK6812`) identically to `LiteLED`. Pass `true` for the `rgbw` constructor argument and select an RGBW-capable `led_strip_type_t`. The `auto_w` parameter in `begin()` behaves the same as for `LiteLED` — see [Regarding RGBW Strips](#regarding-rgbw-strips).

The PARLIO encoder handles 4-byte-per-pixel RGBW strips correctly: each input byte is independently encoded to 24 DMA bytes, so each RGBW LED produces 4 × 24 = 96 DMA bytes.

<a name="driving-multiple-strips-in-parallel"></a>
### Driving Multiple Strips in Parallel

The PARLIO peripheral includes up to 8 data output lines (`data_gpio_nums[0..7]`) that all carry the **same bitstream simultaneously**. This is a fundamental hardware property: all active output lines always see identical data, and there is no way to send different data to different pins within one PARLIO TX unit.

Because all output lines share one bitstream, up to 8 LED strips can be driven concurrently from a single `LiteLEDpio` instance — all strips receive identical colour data with perfectly synchronised timing.

> **Note:** `LiteLEDpio` drives a single strip on `data_gpio_nums[0]` only (the `data_pin` passed to `begin()`). The other 7 PARLIO output lines are routed to `GPIO_NUM_NC` and carry no signal. For driving multiple independent strips simultaneously — each with its own content and colour data — use `LiteLEDpioGroup`. See [LiteLEDpioGroup Class](#liteLEDpioGroup-class).

<a name="class-interface"></a>
### Class Interface

```cpp
class LiteLEDpio {
public:
    LiteLEDpio(led_strip_type_t led_type, bool rgbw);
    ~LiteLEDpio();

    // Initialization — no DMA flag or interrupt priority (PARLIO always uses GDMA; no ISR)
    esp_err_t begin(uint8_t data_pin, size_t length, bool auto_w = true);
    esp_err_t begin(uint8_t data_pin, size_t length, ll_psram_t psram_flag, bool auto_w = true);

    // All display, pixel, colour order, and instance management methods are
    // identical in name, parameters, and return type to LiteLED.
    esp_err_t show();
    esp_err_t clear(bool show = false);
    esp_err_t brightness(uint8_t bright, bool show = false);
    uint8_t   getBrightness();
    esp_err_t setPixel(size_t num, rgb_t color, bool show = false);
    esp_err_t setPixel(size_t num, crgb_t color, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, rgb_t *data, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, crgb_t *data, bool show = false);
    esp_err_t fill(rgb_t color, bool show = false);
    esp_err_t fill(crgb_t color, bool show = false);
    esp_err_t fillRandom(bool show = false);
    rgb_t     getPixel(size_t num);
    crgb_t    getPixelC(size_t num);
    esp_err_t setOrder(color_order_t led_order = ORDER_GRB);
    esp_err_t resetOrder();
    bool      isValid() const;
    int       getGpioPin() const;
    static bool    isGpioAvailable(uint8_t gpio_pin);
    static uint8_t getActiveInstanceCount();
};
```

**Switching between drivers** is a one-line change:

```cpp
// RMT driver
LiteLED    myStrip(LED_STRIP_WS2812, false);

// PARLIO driver — same API, same begin/show/setPixel calls
LiteLEDpio myStrip(LED_STRIP_WS2812, false);
```

**Differences from `LiteLED`:**

| Feature | `LiteLED` (RMT) | `LiteLEDpio` (PARLIO) |
|---|---|---|
| `begin()` full config | `begin(pin, len, dma, priority, psram)` | `begin(pin, len, psram)` |
| DMA control | `DMA_ON` / `DMA_OFF` | Always uses GDMA (no flag) |
| Interrupt priority | `PRIORITY_HIGH` etc. | Not applicable (no ISR) |
| Pixel buffer in PSRAM | Yes, via `psram_flag` | Yes, via `psram_flag` |
| DMA bitstream buffer | Configurable | Always internal DMA RAM |
| RGBW support | Yes | Yes, identical behaviour |
| Max concurrent instances | Up to 8 (RMT channels) | 1 on ESP32-C6 / ESP32-H2 |
| SoC availability | All ESP32 with RMT | `SOC_PARLIO_SUPPORTED` only |

---

<a name="liteledpiogroup-class"></a>
# LiteLEDpioGroup Class

Drives up to 8 independent LED strips simultaneously from a single PARLIO TX unit. Each strip occupies one **bit-lane** of the PARLIO data bus; every `show()` call encodes all lane pixel buffers into a shared DMA bitstream and transmits them in a single GDMA transfer — all strips update in perfect lock-step with zero per-strip CPU overhead.

**Availability:** `SOC_PARLIO_SUPPORTED` only (ESP32-C6, ESP32-H2, ESP32-P4, …).

**Constraints:**
- All strips must share the same `led_strip_type_t`, strip length, and RGBW flag.
- Maximum strips: `PARLIO_TX_UNIT_MAX_DATA_WIDTH` — 8 on ESP32-C6/H2, 16 on ESP32-P4.
- Only one `LiteLEDpioGroup` (or `LiteLEDpio`) can be active at a time on C6/H2.
- `show()` on any lane or the group always transmits **all** lanes; there is no per-lane-only transmit.

<a name="constructor-1"></a>
### Constructor

```cpp
LiteLEDpioGroup(led_strip_type_t led_type, size_t length, bool rgbw);
```

Defines the shared configuration for every lane. No hardware is allocated until `begin()` is called.

| Parameter | Type | Description |
|-----------|------|-------------|
| `led_type` | `led_strip_type_t` | LED strip protocol (e.g., `LED_STRIP_WS2812`) |
| `length` | `size_t` | Number of LEDs per strip (same for all lanes) |
| `rgbw` | `bool` | `true` for RGBW strips; `false` for RGB |

<a name="destructor-1"></a>
### Destructor

```cpp
~LiteLEDpioGroup();
```

Waits for any in-progress GDMA transfer, disables and deletes the PARLIO TX unit, frees all per-lane pixel buffers and the shared DMA bitstream buffer, and unregisters all GPIOs from the Peripheral Manager.

<a name="strip-registration-addstrip"></a>
### Strip Registration — `addStrip()`

Strips must be registered **before** calling `begin()`. Two forms are available:

<a name="sequential-lane-assignment"></a>
#### Sequential lane assignment

```cpp
LiteLEDpioLane &addStrip(uint8_t gpio);
```

Assigns the strip to the next available lane (0, 1, 2 …). Returns a `LiteLEDpioLane` reference for per-strip pixel manipulation. Returns a silent null lane (with an error log) if all lanes are already assigned.

| Parameter | Type | Description |
|-----------|------|-------------|
| `gpio` | `uint8_t` | GPIO pin connected to the strip DIN |

<a name="explicit-lane-assignment"></a>
#### Explicit lane assignment

```cpp
template<uint8_t LANE>
LiteLEDpioLane &addStrip(uint8_t gpio);
```

`LANE` is verified against `PARLIO_TX_UNIT_MAX_DATA_WIDTH` at compile time via `static_assert`.

The lane number has no physical significance — it is an internal index that determines which bit position in the shared DMA buffer carries that strip's signal, and which `data_gpio_nums[N]` slot the GPIO occupies. User code normally accesses strips by their named `LiteLEDpioLane&` references, not by index.

Use the explicit form when the lane **index** itself matters to your code:

- **Sparse assignment** — deliberately leaving lane slots empty for future hardware expansion or to reserve specific index positions.
- **External index storage** — other code records lane numbers and retrieves strips via `strips[n]` later; explicit assignment gives a guaranteed, init-order-independent index.
- **Documentation of intent** — making the lane-to-strip mapping explicit in the source for maintainability.

Both forms give identical freedom to choose any GPIO pin; the explicit form only additionally controls the lane index.

```cpp
// Example: sparse explicit lanes — lanes 1, 2 left empty for future use
LiteLEDpioGroup strips(LED_STRIP_WS2812, 64, false);
LiteLEDpioLane &leftPanel  = strips.addStrip<0>(21);
LiteLEDpioLane &rightPanel = strips.addStrip<3>(19);
```

<a name="begin"></a>
### `begin()`

```cpp
esp_err_t begin(ll_psram_t psram_flag = PSRAM_DISABLE);
```

Allocates per-lane pixel colour buffers, allocates the shared DMA bitstream buffer (always internal DMA-capable RAM), creates and enables the PARLIO TX unit, and registers all assigned GPIOs with the ESP32 Peripheral Manager. Must be called after all `addStrip()` calls and before any pixel operations or `show()`.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `psram_flag` | `ll_psram_t` | `PSRAM_DISABLE` | PSRAM preference for **all** lane pixel colour buffers. Shared DMA buffer is always in internal RAM. |

Returns `ESP_OK` on success, or an `esp_err_t` error code on failure (GPIO conflict, memory allocation failure, etc.).

<a name="show"></a>
### `show()`

```cpp
esp_err_t show();
```

Encodes all assigned lane pixel buffers into the shared DMA bitstream buffer and transmits the frame via a single GDMA transfer. Blocks until transmission (including the reset/latch period) is complete. All strips update simultaneously.

<a name="brightness-getbrightness"></a>
### `brightness()` / `getBrightness()`

```cpp
esp_err_t brightness(uint8_t bright, bool show = false);
uint8_t   getBrightness();
```

Sets or gets the brightness applied to all lanes equally. The brightness is applied non-destructively during `show()` encoding; pixel buffer values are not modified. If `show` is `true`, calls `show()` automatically after updating the brightness.

<a name="operatorlane"></a>
### `operator[](lane)`

```cpp
LiteLEDpioLane &operator[](uint8_t lane);
```

Returns the `LiteLEDpioLane` reference for the given bit-lane index. Returns a silent null lane if the index is out of range or was never registered via `addStrip()`.

<a name="class-interface-1"></a>
### Class Interface

```cpp
class LiteLEDpioGroup {
public:
    LiteLEDpioGroup(led_strip_type_t led_type, size_t length, bool rgbw);
    ~LiteLEDpioGroup();

    LiteLEDpioLane &addStrip(uint8_t gpio);               // sequential
    template<uint8_t LANE>
    LiteLEDpioLane &addStrip(uint8_t gpio);               // explicit lane

    esp_err_t begin(ll_psram_t psram_flag = PSRAM_DISABLE);
    esp_err_t show();
    esp_err_t brightness(uint8_t bright, bool show = false);
    uint8_t   getBrightness();
    LiteLEDpioLane &operator[](uint8_t lane);
    bool isValid() const;
};
```

<a name="quick-example"></a>
### Quick Example

```cpp
#include <LiteLED.h>

LiteLEDpioGroup strips(LED_STRIP_WS2812, 64, false);
LiteLEDpioLane *panelA = nullptr;
LiteLEDpioLane *panelB = nullptr;

void setup() {
    panelA = &strips.addStrip(21);   // lane 0 → GPIO 21
    panelB = &strips.addStrip(19);   // lane 1 → GPIO 19

    if (strips.begin() != ESP_OK) {
        // handle error
    }
    strips.brightness(20);
}

void loop() {
    panelA->fill(0x200000);   // panel A: red
    panelB->fill(0x000020);   // panel B: blue
    strips.show();             // both panels update simultaneously
    delay(1000);

    panelA->fill(0x002000);   // panel A: green
    panelB->clear();          // panel B: off
    strips.show();
    delay(1000);
}
```

---

<a name="liteledpiolane-class"></a>
# LiteLEDpioLane Class

A thin handle for one bit-lane within a `LiteLEDpioGroup`. `LiteLEDpioLane` objects are **owned by the group** and never constructed directly by user code. Obtain a reference from `addStrip()` or `operator[]`.

**Pixel API:** Identical in name, parameters, and return types to `LiteLED` and `LiteLEDpio`.

**`show()` semantics:** Calling `show()` on a lane is equivalent to calling `show()` on the parent group — it always encodes and transmits **all** lanes, not just the one lane. This guarantees that no lane is ever out of sync with its peers.

<a name="isvalid"></a>
### `isValid()`

```cpp
bool isValid() const;
```

Returns `true` if the lane was properly initialised via `addStrip()`. A null-lane sentinel (returned by `operator[]` for an unregistered lane) returns `false`.

<a name="class-interface-2"></a>
### Class Interface

```cpp
class LiteLEDpioLane {
public:
    esp_err_t show();
    esp_err_t setPixel(size_t num, rgb_t  color, bool show = false);
    esp_err_t setPixel(size_t num, crgb_t color, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, rgb_t  *data, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, crgb_t *data, bool show = false);
    esp_err_t fill(rgb_t  color, bool show = false);
    esp_err_t fill(crgb_t color, bool show = false);
    esp_err_t clear(bool show = false);
    esp_err_t brightness(uint8_t bright, bool show = false);
    uint8_t   getBrightness();
    rgb_t     getPixel(size_t num);
    crgb_t    getPixelC(size_t num);
    esp_err_t fillRandom(bool show = false);
    esp_err_t setOrder(color_order_t led_order = ORDER_GRB);
    esp_err_t resetOrder();
    bool      isValid() const;
};
```

---

<a name="enumerations"></a>
# Enumerations


<a name="led_strip_type_t"></a>
## `led_strip_type_t`

Supported LED strip types.

```cpp
    LED_STRIP_WS2812         // WS2812/WS2812B (GRB colour order)
    LED_STRIP_WS2812_RGB     // WS2812 variant with RGB colour order
    LED_STRIP_SK6812         // SK6812 (GRB colour order, with RGBW support)
    LED_STRIP_APA106         // APA106 (RGB colour order)
    LED_STRIP_SM16703        // SM16703 (RGB colour order)

```

**Description**:

Defines the LED strip type, which determines timing parameters and default colour order.

---

<a name="color_order_t"></a>
## `color_order_t`

LED colour byte ordering.

```cpp
    ORDER_RGB.        // Red, Green, Blue
    ORDER_RBG         // Red, Blue, Green
    ORDER_GRB         // Green, Red, Blue
    ORDER_GBR         // Green, Blue, Red
    ORDER_BRG         // Blue, Red, Green
    ORDER_BGR         // Blue, Green, Red
```

**Description**:

Specifies the byte order for transmitting colour data to LEDs. Most WS2812 strips use GRB order by default.

**Default Colour Orders by LED Type**:

- WS2812:       `ORDER_GRB`
- WS2812_RGB:   `ORDER_RGB`
- SK6812:       `ORDER_GRB`
- APA106:       `ORDER_RGB`
- SM16703:      `ORDER_RGB`

---

<a name="ll_dma_t"></a>
## `ll_dma_t`

```cpp
    DMA_ON       // Enable DMA for RMT transfers
    DMA_OFF      // Disable DMA (use internal RMT memory)
    DMA_DEFAULT  // Default behaviour - equivalent to DMA_OFF

```

**Description**:

Controls whether the RMT peripheral uses DMA for data transfers. DMA can improve performance for long LED strips.

**Availability**:

Only supported on ESP32 variants with RMT DMA support. The library will issue a compile-time warning if DMA is not available on the selected chip.

**Note**:

- Some ESP32 models do not support RMT DMA. Check the data sheet of the SoC.
- When using DMA, the total number of available RMT channels will be reduced.

---

<a name="ll_priority_t"></a>
## `ll_priority_t`

RMT interrupt priority level

```cpp
    PRIORITY_DEFAULT  // Default interrupt priority
    PRIORITY_HIGH     // High priority
    PRIORITY_MED      // Medium priority
    PRIORITY_LOW      // Low priority
```

**Description:** Sets the interrupt priority for the RMT encoder callback. Higher priority ensures more precise timing for LED updates.

**Availability:** Only supported with arduino-esp32 core v3.0.2 or greater. The library will issue a compile-time warning if not available.

---

<a name="ll_psram_t"></a>
## `ll_psram_t`

PSRAM buffer allocation preference.

```cpp
    PSRAM_DISABLE      // Allocate LED buffer in internal RAM
    PSRAM_ENABLE       // Allocate LED buffer in PSRAM (if available)
    PSRAM_AUTO         // Automatically use PSRAM if available
```

**Description**: Controls whether the LED buffer is allocated in internal RAM or external PSRAM. PSRAM is useful for large LED arrays (hundreds to thousands of LEDs).

**Recommendations**:

- **Small arrays** (<100 LEDs): Use `PSRAM_DISABLE` for best performance
- **Large arrays** (>500 LEDs): Use `PSRAM_ENABLE` or `PSRAM_AUTO`
- **Unknown requirements**: Use `PSRAM_AUTO` for automatic selection

---

<a name="structures"></a>
# Structures

<a name="rgb_t"></a>
## `rgb_t`

RGB colour representation (struct style).

```cpp
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

**Description**: Represents an RGB colour with 8-bit values per channel (0-255).

**Access Methods**:

```cpp
rgb_t color;
color.r = 255;        // or color.red = 255;
color.g = 128;        // or color.green = 128;
color.b = 0;          // or color.blue = 0;
```
---

<a name="crgb_t"></a>
## `crgb_t`

RGB colour code (32-bit hex format).

```cpp
typedef uint32_t crgb_t;
```

**Description**: Represents an RGB colour as a 32-bit integer in format `0x00RRGGBB`.

**Examples**:

```cpp
crgb_t red   = 0xFF0000;
crgb_t green = 0x00FF00;
crgb_t blue  = 0x0000FF;
crgb_t white = 0xFFFFFF;
```

---

<a name="library-api"></a>
# Library API

<a name="constructor-2"></a>
## Constructor

<a name="liteled"></a>
### `LiteLED()`

Creates a new LiteLED instance.

```cpp
LiteLED(led_strip_type_t led_type, bool rgbw);
```

**Parameters**:

- `led_type`: Type of LED strip (see [`led_strip_type_t`](#led_strip_type_t))
- `rgbw`: Set to `true` for RGBW strips, `false` for RGB strips

**Example**:

```cpp
// Create RGB WS2812 strip
LiteLED strip(LED_STRIP_WS2812, false);

// Create RGBW SK6812 strip
LiteLED rgbwStrip(LED_STRIP_SK6812, true);
```

**Notes**:

- Does not allocate hardware resources (call a `begin()` method to initialize)
- Default brightness is 255 (full brightness)
- Multiple instances can be created on different GPIO pins

---

<a name="destructor-2"></a>
## Destructor

<a name="liteled-1"></a>
### `~LiteLED()`

Destroys the LiteLED instance and frees all resources.

```cpp
~LiteLED();
```

**Description**:

Automatically called when the object goes out of scope.

Frees:

- LED colour buffer (RAM or PSRAM)
- RMT channel
- RMT encoder
- Peripheral Manager GPIO registration
- Registry tracking entry

---

<a name="initialization-methods"></a>
## Initialization Methods

<a name="begin-basic"></a>
### `begin()` - Basic

Initialize LED strip with default settings.

```cpp
esp_err_t begin(uint8_t data_pin, size_t length, bool auto_w = true);
```

**Parameters**:

- `data_pin`: GPIO pin connected to LED strip data input (DIN)
- `length`: Number of LEDs in the strip
- `auto_w`: For RGBW strips, automatically calculate white channel (default: `true`)

**Returns**:

- `ESP_OK`: Success
- `ESP_ERR_INVALID_ARG`: Invalid GPIO pin
- `ESP_ERR_INVALID_STATE`: GPIO pin already in use
- `ESP_ERR_NO_MEM`: Failed to allocate memory

**Example**:

```cpp
LiteLED strip( LED_STRIP_WS2812, false );
if ( strip.begin( 14, 60 ) == ESP_OK ) {
    // Strip initialized successfully
}
```

**Notes**:

- Allocates LED buffer in internal RAM
- Uses default RMT settings (no DMA, default priority)
- Registers GPIO with Peripheral Manager

---

<a name="begin-psram"></a>
### `begin()` - PSRAM

Initialize LED strip with PSRAM control.

```cpp
esp_err_t begin(uint8_t data_pin, size_t length, ll_psram_t psram_flag, bool auto_w = true);
```

**Parameters**:

- `data_pin`: GPIO pin connected to LED strip data input
- `length`: Number of LEDs in the strip
- `psram_flag`: PSRAM allocation preference (see [`ll_psram_t`](#ll_psram_t))
- `auto_w`: For RGBW strips, automatically calculate white channel (default: `true`)

**Returns**:

Same as basic `begin()`

**Example**:

```cpp
// Large strip using PSRAM
LiteLED largeStrip(LED_STRIP_WS2812, false);
largeStrip.begin(14, 1000, PSRAM_AUTO);

// Small strip forcing internal RAM
LiteLED smallStrip(LED_STRIP_WS2812, false);
smallStrip.begin(15, 30, PSRAM_DISABLE);
```

---

<a name="begin-full-configuration"></a>
### `begin()` - Full Configuration

Initialize LED strip with all advanced options.

```cpp
esp_err_t begin( uint8_t data_pin, size_t length, ll_dma_t dma_flag, 
                ll_priority_t priority, ll_psram_t psram_flag, bool auto_w = true );
```

**Parameters**:

- `data_pin`: GPIO pin connected to LED strip data input
- `length`: Number of LEDs in the strip
- `dma_flag`: DMA usage setting (see [`ll_dma_t`](#ll_dma_t))
- `priority`: Interrupt priority (see [`ll_priority_t`](#ll_priority_t))
- `psram_flag`: PSRAM allocation preference (see [`ll_psram_t`](#ll_psram_t))
- `auto_w`: For RGBW strips, automatically calculate white channel (default: `true`)

**Returns**:

Same as basic `begin()`

**Example**:

```cpp
// High-performance configuration with DMA
LiteLED perfStrip(LED_STRIP_WS2812, false);
perfStrip.begin(14, 500, DMA_ON, PRIORITY_HIGH, PSRAM_AUTO);
```

**Notes**:

- DMA only works where supported by the ESP32 variant
- Use of DMA will reduce the total number of available RMT channels
- Setting interrupt priority is only supported when using arduino-esp32 core v3.2.0 and greater
- Invalid options are automatically handled (library issues warnings)

---

<a name="display-control-methods"></a>
## Display Control Methods

<a name="show-1"></a>
### `show()`

Send the LED buffer data to the strip.

```cpp
esp_err_t show();
```

**Returns**:

- `ESP_OK`: Success
- Error codes from RMT transmit operation

**Description**:

LiteLED maintains a buffer in memory that holds the colour data for each of the LED's in the strip.

This data does not affect the colour of the LED's until a `show()` or a method that calls `show()` is used which transmits the LED buffer to the physical LED strip using the RMT peripheral.

This is a blocking operation that waits for transmission to complete.

**Example**:

```cpp
strip.setPixel(0, rgb_from_code(0xFF0000));  // Set first LED to red
strip.setPixel(1, rgb_from_code(0x00FF00));  // Set second LED to green
strip.show();  // Update the strip
```

---

<a name="clear"></a>
### `clear()`

Set all LEDs to black (off), optionally update strip.

```cpp
esp_err_t clear(bool show = false);
```

**Parameters**:

- `show`: If `true`, immediately update the strip (default: `false`)

**Returns**:

`ESP_OK` on success

**Example**:

```cpp
// Clear buffer but don't update strip yet
strip.clear();

// Clear buffer and immediately update strip
strip.clear(true);
```

---

<a name="brightness"></a>
### `brightness()`

Set global brightness level, optionally update strip.

```cpp
esp_err_t brightness(uint8_t bright, bool show = false);
```

**Parameters**:

- `bright`: Brightness level (0-255, where 0=off, 255=full brightness)
- `show`: If `true`, immediately update the strip (default: `false`)

**Returns**:

`ESP_OK` on success

**Description**: Sets global brightness without modifying the colour values in the buffer. This is applied during transmission to the LED strip.

**Example**:

```cpp
// Set brightness to 50%
strip.brightness(128);
strip.show();

// Set brightness to 20% and update immediately
strip.brightness(51, true);
```

**Notes**:

- Valid range is 0-255
- Brightness scaling uses `scale8_video()` for smooth dimming
- Setting brightness to 0 turns off all LEDs
- Colour values in the buffer remain unchanged

---

<a name="getbrightness"></a>
### `getBrightness()`

Get current brightness level.

```cpp
uint8_t getBrightness();
```

**Returns**: Current brightness value (0-255)

**Example**:

```cpp
uint8_t current = strip.getBrightness();
Serial.printf("Current brightness: %d\n", current);
```

**Notes:**

The method returns the actual operating intensity value of the strip. That is, it returns the brightness value used the last time a `show()` or method that calls `show()` was called. 

If `getBrightness()` is used after using `brightness()` but before a `show()` or method that calls `show()` was used, the return value will be what was set with the `brightness()` method before the last call to `show()`.

---

<a name="pixel-manipulation-methods"></a>
## Pixel Manipulation Methods

<a name="setpixel-rgb_t"></a>
### `setPixel()` - `rgb_t`

Set single LED colour using `rgb_t` structure.

```cpp
esp_err_t setPixel(size_t num, rgb_t color, bool show = false);
```

**Parameters**:

- `num`: LED index (0-based)
- `color`: Colour as `rgb_t` structure
- `show`: If `true`, immediately update the strip (default: `false`)

**Returns**:

- `ESP_OK`: Success
- `ESP_ERR_INVALID_ARG`: LED index out of bounds or strip not initialized

**Example**:

```cpp
rgb_t red = rgb_from_values(255, 0, 0);
strip.setPixel(0, red);
strip.show();

// Set and show immediately
strip.setPixel(5, rgb_from_values(0, 255, 0), true);
```

---

<a name="setpixel-crgb_t"></a>
### `setPixel()` - `crgb_t`

Set the colour of a single LED using 32-bit 
 code.

```cpp
esp_err_t setPixel(size_t num, crgb_t color, bool show = false);
```

**Parameters**:

- `num`: LED index (0-based)
- `color`: Colour as `crgb_t` 32-bit code (`0x00RRGGBB`)
- `show`: If `true`, immediately update the strip (default: `false`)

**Returns**:

Same as `rgb_t` version

**Example**:

```cpp
strip.setPixel(0, 0xFF0000);  // Red
strip.setPixel(1, 0x00FF00);  // Green
strip.setPixel(2, 0x0000FF);  // Blue
strip.show();
```

---

<a name="setpixels-rgb_t-array"></a>
### `setPixels()` - `rgb_t` Array

Set multiple consecutive LEDs from an `rgb_t` array.

```cpp
esp_err_t setPixels(size_t start, size_t len, rgb_t *data, bool show = false);
```

**Parameters**:

- `start`: First LED index (0-based)
- `len`: Number of LEDs to set
- `data`: Pointer to array of `rgb_t` colours
- `show`: If `true`, immediately update the strip (default: `false`)

**Returns**:

- `ESP_OK`: Success
- `ESP_ERR_INVALID_ARG`: Invalid parameters or out of bounds

**Example**:

```cpp
rgb_t rainbow[] = {
    rgb_from_code(0xFF0000),  // Red
    rgb_from_code(0xFF7F00),  // Orange
    rgb_from_code(0xFFFF00),  // Yellow
    rgb_from_code(0x00FF00),  // Green
    rgb_from_code(0x0000FF),  // Blue
    rgb_from_code(0x8B00FF)   // Violet
};
strip.setPixels(0, 6, rainbow, true);
```

---

<a name="setpixels-crgb_t-array"></a>
### `setPixels()` - `crgb_t` Array

Set multiple consecutive LEDs from a `crgb_t` array.

```cpp
esp_err_t setPixels(size_t start, size_t len, crgb_t *data, bool show = false);
```

**Parameters**:

- `start`: First LED index (0-based)
- `len`: Number of LEDs to set
- `data`: Pointer to array of 32-bit colour codes
- `show`: If `true`, immediately update the strip (default: `false`)

**Returns**:

Same as `rgb_t` version

**Example**:

```cpp
crgb_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF};
strip.setPixels(10, 3, colors);
strip.show();
```

---

<a name="fill-rgb_t"></a>
### `fill()` - `rgb_t`

Fill entire strip with a single colour.

```cpp
esp_err_t fill(rgb_t color, bool show = false);
```

**Parameters**:

- `color`: Colour as `rgb_t` structure
- `show`: If `true`, immediately update the strip (default: `false`)

**Returns**:

`ESP_OK` on success

**Example**:

```cpp
// Fill with blue
strip.fill(rgb_from_values(0, 0, 255), true);

// Fill with custom colour
rgb_t purple = {128, 0, 128};
strip.fill(purple);
strip.show();
```

---

<a name="fill-crgb_t"></a>
### `fill()` - `crgb_t`

Fill entire strip with a single colour code.

```cpp
esp_err_t fill(crgb_t color, bool show = false);
```

**Parameters**:

- `color`: Colour as 32-bit code (`0x00RRGGBB`)
- `show`: If `true`, immediately update the strip (default: `false`)

**Returns**:

`ESP_OK` on success

**Example**:

```cpp
// Fill with red and show immediately
strip.fill(0xFF0000, true);
```

---

<a name="fillrandom"></a>
### `fillRandom()`

Fill strip with random colours.

```cpp
esp_err_t fillRandom(bool show = false);
```

**Parameters**:

- `show`: If `true`, immediately update the strip (default: `false`)

**Returns**:

`ESP_OK` on success

**Description**:

Fills each LED with a random RGB colour using the ESP32's hardware random number generator.

**Example**:

```cpp
// Fill with random colours
strip.fillRandom(true);

// Create random pattern every second
void loop() {
    strip.fillRandom(true);
    delay(1000);
}
```
**Note:**

- Each colour channel of the LED is set independently to a random value between 5 and 255. Thus when using this method, the strip can be quite bright. The `brightness()` method can be used beforehand to lower the strip intensity.

---

<a name="pixel-reading-methods"></a>
## Pixel Reading Methods

<a name="getpixel"></a>
### `getPixel()`

Get colour of a single LED as `rgb_t` structure.

```cpp
rgb_t getPixel(size_t num);
```

**Parameters**:

- `num`: LED index (0-based)

**Returns**:

`rgb_t` structure with LED colour, or black (`{0,0,0}`) if invalid index

**Example**:

```cpp
rgb_t color = strip.getPixel(5);
Serial.printf("LED 5: R=%d, G=%d, B=%d\n", color.r, color.g, color.b);
```

**Note**:

- Returns the colour value in the buffer, not accounting for brightness scaling.

---

<a name="getpixelc"></a>
### `getPixelC()`

Get colour of a single LED as 32-bit colour code.

```cpp
crgb_t getPixelC(size_t num);
```

**Parameters**:

- `num`: LED index (0-based)

**Returns**: 32-bit colour code (`0x00RRGGBB`), or `0x000000` if invalid index

**Example**:

```cpp
crgb_t color = strip.getPixelC(10);
if (color == 0xFF0000) {
    Serial.println("LED 10 is red");
}
```

---

<a name="colour-order-methods"></a>
## Colour Order Methods

<a name="setorder"></a>
### `setOrder()`

Set custom colour byte order for the LED strip.

```cpp
esp_err_t setOrder(color_order_t led_order = ORDER_GRB);
```

**Parameters**:

- `led_order`: Colour order (see [`color_order_t`](#color_order_t))

**Returns**:

`ESP_OK` on success

**Description**:

Overrides the default colour order for the LED strip type. Useful for LED strips that don't match standard specifications.

**Example**:

```cpp
// Force RGB order instead of default GRB
strip.setOrder(ORDER_RGB);

// Some LED strips might need BGR
strip.setOrder(ORDER_BGR);
```

**Notes**:

- This method overrides the order set by the LED strip type. As all LED strip types have a defined colour order it is not required to call this method. It was added to address LED's with non-standard colour orders.

- Can be called any time after declaring the LiteLED object and takes effect after a call to `show()` or any call that invokes `show()` and remains in effect until another call to `setOrder()` or `resetOrder()` is made.

- FWIW, originally intended to be a "one and done" call, multiple calls can be made for creative effect.

---

<a name="resetorder"></a>
### `resetOrder()`

Reset colour order to the default for the LED strip type.

```cpp
esp_err_t resetOrder();
```

**Returns**:

`ESP_OK` on success

**Example**:

```cpp
// Restore default colour order
strip.resetOrder();
```

**Notes:**

- Can be called any time after declaring the LiteLED object and takes effect after a call to `show()` or any call that invokes `show()` and remains in effect until another call to `setOrder()` or `resetOrder()` is made.

---

<a name="instance-management-methods"></a>
## Instance Management Methods

<a name="isvalid-1"></a>
### `isValid()`

Check if this LiteLED instance is still valid.

```cpp
bool isValid() const;
```

**Returns**: 

- `true`: Instance is valid and GPIO pin is still owned by this instance
- `false`: GPIO pin was reassigned or instance is invalid

**Description**:

Checks with the LiteLED registry and Peripheral Manager to ensure the GPIO pin is still allocated to this instance.

**Example**:

```cpp
if (!strip.isValid()) {
    Serial.println("Warning: LED strip instance is no longer valid!");
    // Reinitialize or handle error
}
```

**Use Case**:

Useful in complex applications where GPIOs might be dynamically reassigned.

---

<a name="getgpiopin"></a>
### `getGpioPin()`

Get the GPIO pin number used by this instance.

```cpp
int getGpioPin() const;
```

**Returns**: 

- GPIO pin number if initialized
- `-1` if not initialized

**Example**:

```cpp
int pin = strip.getGpioPin();
if (pin >= 0) {
    Serial.printf("Strip using GPIO %d\n", pin);
}
```

---

<a name="isgpioavailable-static"></a>
### `isGpioAvailable()` - Static

Check if a GPIO pin is available for LiteLED use.

```cpp
static bool isGpioAvailable(uint8_t gpio_pin);
```

**Parameters:**

- `gpio_pin`: GPIO pin number to check

**Returns:**

- `true`: GPIO is available
- `false`: GPIO is in use by another peripheral

**Description:**

Checks with the Peripheral Manager to see if the GPIO is free.

**Example:**

```cpp
if (LiteLED::isGpioAvailable(14)) {
    Serial.println("GPIO 14 is available for LED strip");
} else {
    Serial.println("GPIO 14 is already in use");
}
```

---

<a name="getactiveinstancecount-static"></a>
### `getActiveInstanceCount()` - Static

Get count of active LiteLED instances.

```cpp
static uint8_t getActiveInstanceCount();
```

**Returns**:

Number of currently active LiteLED instances

**Example**:

```cpp
uint8_t count = LiteLED::getActiveInstanceCount();
Serial.printf("Active LED strip instances: %d\n", count);
```

---

<a name="advanced-features"></a>
# Advanced Features

<a name="multi-display-support"></a>
## Multi-Display Support

LiteLED provides advanced support for managing multiple LED displays simultaneously with automatic interrupt priority management and conflict resolution.

A maximum of eight displays are supported as at the time of writing that is the maximum number of available RMT channels of any ESP32 SoC.

<a name="simple-multi-display-setup"></a>
### Simple Multi-Display Setup

The recommended approach for most applications:

```cpp
LiteLED display1(LED_STRIP_WS2812, false);
LiteLED display2(LED_STRIP_WS2812, false);
LiteLED display3(LED_STRIP_WS2812, false);

void setup() {
    // Simple initialization - automatic priority management
    display1.begin(14, 100);
    display2.begin(27, 100);
    display3.begin(26, 100);
    
    Serial.printf("Active displays: %d\n", LiteLED::getActiveInstanceCount());
}
```

<a name="advanced-multi-display-configuration"></a>
### Advanced Multi-Display Configuration

For applications with specific requirements:

```cpp
void setup() {
    // High priority display for time-critical applications
    display1.begin(14, 100, DMA_OFF, PRIORITY_HIGH, PSRAM_DISABLE);
    
    // Automatic priority selection (recommended)
    display2.begin(27, 500, DMA_OFF, PRIORITY_DEFAULT, PSRAM_AUTO);
    
    // Simple initialization (best for most cases)
    display3.begin(26, 200);
}
```

<a name="automatic-priority-management"></a>
### Automatic Priority Management

LiteLED v3.0.0+ includes intelligent priority conflict resolution:

- **Pre-flight checks**: Verifies priority availability before allocation
- **Smart fallback**: Automatically selects alternative priorities when conflicts occur
- **Resource tracking**: Monitors and manages interrupt priority usage
- **Graceful degradation**: Continues operation even when requested priorities are unavailable

**Note:**

- Interrupt priority support requires arduino-esp32 core version 3.2.0 or greater.

<a name="priority-levels"></a>
#### Priority Levels

| Priority | Enumeration | Description |
|:----------:|:----------:|-------------|
| 0 | `PRIORITY_DEFAULT` | Default priority (recommended) |
| 1 | `PRIORITY_HIGH` | High priority for time-critical displays |
| 2 | `PRIORITY_MED` | Medium priority |
| 3 | `PRIORITY_LOW` | Low priority for background displays |

<a name="gpio-management"></a>
### GPIO Management

Integration with ESP32 Peripheral Manager ensures safe GPIO usage:

```cpp
// Check GPIO availability before use
if (LiteLED::isGpioAvailable(gpio_pin)) {
    display.begin(gpio_pin, num_leds);  // Safe to use
} else {
    Serial.printf("GPIO %d is already in use\n", gpio_pin);
}

// Get count of active instances
uint8_t active = LiteLED::getActiveInstanceCount();
Serial.printf("Currently managing %d displays\n", active);
```

<a name="best-practices-for-multi-display"></a>
### Best Practices for Multi-Display

1. **Use simple initialization** when possible:

   ```cpp
   display.begin(gpio, leds);  // Recommended
   ```

2. **Check initialization results**:

   ```cpp
   esp_err_t result = display.begin(gpio, leds);
   if (result != ESP_OK) {
       Serial.printf("Failed: %s\n", esp_err_to_name(result));
   }
   ```

3. **Monitor resources**:

   ```cpp
   Serial.printf("Active: %d\n", LiteLED::getActiveInstanceCount());
   ```
---

<a name="psram-for-large-arrays"></a>
## PSRAM for Large Arrays

PSRAM is useful for LED arrays with hundreds of LEDs:

```cpp
// Auto-detect and use PSRAM if available
LiteLED bigStrip(LED_STRIP_WS2812, false);
bigStrip.begin(14, 2000, PSRAM_AUTO);

// Force PSRAM usage
bigStrip.begin(14, 2000, PSRAM_ENABLE);

// Force internal RAM (fastest)
bigStrip.begin(14, 100, PSRAM_DISABLE);
```

**PSRAM Considerations**:

- **Slightly slower** than internal RAM (~5-10% performance impact)
- **Essential for large arrays** (>500 LEDs) on ESP32 with limited RAM
- **Automatically detected** with `PSRAM_AUTO`

---

<a name="instance-validation"></a>
### Instance Validation

In dynamic applications where GPIO pins might be reassigned:

```cpp
void loop() {
    if (!strip.isValid()) {
        Serial.println("Warning: Strip instance invalidated!");
        // Attempt to reinitialize
        if (strip.begin(14, 60) == ESP_OK) {
            Serial.println("Strip reinitialized");
        }
    }
    
    // Normal operation
    strip.fillRandom(true);
    delay(1000);
}
```

---

<a name="utilities"></a>
# Utilities

<a name="liteled_utils"></a>
## LiteLED_Utils

The `LiteLED_Utils` namespace provides compile-time utility functions for querying hardware capabilities. These functions allow developers to check platform support for various features before configuring their LED strips.

<a name="isdmasupported"></a>
### `isDmaSupported()`

**Description:**  

Checks at compile time whether the current ESP32 chip supports RMT DMA (Direct Memory Access) for LED strip operations.

**Syntax:**

```cpp
constexpr bool LiteLED_Utils::isDmaSupported()
```

**Parameters:**

None

**Returns:**

- `true` - RMT DMA is supported on this ESP32 model
- `false` - RMT DMA is not supported on this ESP32 model

**Notes:**

- This is a `constexpr` function, so the result is determined at compile time
- On chips without RMT DMA support (ESP32, ESP32-C2, ESP32-C6), this returns `false`
- On chips with RMT DMA support (ESP32-S2, ESP32-S3, ESP32-C3, ESP32-H2), this returns `true`
- If you attempt to use `DMA_ON` on unsupported hardware, the library will automatically fall back to `DMA_OFF` with a warning

**Example Usage:**

```cpp
#include <LiteLED.h>

void setup() {
    Serial.begin(115200);
    
    // Query DMA support at compile time
    if (LiteLED_Utils::isDmaSupported()) {
        Serial.println("This chip supports RMT DMA");
    }
    else {
        Serial.println("This chip does not support RMT DMA");
    }
    
    // Use in conditional initialization
    LiteLED strip(LED_STRIP_WS2812, false);
    if (LiteLED_Utils::isDmaSupported()) {
        // Enable DMA for better performance on supported chips
        strip.begin(14, 100, DMA_ON, PRIORITY_DEFAULT, PSRAM_AUTO);
    }
    else {
        // Use non-DMA mode on chips without support
        strip.begin(14, 100, DMA_OFF, PRIORITY_DEFAULT, PSRAM_AUTO);
    }
}
```

---

<a name="isprioritysupported"></a>
### `isPrioritySupported()`

**Description:**
Checks at compile time whether the current ESP-IDF/arduino-esp32 core version supports setting RMT interrupt priority levels.

**Syntax:**

```cpp
constexpr bool LiteLED_Utils::isPrioritySupported()
```

**Parameters:**  

None

**Returns:**

- `true` - Interrupt priority setting is supported (ESP-IDF 5.1.2 or later)
- `false` - Interrupt priority setting is not supported (older ESP-IDF versions)

**Notes:**

- This is a `constexpr` function, so the result is determined at compile time
- Requires ESP-IDF version 5.1.2 or higher for support
- If priority setting is not supported, the library will use the default priority (0) and log a note
- Attempting to set priority on unsupported versions is safe - the library gracefully falls back to defaults

**Example Usage:**

```cpp
#include <LiteLED.h>

void setup() {
    Serial.begin(115200);
    
    // Query interrupt priority support at compile time
    if (LiteLED_Utils::isPrioritySupported()) {
        Serial.println("This core version supports interrupt priority setting");
    } else {
        Serial.println("This core version uses default interrupt priority");
    }
    
    // Use in conditional configuration
    LiteLED strip(LED_STRIP_WS2812, false);
    if (LiteLED_Utils::isPrioritySupported()) {
        // Set custom priority on supported cores
        strip.begin(14, 100, DMA_OFF, PRIORITY_HIGH, PSRAM_AUTO);
    } else {
        // Priority parameter is ignored on older cores (uses default)
        strip.begin(14, 100, DMA_OFF, PRIORITY_DEFAULT, PSRAM_AUTO);
    }
}
```

---

<a name="complete-example"></a>
### Complete Example

Here's a comprehensive example showing how to use both utility functions together:

```cpp
#include <LiteLED.h>

#define LED_GPIO 14
#define LED_COUNT 100

LiteLED myStrip(LED_STRIP_WS2812, false);

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    // Display hardware capabilities
    Serial.println("=== LiteLED Hardware Capabilities ===");
    Serial.printf("RMT DMA Support: %s\n", 
                  LiteLED_Utils::isDmaSupported() ? "YES" : "NO");
    Serial.printf("Interrupt Priority Support: %s\n", 
                  LiteLED_Utils::isPrioritySupported() ? "YES" : "NO");
    Serial.printf("DMA_DEFAULT value: %s\n", 
                  (DMA_DEFAULT == DMA_ON) ? "DMA_ON" : "DMA_OFF");
    Serial.println("=====================================\n");
    
    // Smart initialization based on capabilities
    ll_dma_t dma_setting = DMA_DEFAULT;  // Let the library choose the best default
    ll_priority_t priority = PRIORITY_DEFAULT;
    
    // Optionally enable DMA on supported hardware if you want the performance boost
    if (LiteLED_Utils::isDmaSupported() && LED_COUNT > 300) {
        dma_setting = DMA_ON;  // DMA is beneficial for large strips
        Serial.println("Enabling DMA for large LED strip");
    }
    
    // Optionally set higher priority on supported cores if timing is critical
    if (LiteLED_Utils::isPrioritySupported()) {
        priority = PRIORITY_HIGH;
        Serial.println("Setting high interrupt priority");
    }
    
    // Initialize with optimal settings
    esp_err_t result = myStrip.begin(LED_GPIO, LED_COUNT, dma_setting, priority, PSRAM_AUTO);
    
    if (result == ESP_OK) {
        Serial.println("LED strip initialized successfully!");
        myStrip.brightness(50);
        myStrip.fill(0x00FF00, true);  // Green
    }
    else {
        Serial.printf("LED strip initialization failed: %s\n", esp_err_to_name(result));
    }
}

void loop() {
    // Your LED animation code here
}
```
<a name="integration-with-existing-code"></a>
### Integration with Existing Code

These utility functions are particularly useful when:

1. **Writing portable code** that runs on multiple ESP32 variants
2. **Creating libraries** that build on top of LiteLED
3. **Performance optimization** where you want to enable DMA only on supported chips
4. **Debugging** to understand platform limitations
5. **Compile-time configuration** using `if constexpr` in C++17 or later

<a name="related-configuration"></a>
### Related Configuration

<a name="dma_default-behaviour"></a>
#### `DMA_DEFAULT` Behaviour

The `DMA_DEFAULT` enum value is always set to `DMA_OFF` to preserve DMA channels for user applications. This is a conservative default that works on all ESP32 chips. Users who want DMA performance benefits should explicitly use `DMA_ON` after checking `isDmaSupported()`.

---

<a name="colour-utility-functions"></a>
## Colour Utility Functions

A number of functions are included to assist in manipulation and conversion of colours.

<a name="rgb_from_code"></a>
### `rgb_from_code()`

Convert a 32-bit colour code to `rgb_t` structure.

```cpp
static inline rgb_t rgb_from_code(crgb_t color_code);
```

**Parameters**:

- `color_code`: 32-bit colour in format `0x00RRGGBB`

**Returns**: `rgb_t` structure with separated R, G, B values

**Example**:

```cpp
rgb_t red = rgb_from_code(0xFF0000);
// red.r = 255, red.g = 0, red.b = 0
```

---

<a name="rgb_from_values"></a>
### `rgb_from_values()`

Create an `rgb_t` colour from individual channel values.

```cpp
static inline rgb_t rgb_from_values(uint8_t r, uint8_t g, uint8_t b);
```

**Parameters**:

- `r`: Red value (0-255)
- `g`: Green value (0-255)
- `b`: Blue value (0-255)

**Returns**: `rgb_t` structure

**Example**:

```cpp
rgb_t purple = rgb_from_values(128, 0, 128);
```

---

<a name="rgb_to_code"></a>
### `rgb_to_code()`

Convert an `rgb_t` structure to a 32-bit colour code.

```cpp
static inline crgb_t rgb_to_code(rgb_t color);
```

**Parameters**:

- `color`: `rgb_t` structure

**Returns**: 32-bit colour code in format `0x00RRGGBB`

**Example**:

```cpp
rgb_t color = {255, 128, 0};
crgb_t code = rgb_to_code(color);  // Returns 0xFF8000
```

---

<a name="rgb_is_zero"></a>
### `rgb_is_zero()`

Check if an RGB colour is black (all channels zero).

```cpp
static inline bool rgb_is_zero(rgb_t a);
```

**Parameters**:

- `a`: `rgb_t` colour to test

**Returns**: `true` if all colour channels are 0, `false` otherwise

**Example**:

```cpp
rgb_t black = {0, 0, 0};
if (rgb_is_zero(black)) {
    // Colour is black
}
```

---

<a name="rgb_luma"></a>
### `rgb_luma()`

Calculate the perceived brightness (luma) of an RGB colour.

```cpp
static inline uint8_t rgb_luma(rgb_t a);
```

**Parameters**:

- `a`: `rgb_t` color

**Returns**: Luma value (0-255)

**Description**: Calculates perceptual brightness using the formula:

```
Luma = (R * 54 + G * 183 + B * 18) / 256
```

This approximates human eye sensitivity where green contributes most to perceived brightness.

**Example**:

```cpp
rgb_t color = {100, 200, 50};
uint8_t brightness = rgb_luma(color);  // Returns ~158
```

---

<a name="scale8-and-scale8_video"></a>
### `scale8()` and `scale8_video()`

Scale an 8-bit value by a fractional amount.

```cpp
static inline uint8_t scale8(uint8_t i, fract8 scale);
static inline uint8_t scale8_video(uint8_t i, fract8 scale);
```

**Parameters**:

- `i`: Value to scale (0-255)
- `scale`: Scale factor (0-255, represents 0.0-1.0)

**Returns**:

Scaled value (0-255)

**Difference**:

- `scale8()`: Fast scaling, may produce zero for non-zero inputs
- `scale8_video()`: Guarantees non-zero output if both inputs are non-zero (better for LED dimming)

**Example**:

```cpp
uint8_t half_bright = scale8_video(255, 128);  // Returns 128
uint8_t dim = scale8_video(255, 10);           // Returns ~10
```

---


<a name="performance-considerations"></a>
# Performance Considerations

<a name="transmission-time"></a>
## Transmission Time

LED strip update time depends on the number of LEDs:

| LED Count | Transmission Time (approx.) |
|-----------|------------------------------|
| 30 LEDs | ~1 ms |
| 60 LEDs | ~2 ms |
| 150 LEDs | ~5 ms |
| 300 LEDs | ~10 ms |
| 600 LEDs | ~20 ms |
| 1000 LEDs | ~33 ms |

**Formula**: Time ≈ (LED_COUNT × 30µs)

<a name="memory-usage"></a>
## Memory Usage

Each driver allocates two distinct buffers: a **pixel colour buffer** that holds the R/G/B(/W) values you write via `setPixel()`, `fill()`, etc., and a **DMA bitstream buffer** that holds the pre-encoded waveform sent to the hardware. The two buffers have different size characteristics and different allocation rules.

<a name="liteled-rmt-memory"></a>
### LiteLED (RMT) Memory

The RMT driver encodes pixels on-the-fly inside an encoder callback, so there is **no pre-encoded DMA buffer**. Only the pixel colour buffer is heap-allocated.

| Strip type | Bytes per LED | 30 LEDs | 60 LEDs | 144 LEDs | 300 LEDs | 1000 LEDs |
|------------|:-------------:|--------:|--------:|---------:|---------:|----------:|
| RGB | 3 | 90 B | 180 B | 432 B | 900 B | 2.9 KB |
| RGBW | 4 | 120 B | 240 B | 576 B | 1.2 KB | 3.9 KB |

The pixel colour buffer can optionally be placed in PSRAM using the `psram_flag` parameter in `begin()`. When DMA is enabled on the RMT channel (via `DMA_ON` in the full `begin()` overload), the IDF RMT driver allocates additional internal DMA memory; this is managed internally and not reflected in the table above.

<a name="liteledpio-parlio-memory"></a>
### LiteLEDpio (PARLIO) Memory

The PARLIO driver **pre-encodes the entire waveform before transmission**. Each input byte expands to 24 DMA bytes (8 bits × 3 samples/bit), and a 1000-byte reset tail is appended and zero-filled. This DMA bitstream buffer is always allocated from **internal DMA-capable RAM** — GDMA cannot access PSRAM on C6/H2.

The pixel colour buffer follows the same rule as the RMT driver and can optionally be placed in PSRAM.

**RGB strips** (3 bytes/LED colour buffer · 72 DMA bytes/LED encoded)

| LEDs | Colour buffer | DMA buffer | Total heap | DMA buf (internal only) |
|-----:|:-------------:|:----------:|:----------:|:-----------------------:|
| 8 | 24 B | 1,576 B | 1,600 B | 1,576 B |
| 16 | 48 B | 2,152 B | 2,200 B | 2,152 B |
| 30 | 90 B | 3,160 B | 3,250 B | 3,160 B |
| 60 | 180 B | 5,320 B | 5,500 B | 5,320 B |
| 64 | 192 B | 5,608 B | 5,800 B | 5,608 B |
| 144 | 432 B | 11,368 B | 11.5 KB | 11,368 B |
| 256 | 768 B | 19,432 B | 20.0 KB | 19,432 B |
| 300 | 900 B | 22,600 B | 23.4 KB | 22,600 B |

**RGBW strips** (4 bytes/LED colour buffer · 96 DMA bytes/LED encoded)

| LEDs | Colour buffer | DMA buffer | Total heap | DMA buf (internal only) |
|-----:|:-------------:|:----------:|:----------:|:-----------------------:|
| 30 | 120 B | 3,880 B | 4,000 B | 3,880 B |
| 60 | 240 B | 6,760 B | 7,000 B | 6,760 B |
| 64 | 256 B | 7,144 B | 7,400 B | 7,144 B |
| 144 | 576 B | 14,824 B | 15.2 KB | 14,824 B |
| 256 | 1,024 B | 25,576 B | 26.4 KB | 25,576 B |

**DMA buffer formula** (total bytes, 4-byte aligned):

```
RGB:  floor(N × 72 + 1000 + 3) & ~3
RGBW: floor(N × 96 + 1000 + 3) & ~3
```

<a name="liteledpiogroup-memory"></a>
### LiteLEDpioGroup Memory

`LiteLEDpioGroup` allocates one pixel colour buffer per lane plus a **single shared DMA bitstream buffer** for all lanes. The DMA buffer size depends only on the strip length and LED type — it does not grow with the number of lanes, because the per-bit-lane encoding packs each lane into individual bits of each DMA byte without increasing the byte count.

This makes `LiteLEDpioGroup` significantly more memory-efficient than running N separate `LiteLEDpio` instances: a group pays one DMA buffer cost regardless of how many lanes are active.

**RGB strips, 64 LEDs per lane:**

| Configuration | Pixel buffers | DMA buffer | Total heap | Internal RAM |
|---|---:|---:|---:|---:|
| 1-lane group | 192 B | 5,608 B | 5,800 B | 5,800 B |
| 2-lane group | 384 B | 5,608 B | 5,992 B | 5,608 B minimum\* |
| 4-lane group | 768 B | 5,608 B | 6,376 B | 5,608 B minimum\* |
| 8-lane group | 1,536 B | 5,608 B | 7,144 B | 5,608 B minimum\* |
| 2 × separate `LiteLEDpio` | 2 × 192 = 384 B | 2 × 5,608 = 11,216 B | 11,600 B | 11,216 B |

\* Pixel colour buffers may reside in PSRAM via `begin(PSRAM_ENABLE)`, leaving only the shared DMA buffer in internal RAM.

DMA buffer size formula (same as single-strip; independent of lane count):

```
RGB:  floor(N × 72 + 1000 + 3) & ~3
RGBW: floor(N × 96 + 1000 + 3) & ~3
```

<a name="total-internal-ram-footprint"></a>
### Total Internal RAM Footprint

The critical constraint is **internal DMA-capable RAM**. On the ESP32-C6 this is shared system SRAM (~400 KB usable after IDF overhead). For `LiteLEDpio` the DMA buffer is the dominant cost:

| Driver | Pixel buffer location | DMA buffer location | Internal RAM used (RGB, 64 LEDs) |
|--------|-----------------------|---------------------|---------------------------------:|
| `LiteLED` (no DMA) | configurable | none | 192 B |
| `LiteLED` (DMA on) | configurable | internal (IDF-managed) | ~192 B + IDF overhead |
| `LiteLEDpio` | configurable (PSRAM ok) | **always internal** | **5,608 B** (+ 192 B if colour buf also internal) |
| `LiteLEDpioGroup` 2 lanes | configurable (PSRAM ok) | **always internal, shared** | **5,608 B** DMA + 384 B pixel (if internal) |
| `LiteLEDpioGroup` 8 lanes | configurable (PSRAM ok) | **always internal, shared** | **5,608 B** DMA + 1,536 B pixel (if internal) |

For very large arrays with `LiteLEDpio`, use `PSRAM_ENABLE` for the colour buffer to keep it out of internal SRAM, and ensure the DMA buffer fits within available internal heap before calling `begin()`.

---

<a name="optimization-tips"></a>
# Optimization Tips

1. **Use DMA for large arrays** (300+ LEDs on supported chips)
2. **Use PSRAM for very large arrays** (1000+ LEDs)
3. **Batch updates**: Set multiple pixels before calling `show()`
4. **Avoid frequent `show()` calls**: Maximum practical update rate is ~60 Hz
5. **Use `show` parameter**: Methods like `setPixel()` have optional `show` parameter for immediate update

**Good Practice**:

```cpp
// Efficient: Batch updates
for (size_t i = 0; i < LED_COUNT; i++) {
    strip.setPixel(i, color);
}
strip.show();  // Single update

// Inefficient: Update after each pixel
for (size_t i = 0; i < LED_COUNT; i++) {
    strip.setPixel(i, color, true);         // Don't do this!
}
```

---

<a name="troubleshooting"></a>
# Troubleshooting

<a name="no-leds-light-up"></a>
## No LEDs Light Up

1. **Check GPIO connection**: Verify DIN pin connection
2. **Check power supply**: LEDs need adequate power (5V, sufficient current)
3. **Check LED type**: Ensure `LED_STRIP_*****` matches your LED type
4. **Verify initialization**: Check `begin()` return value
5. **Call `show()`**: LED buffer must be transmitted with `show()`

<a name="wrong-colours"></a>
## Wrong Colours

1. **Check colour order**: Try different `setOrder()` values
2. **Check LED type**: Some "WS2812" clones use RGB instead of GRB
3. **Power supply issues**: Insufficient power can cause colour errors

<a name="flickering-or-glitches"></a>
## Flickering or Glitches

1. **Add 0.1µF capacitor** between ESP32 GND and LED strip GND
1. **Use shorter wires** for DIN connection (< 15cm recommended)
1. **Add 330Ω resistor** in series with DIN line
1. **Use a level shifter** between the GPIO pin and the DIN line if the strip power supply is greater than 3.3V.
1. **Check power supply quality**: Use stable supply

<a name="gpio-already-in-use-error"></a>
## GPIO Already in Use Error

1. **Check for duplicate instances** using same GPIO
2. **Free previous instance** before reusing GPIO
3. **Use `isGpioAvailable()`** to verify if GPIO is free

<a name="memory-allocation-failed"></a>
## Memory Allocation Failed

1. **Reduce LED count** if using internal RAM
2. **Enable PSRAM**: Use `PSRAM_AUTO` or `PSRAM_ENABLE`
3. **Check available heap**: Use `ESP.getFreeHeap()`

<a name="compile-time-assertions"></a>
## Compile-Time Assertions

The library performs the following compile-time checks and will halt compilation if any of the following conditions are found.:

* **ESP32 platform:** target is not an ESP32 microcontroller.
* **RMT peripheral:** target ESP32 does not have an RMT peripheral.
* **arduino-esp32 core version:** the version of the arduino-esp32 core is incompatible with this version of the library.

<a name="compile-time-warnings"></a>
## Compile-Time Warnings

When compiling for chips or core versions with limited capabilities, you'll see compile-time warnings:

- `"LiteLED: Selected ESP32 model does not support RMT DMA access. Use of RMT DMA will be disabled."`
- `"LiteLED: This version of the core does not support setting of RMT interrupt priority. Default will be used."`

These warnings inform you at build time of platform limitations, while the utility functions allow you to query these capabilities programmatically.

---

<a name="error-handling"></a>
## Error Handling

<a name="return-status-codes"></a>
### Return Status Codes

The LiteLED methods that write to the LED string buffer or set/reset the colour order return a status code of `esp_err_t ` type on completion. Checking this code and taking action is optional and is an exercise left to the developer.

If things go as normal, the return code is `ESP_OK` which is of type `int` with a value of `0`. So a quick check would be, if the return code is anything other than `0`, something went amok.

Full description of these codes can be found on the Espressif ESP-IDF site [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/error-codes.html?highlight=error%20handling).

If you're really interested in diving deeper, head over to the Espressif [ESP-IDF Error Handling](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/error-handling.html?highlight=error%20handling) docs.


| Error Code | Value | Description |
|------------|:-------:|-------------|
| `ESP_OK` | `0` | Success |
| `ESP_ERR_INVALID_ARG` | `0x102` | Invalid argument (e.g., invalid GPIO, out-of-bounds LED index) |
| `ESP_ERR_INVALID_STATE` | `0x103` | Invalid state (e.g., GPIO already in use) |
| `ESP_ERR_NO_MEM` | `0x101` | Memory allocation failed |
| `ESP_FAIL` | `-1` | Generic failure |


<a name="error-checking-pattern"></a>
### Error Checking Pattern

```cpp
esp_err_t err = strip.begin(14, 60);
if (err != ESP_OK) {
    Serial.printf("Failed to initialize LED strip: %s\n", esp_err_to_name(err));
    // Handle error
}
```

<a name="common-error-scenarios"></a>
### Common Error Scenarios

<a name="gpio-already-in-use"></a>
#### GPIO Already in Use

```cpp
// First strip uses GPIO 14
LiteLED strip1(LED_STRIP_WS2812, false);
strip1.begin(14, 30);  // OK

// Attempting to use same GPIO fails
LiteLED strip2(LED_STRIP_WS2812, false);
esp_err_t err = strip2.begin(14, 60);  // Returns ESP_ERR_INVALID_STATE
```

**Solution**:

Use different GPIO pins for each strip, or free the first strip before reusing the GPIO.

---

<a name="out-of-bounds-led-index"></a>
#### Out of Bounds LED Index

```cpp
LiteLED strip(LED_STRIP_WS2812, false);
strip.begin(14, 30);  // 30 LEDs (indices 0-29)

esp_err_t err = strip.setPixel(30, 0xFF0000);  // Returns ESP_ERR_INVALID_ARG
```

**Solution**:

Ensure LED indices are within valid range (0 to length-1).

---

<a name="memory-allocation-failure"></a>
#### Memory Allocation Failure

```cpp
// Very large LED array might fail
LiteLED hugeStrip(LED_STRIP_WS2812, false);
esp_err_t err = hugeStrip.begin(14, 10000, PSRAM_DISABLE);  // May return ESP_ERR_NO_MEM
```

**Solution**:

Use PSRAM for large arrays, reduce LED count, or free unused memory.

---

<a name="logging"></a>
## Logging

To assist in debugging, a number of status and error messages can be sent to the serial port via the esp32 `log_'x'` facility.

All log messages from the library are sent at the  `log_d` level except a very nerdy and long LiteLED Debug Report which is sent at the `log_v` level.

To see these messages, in the Arduino IDE, set the *Core Debug Level* from the *Tools* menu to either *Debug* or *Verbose*

If using PlatformIO or pioarduino, add `-DCORE_DEBUG_LEVEL=<level>` to the `build_flags` section the `platformio.ini` file setting where `<level>` is either `4` (debug) or  `5` (verbose).

If something is not behaving as you think, or you're just curious, set the log level to Debug or Verbose, recompile and upload the sketch and review the messages.

---

<a name="usage-examples"></a>
# Usage Examples

<a name="basic-example-solid-colour"></a>
## Basic Example - Solid Colour

```cpp
#include <LiteLED.h>

#define LED_GPIO    14
#define LED_COUNT   30

LiteLED strip(LED_STRIP_WS2812, false);

void setup() {
    Serial.begin(115200);
    
    if (strip.begin(LED_GPIO, LED_COUNT) == ESP_OK) {
        Serial.println("LED strip initialized");
        
        // Set all LEDs to blue at 50% brightness
        strip.brightness(128);
        strip.fill(0x0000FF, true);
    } else {
        Serial.println("Failed to initialize LED strip");
    }
}

void loop() {
    // Nothing to do
}
```

---

<a name="brightness-ramping"></a>
## Brightness Ramping

Smooth brightness transitions:

```cpp
void rampBrightness(LiteLED &strip, uint8_t target, uint8_t step = 1) {
    uint8_t current = strip.getBrightness();
    
    if (current < target) {
        for (uint8_t b = current; b <= target; b += step) {
            strip.brightness(b, true);
            delay(10);
        }
    } else {
        for (uint8_t b = current; b >= target; b -= step) {
            strip.brightness(b, true);
            delay(10);
        }
    }
}

// Usage
rampBrightness(strip, 255, 5);  // Fade up to full brightness
delay(2000);
rampBrightness(strip, 0, 5);    // Fade down to off
```

---

<a name="rainbow-pattern"></a>
## Rainbow Pattern

```cpp
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
```

---

<a name="multiple-strips"></a>
## Multiple Strips

```cpp
#include <LiteLED.h>

#define STRIP1_GPIO  14
#define STRIP2_GPIO  27
#define LED_COUNT    30

LiteLED strip1(LED_STRIP_WS2812, false);
LiteLED strip2(LED_STRIP_WS2812, false);

void setup() {
    Serial.begin(115200);
    
    // Initialize first strip
    if (strip1.begin(STRIP1_GPIO, LED_COUNT) == ESP_OK) {
        Serial.println("Strip 1 initialized");
        strip1.fill(0xFF0000, true);  // Red
    }
    
    // Initialize second strip
    if (strip2.begin(STRIP2_GPIO, LED_COUNT) == ESP_OK) {
        Serial.println("Strip 2 initialized");
        strip2.fill(0x0000FF, true);  // Blue
    }
    
    Serial.printf("Active instances: %d\n", LiteLED::getActiveInstanceCount());
}

void loop() {
    // Alternate colours
    delay(1000);
    strip1.fill(0x00FF00, true);  // Green
    strip2.fill(0xFF00FF, true);  // Magenta
    
    delay(1000);
    strip1.fill(0xFF0000, true);  // Red
    strip2.fill(0x0000FF, true);  // Blue
}
```

---

<a name="rgbw-strip-with-auto-white"></a>
## RGBW Strip with Auto White

```cpp
#include <LiteLED.h>

#define LED_GPIO    14
#define LED_COUNT   30

LiteLED rgbwStrip(LED_STRIP_SK6812, true);  // RGBW mode

void setup() {
    // Enable automatic white channel calculation
    rgbwStrip.begin(LED_GPIO, LED_COUNT, true);
    rgbwStrip.brightness(100);
}

void loop() {
    // Set pure white - auto_w will calculate W channel
    rgbwStrip.fill(rgb_from_values(255, 255, 255), true);
    delay(1000);
    
    // Set colour (no white channel)
    rgbwStrip.fill(rgb_from_values(255, 0, 0), true);
    delay(1000);
}
```

---

<a name="large-array-with-psram"></a>
## Large Array with PSRAM

```cpp
#include <LiteLED.h>

#define LED_GPIO    14
#define LED_COUNT   1000  // Large array

LiteLED bigStrip(LED_STRIP_WS2812, false);

void setup() {
    Serial.begin(115200);
    
    // Use PSRAM for large buffer
    esp_err_t err = bigStrip.begin(LED_GPIO, LED_COUNT, PSRAM_AUTO);
    
    if (err == ESP_OK) {
        Serial.println("Large strip initialized successfully");
        Serial.printf("Using %d LEDs\n", LED_COUNT);
        
        // Create gradient effect
        for (size_t i = 0; i < LED_COUNT; i++) {
            uint8_t brightness = (i * 255) / LED_COUNT;
            bigStrip.setPixel(i, rgb_from_values(brightness, 0, 255 - brightness));
        }
        bigStrip.show();
    } else {
        Serial.printf("Failed to initialize: %s\n", esp_err_to_name(err));
    }
}

void loop() {
    // Nothing to do
}
```

---

<a name="high-performance-with-dma"></a>
## High-Performance with DMA

```cpp
#include <LiteLED.h>

#define LED_GPIO    14
#define LED_COUNT   300

LiteLED perfStrip(LED_STRIP_WS2812, false);

void setup() {
    Serial.begin(115200);
    
    // Use DMA and high priority for best performance
    esp_err_t err = perfStrip.begin(LED_GPIO, LED_COUNT, DMA_ON, 
                                    PRIORITY_HIGH, PSRAM_AUTO);
    
    if (err == ESP_OK) {
        Serial.println("High-performance strip initialized");
    } else {
        Serial.printf("Init failed: %s\n", esp_err_to_name(err));
    }
}

void loop() {
    // Fast animation updates
    static uint8_t offset = 0;
    
    for (size_t i = 0; i < LED_COUNT; i++) {
        uint8_t pos = (i + offset) & 0xFF;
        perfStrip.setPixel(i, rgb_from_values(pos, 0, 255 - pos));
    }
    
    perfStrip.show();
    offset += 2;
    delay(10);
}
```

---

<a name="version-history"></a>
# Version History

**v3.1.0**
- Added support for using the ESP32 PARLIO peripheral as the driver.
- Added `LiteLEDpio` class: 
    - PARLIO-backed single-strip LED driver for SoCs with a PARLIO peripheral
- Added `LiteLEDpioGroup` and `LiteLEDpioLane` classes:
    - drive up to 8 independent strips simultaneously from one PARLIO TX unit; 
    - single `show()` transmits all lanes in lock-step via one GDMA transfer
- API-identical to `LiteLED`; 
    - for single-strip LEDs, switch between RMT and PARLIO driver with one type declaration
- Added `multi_strip_parlio` example sketch


**v3.0.1**
- Bug fixes.

**v3.0.0**
- Initial release for LiteLED library version 3.0.0.


<!-- EOF -->
