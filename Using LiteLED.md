# Using LiteLED

## Table of Contents

- [Quick Start](#quick-start)
    * [Basic Example](#basic-example)
- [Colour Representation](#colour-representation)
    * [Regarding RGBW Strips](#regarding-rgbw-strips)
- [Driver Architecture](#driver-architecture)
    * [LiteLED — RMT Driver](#liteled-rmt-driver)
    * [LiteLEDpio — PARLIO Single-Strip Driver](#liteledpio-parlio-single-strip-driver)
    * [LiteLEDpioGroup / LiteLEDpioLane — PARLIO Multi-Strip Driver](#liteledpiogroup--liteledpiolane--parlio-multi-strip-driver)
    * [Driver Comparison](#driver-comparison)
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
    * [Constructor](#constructor)
    * [Destructor](#destructor)
    * [Initialization — `begin()`](#initialization-methods)
        + [`begin()` — LiteLED (RMT)](#begin-liteled)
        + [`begin()` — LiteLEDpio (PARLIO)](#begin-liteledpio)
        + [`begin()` — LiteLEDpioGroup (PARLIO)](#begin-liteledpiogroup)
    * [Strip Registration — `addStrip()` (LiteLEDpioGroup)](#strip-registration-addstrip)
        + [Sequential lane assignment](#sequential-lane-assignment)
        + [Explicit lane assignment](#explicit-lane-assignment)
    * [Display Control Methods](#display-control-methods)
        + [`show()`](#show)
        + [`clear()`](#clear)
    * [Brightness Methods](#brightness-methods)
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
        + [`isValid()`](#isvalid)
        + [`getGpioPin()`](#getgpiopin)
        + [`isGpioAvailable()` — Static](#isgpioavailable-static)
        + [`getActiveInstanceCount()` — Static](#getactiveinstancecount-static)
        + [`operator[](lane)` — LiteLEDpioGroup](#operatorlane)
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

<a name="driver-architecture"></a>
# Driver Architecture

LiteLED provides three driver classes that share a common pixel-manipulation API. Choose the driver that matches your hardware and concurrency requirements.

---

<a name="liteled-rmt-driver"></a>
## LiteLED — RMT Driver

### How It Works

`LiteLED` drives LED strips through the ESP32 **RMT** (Remote Control Transceiver) peripheral. When `show()` is called, an ESP-IDF RMT encoder callback runs — converting the pixel colour buffer to precise timing waveforms on-the-fly — and loads them into RMT symbol memory or (where available) a DMA buffer. The RMT peripheral then autonomously clocks out the waveform with nanosecond timing accuracy.

Because encoding is performed inside a callback, there is **no pre-allocated bitstream buffer** and the RAM cost per strip is just the pixel colour buffer (3 or 4 bytes per LED). Multiple instances can run concurrently, each on its own RMT channel, with independently configurable DMA and interrupt priority.

### Availability and Limits

- **SoC support:** All ESP32 variants with an RMT peripheral (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, ESP32-H2, …)
- **Maximum instances:** Up to 8 concurrent strips (one RMT TX channel each); the exact limit depends on the SoC
- **DMA:** Optional — available on ESP32-S2, S3, C3, H2; not available on original ESP32 or C6
- **Interrupt priority:** Configurable on arduino-esp32 core v3.2.0+

### When to Choose LiteLED

- Your target SoC does not have PARLIO (`SOC_PARLIO_SUPPORTED` is absent)
- You need more than one independent strip and your SoC has only one PARLIO TX unit (e.g., ESP32-C6)
- You want to mix RMT strips with PARLIO strips in the same application
- You prefer the smallest possible pre-allocated heap footprint per strip

### Pros and Cons

| | |
|---|---|
| ✅ | Available on all ESP32 SoCs with RMT |
| ✅ | Up to 8 concurrent independent strips |
| ✅ | Minimal heap: only the pixel colour buffer (3–4 B/LED) |
| ✅ | Optional DMA and configurable interrupt priority |
| ⚠️ | Each strip consumes one RMT TX channel |
| ⚠️ | Interrupt-driven; priority conflicts possible with multiple strips at high update rates |
| ⚠️ | RMT DMA not available on all SoC variants |

### Class Interface

```cpp
class LiteLED {
public:
    LiteLED(led_strip_type_t led_type, bool rgbw);
    ~LiteLED();

    esp_err_t begin(uint8_t data_pin, size_t length, bool auto_w = true);
    esp_err_t begin(uint8_t data_pin, size_t length, ll_psram_t psram_flag, bool auto_w = true);
    esp_err_t begin(uint8_t data_pin, size_t length, ll_dma_t dma_flag,
                    ll_priority_t priority, ll_psram_t psram_flag, bool auto_w = true);

    esp_err_t show();
    esp_err_t clear(bool show = false);
    esp_err_t brightness(uint8_t bright, bool show = false);
    uint8_t   getBrightness();

    esp_err_t setPixel(size_t num, rgb_t  color, bool show = false);
    esp_err_t setPixel(size_t num, crgb_t color, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, rgb_t  *data, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, crgb_t *data, bool show = false);
    esp_err_t fill(rgb_t  color, bool show = false);
    esp_err_t fill(crgb_t color, bool show = false);
    esp_err_t fillRandom(bool show = false);

    rgb_t   getPixel(size_t num);
    crgb_t  getPixelC(size_t num);

    esp_err_t setOrder(color_order_t led_order = ORDER_GRB);
    esp_err_t resetOrder();

    bool      isValid() const;
    int       getGpioPin() const;
    static bool    isGpioAvailable(uint8_t gpio_pin);
    static uint8_t getActiveInstanceCount();
};
```

If you're curious about how the library is structured internally, see `LiteLED Architecture.md` in the `docs` folder of the [library repository](https://github.com/Xylopyrographer/LiteLED).

---

<a name="liteledpio-parlio-single-strip-driver"></a>
## LiteLEDpio — PARLIO Single-Strip Driver

### How It Works

`LiteLEDpio` drives LED strips through the ESP32 **PARLIO** (Parallel IO) TX peripheral with **GDMA** (General DMA). When `show()` is called, the entire LED waveform is **pre-encoded** into a DMA bitstream buffer — each input byte expands to 24 DMA bytes (8 bits × 3 sample bytes per bit), with a reset tail appended — and a single GDMA transfer streams that buffer to the PARLIO TX unit. The CPU is not involved once transmission starts; there is no interrupt service routine.

The API is deliberately identical to `LiteLED`. Switching between the RMT and PARLIO driver requires changing only the class name in the type declaration.

### Availability and Limits

- **SoC support:** Only on SoCs where `SOC_PARLIO_SUPPORTED` is defined (ESP32-C6, ESP32-H2, ESP32-P4, and later parts). The class is conditionally compiled out on unsupported targets.
- **Maximum instances:** 1 active `LiteLEDpio` (or `LiteLEDpioGroup`) per PARLIO TX unit. On ESP32-C6 and ESP32-H2 there is one PARLIO TX unit, so only one PARLIO instance can be active at a time. Combine with `LiteLED` RMT instances for additional independent strips.
- **DMA control:** PARLIO always uses GDMA — there is no DMA on/off flag.
- **Interrupt priority:** No ISR during transmission — no priority parameter.
- **RGBW support:** Identical to `LiteLED`: pass `true` for the `rgbw` constructor argument and use an RGBW-capable `led_strip_type_t`.

### When to Choose LiteLEDpio

- You need a single-strip PARLIO driver with the same API as `LiteLED`
- You want GDMA-driven transmission with zero CPU overhead during frame output
- You are migrating an existing `LiteLED` application to a PARLIO-capable SoC and want a one-line code change
- You are driving one strip and want the simplest possible setup

### Pros and Cons

| | |
|---|---|
| ✅ | GDMA transfer — zero CPU overhead during transmission |
| ✅ | No ISR — no interrupt priority configuration or conflicts |
| ✅ | API-identical to `LiteLED`; one-line driver swap |
| ✅ | PSRAM support for the pixel colour buffer |
| ⚠️ | `SOC_PARLIO_SUPPORTED` targets only |
| ⚠️ | One active instance per PARLIO TX unit |
| ⚠️ | Large pre-encoded DMA bitstream buffer (~72 B/LED for RGB, always in internal RAM) |

### Class Interface

```cpp
class LiteLEDpio {
public:
    LiteLEDpio(led_strip_type_t led_type, bool rgbw);
    ~LiteLEDpio();

    // No DMA flag or interrupt priority (PARLIO always uses GDMA; no ISR)
    esp_err_t begin(uint8_t data_pin, size_t length, bool auto_w = true);
    esp_err_t begin(uint8_t data_pin, size_t length, ll_psram_t psram_flag, bool auto_w = true);

    esp_err_t show();
    esp_err_t clear(bool show = false);
    esp_err_t brightness(uint8_t bright, bool show = false);
    uint8_t   getBrightness();

    esp_err_t setPixel(size_t num, rgb_t  color, bool show = false);
    esp_err_t setPixel(size_t num, crgb_t color, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, rgb_t  *data, bool show = false);
    esp_err_t setPixels(size_t start, size_t len, crgb_t *data, bool show = false);
    esp_err_t fill(rgb_t  color, bool show = false);
    esp_err_t fill(crgb_t color, bool show = false);
    esp_err_t fillRandom(bool show = false);

    rgb_t   getPixel(size_t num);
    crgb_t  getPixelC(size_t num);

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
LiteLED    myStrip(LED_STRIP_WS2812, false);  // RMT driver
LiteLEDpio myStrip(LED_STRIP_WS2812, false);  // PARLIO driver — identical API
```

---

<a name="liteledpiogroup--liteledpiolane--parlio-multi-strip-driver"></a>
## LiteLEDpioGroup / LiteLEDpioLane — PARLIO Multi-Strip Driver

### How It Works

`LiteLEDpioGroup` drives up to 8 independent LED strips from a **single PARLIO TX unit** by multiplexing each strip onto one bit-lane of the PARLIO data bus. Each strip's colour data is stored in a private per-lane pixel buffer, accessible via a `LiteLEDpioLane` handle returned by `addStrip()`.

When `show()` is called, LiteLED encodes all lanes' pixel buffers into a **single shared DMA bitstream buffer** — each lane's signal occupies one bit position in every DMA byte — then issues one GDMA transfer that updates all strips simultaneously.

The critical consequence: **every `show()` transmits all lanes.** There is no per-lane-only transmit. This guarantees frame-perfect synchronisation across all connected displays with minimal RAM overhead.

`LiteLEDpioLane` is a thin handle for one lane within a group. It is owned and managed by the group; user code obtains a reference from `addStrip()` or `operator[]`. Calling `show()` on a lane is identical to calling `show()` on the parent group — all lanes always transmit together.

### Availability and Limits

- **SoC support:** `SOC_PARLIO_SUPPORTED` only (same as `LiteLEDpio`)
- **Maximum lanes:** `PARLIO_TX_UNIT_MAX_DATA_WIDTH` — 8 on ESP32-C6 / ESP32-H2, 16 on ESP32-P4
- **Shared constraints:** All strips in a group must share the same `led_strip_type_t`, strip length, and RGBW flag
- **One active group:** Only one `LiteLEDpioGroup` (or `LiteLEDpio`) can be active at a time on C6/H2
- **DMA buffer:** One shared buffer regardless of lane count; size depends on strip length only

### When to Choose LiteLEDpioGroup

- You need to drive 2–8 independent LED strips simultaneously from one PARLIO TX unit
- All your strips share the same LED type and length
- You want frame-perfect lock-step synchronisation across all displays at zero per-strip CPU cost
- You want to minimise internal DMA RAM (one shared DMA buffer vs. N separate buffers)

### Pros and Cons

| | |
|---|---|
| ✅ | Up to 8 independent strips from one PARLIO TX unit |
| ✅ | Single shared DMA buffer — size does not grow with lane count |
| ✅ | Frame-perfect lock-step update across all strips |
| ✅ | Zero per-strip CPU overhead after setup |
| ✅ | Per-lane pixel colour buffers may reside in PSRAM |
| ⚠️ | `SOC_PARLIO_SUPPORTED` targets only |
| ⚠️ | All strips must share the same LED type, length, and RGBW flag |
| ⚠️ | No per-lane-only show — every `show()` transmits all lanes |
| ⚠️ | One active group per PARLIO TX unit |

### Class Interfaces

```cpp
class LiteLEDpioGroup {
public:
    LiteLEDpioGroup(led_strip_type_t led_type, size_t length, bool rgbw);
    ~LiteLEDpioGroup();

    LiteLEDpioLane &addStrip(uint8_t gpio);          // sequential lane assignment
    template<uint8_t LANE>
    LiteLEDpioLane &addStrip(uint8_t gpio);          // explicit lane assignment

    esp_err_t begin(ll_psram_t psram_flag = PSRAM_DISABLE);
    esp_err_t show();
    esp_err_t brightness(uint8_t bright, bool show = false);
    uint8_t   getBrightness();
    LiteLEDpioLane &operator[](uint8_t lane);
    bool isValid() const;
};

class LiteLEDpioLane {
public:
    // show() always transmits all lanes in the parent group
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

<a name="driver-comparison"></a>
## Driver Comparison

| Feature | `LiteLED` (RMT) | `LiteLEDpio` (PARLIO) | `LiteLEDpioGroup` (PARLIO) |
|---|---|---|---|
| SoC availability | All ESP32 with RMT | `SOC_PARLIO_SUPPORTED` | `SOC_PARLIO_SUPPORTED` |
| Max concurrent strips | Up to 8 (one per RMT channel) | 1 per PARLIO TX unit | Up to 8 lanes per PARLIO TX unit |
| Strips share type / length? | No — each independent | No — single strip | Yes — all lanes must match |
| Encoding method | On-the-fly in RMT callback | Pre-encoded to DMA bitstream | Pre-encoded, all lanes merged |
| DMA buffer | None (optional RMT DMA) | Per-instance (~72 B/LED RGB) | One shared buffer (~72 B/LED RGB, any lane count) |
| CPU overhead during transmit | Interrupt callback | None (GDMA) | None (GDMA) |
| Interrupt priority config | Yes (core v3.2.0+) | Not applicable | Not applicable |
| DMA enable/disable | `DMA_ON` / `DMA_OFF` | Always GDMA, no flag | Always GDMA, no flag |
| Per-strip independent `show()` | Yes | Yes | No — all lanes always together |
| PSRAM for pixel buffer | Yes | Yes | Yes (per-lane) |
| PSRAM for DMA buffer | No | No | No |
| RGBW support | Yes | Yes | Yes |
| API compatibility | — | Identical to `LiteLED` | `LiteLEDpioLane` API identical to `LiteLED` |

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

This section documents every method in all three driver classes. Where a method exists in more than one class, all forms and their behavioural differences are listed together. An **Applies to** note at the start of each entry shows which classes provide the method.

---

<a name="constructor"></a>
## Constructor

Creates a new driver instance. No hardware is allocated until `begin()` is called. Default brightness is 255 (full).

**LiteLED (RMT) and LiteLEDpio (PARLIO single-strip)**

Both classes take identical constructor parameters.

```cpp
LiteLED   (led_strip_type_t led_type, bool rgbw);
LiteLEDpio(led_strip_type_t led_type, bool rgbw);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `led_type` | `led_strip_type_t` | LED strip protocol (see [`led_strip_type_t`](#led_strip_type_t)) |
| `rgbw` | `bool` | `true` for RGBW strips (e.g., SK6812 RGBW); `false` for RGB strips |

**LiteLEDpioGroup (PARLIO multi-strip)**

The group constructor also takes the per-lane strip length because all lanes share the same length.

```cpp
LiteLEDpioGroup(led_strip_type_t led_type, size_t length, bool rgbw);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `led_type` | `led_strip_type_t` | LED strip protocol — same for all lanes |
| `length` | `size_t` | Number of LEDs per strip — same for all lanes |
| `rgbw` | `bool` | `true` for RGBW strips; `false` for RGB |

**Example**:

```cpp
// RMT driver — RGB WS2812
LiteLED strip(LED_STRIP_WS2812, false);

// PARLIO single-strip — same parameters, identical API
LiteLEDpio strip(LED_STRIP_WS2812, false);

// PARLIO multi-strip — strip length included in constructor
LiteLEDpioGroup panels(LED_STRIP_WS2812, 64, false);
```

---

<a name="destructor"></a>
## Destructor

Releases all hardware resources and heap allocations. Safe to call even if `begin()` was never called or failed partially.

**LiteLED**

```cpp
~LiteLED();
```

Frees the LED colour buffer (RAM or PSRAM), releases the RMT channel and encoder, and unregisters the GPIO from the Peripheral Manager and the LiteLED registry.

**LiteLEDpio**

```cpp
~LiteLEDpio();
```

Waits for any in-progress GDMA transfer to complete, then disables and deletes the PARLIO TX unit, frees the pixel colour buffer and the DMA bitstream buffer, and unregisters the GPIO from the Peripheral Manager.

**LiteLEDpioGroup**

```cpp
~LiteLEDpioGroup();
```

Waits for any in-progress GDMA transfer, disables and deletes the PARLIO TX unit, frees all per-lane pixel colour buffers and the shared DMA bitstream buffer, and unregisters all lane GPIOs from the Peripheral Manager.

---

<a name="initialization-methods"></a>
## Initialization — `begin()`

`begin()` allocates hardware resources, allocates heap buffers, and registers GPIOs. It must be called before any pixel operations or `show()`.

All `begin()` overloads return `ESP_OK` on success, or an `esp_err_t` error code on failure.

<a name="begin-liteled"></a>
### `begin()` — LiteLED (RMT)

**Applies to:** `LiteLED`

Three overloads are available.

#### Basic

```cpp
esp_err_t begin(uint8_t data_pin, size_t length, bool auto_w = true);
```

Allocates the pixel colour buffer in internal RAM and initialises the RMT channel with default settings (no DMA, default interrupt priority).

#### With PSRAM control

```cpp
esp_err_t begin(uint8_t data_pin, size_t length, ll_psram_t psram_flag, bool auto_w = true);
```

Allocates the pixel colour buffer in internal RAM or PSRAM according to `psram_flag`. Useful for large LED arrays (hundreds to thousands of LEDs).

#### Full configuration

```cpp
esp_err_t begin(uint8_t data_pin, size_t length, ll_dma_t dma_flag,
                ll_priority_t priority, ll_psram_t psram_flag, bool auto_w = true);
```

Full control over RMT DMA, interrupt priority, and pixel buffer placement.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `data_pin` | `uint8_t` | — | GPIO pin connected to the strip DIN |
| `length` | `size_t` | — | Number of LEDs in the strip |
| `dma_flag` | `ll_dma_t` | `DMA_DEFAULT` | RMT DMA mode (see [`ll_dma_t`](#ll_dma_t)) |
| `priority` | `ll_priority_t` | `PRIORITY_DEFAULT` | RMT interrupt priority (see [`ll_priority_t`](#ll_priority_t)) |
| `psram_flag` | `ll_psram_t` | `PSRAM_DISABLE` | Pixel colour buffer placement (see [`ll_psram_t`](#ll_psram_t)) |
| `auto_w` | `bool` | `true` | RGBW strips only: `true` derives the W channel automatically from R/G/B values; `false` leaves W at 0 |

**Notes:**

- DMA availability and interrupt priority support vary by SoC and arduino-esp32 core version; see [Compile-Time Warnings](#compile-time-warnings)
- Use `DMA_ON` only after verifying `LiteLED_Utils::isDmaSupported()`; the library will fall back gracefully but will log a warning

**Example:**

```cpp
LiteLED strip(LED_STRIP_WS2812, false);

// Basic
strip.begin(14, 60);

// PSRAM for large array
strip.begin(14, 1000, PSRAM_AUTO);

// Full config — DMA + high priority + PSRAM
strip.begin(14, 500, DMA_ON, PRIORITY_HIGH, PSRAM_AUTO);
```

---

<a name="begin-liteledpio"></a>
### `begin()` — LiteLEDpio (PARLIO)

**Applies to:** `LiteLEDpio`

Two overloads are available. There is no DMA flag or interrupt priority parameter — PARLIO always uses GDMA and has no ISR.

#### Basic

```cpp
esp_err_t begin(uint8_t data_pin, size_t length, bool auto_w = true);
```

Allocates the pixel colour buffer in internal RAM, allocates the DMA bitstream buffer in internal DMA-capable RAM, creates and enables the PARLIO TX unit.

#### With PSRAM control

```cpp
esp_err_t begin(uint8_t data_pin, size_t length, ll_psram_t psram_flag, bool auto_w = true);
```

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `data_pin` | `uint8_t` | — | GPIO pin connected to the strip DIN |
| `length` | `size_t` | — | Number of LEDs in the strip |
| `psram_flag` | `ll_psram_t` | `PSRAM_DISABLE` | PSRAM preference for the **pixel colour buffer** only |
| `auto_w` | `bool` | `true` | RGBW strips only: automatically derive W channel from R/G/B |

**PSRAM and DMA buffer allocation:**

The `psram_flag` controls the **pixel colour buffer** only. The **DMA bitstream buffer** is always allocated from internal DMA-capable RAM (`MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL`) — GDMA on C6/H2 cannot read PSRAM. The DMA buffer is approximately 72 bytes per LED (24 DMA bytes × 3 colour channels) plus a 1000-byte reset tail; for 64 LEDs this is ~5.6 KB.

**Example:**

```cpp
LiteLEDpio strip(LED_STRIP_WS2812, false);

// Basic
strip.begin(21, 64);

// PSRAM for large pixel buffer
strip.begin(21, 300, PSRAM_AUTO);
```

---

<a name="begin-liteledpiogroup"></a>
### `begin()` — LiteLEDpioGroup (PARLIO)

**Applies to:** `LiteLEDpioGroup`

One overload. GPIO pins are supplied per-lane via `addStrip()` **before** calling `begin()`.

```cpp
esp_err_t begin(ll_psram_t psram_flag = PSRAM_DISABLE);
```

Allocates per-lane pixel colour buffers, allocates the shared DMA bitstream buffer in internal DMA-capable RAM, creates and enables the PARLIO TX unit, and registers all lane GPIOs with the Peripheral Manager.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `psram_flag` | `ll_psram_t` | `PSRAM_DISABLE` | PSRAM preference for **all** per-lane pixel colour buffers. The shared DMA buffer is always internal RAM. |

Returns `ESP_ERR_INVALID_STATE` (with a log message) if called before any `addStrip()` has been registered.

**Example:**

```cpp
LiteLEDpioGroup panels(LED_STRIP_WS2812, 64, false);
panels.addStrip(21);   // lane 0 → GPIO 21
panels.addStrip(19);   // lane 1 → GPIO 19
panels.begin();        // allocate & enable

// Large arrays — pixel buffers in PSRAM
panels.begin(PSRAM_ENABLE);
```

---

<a name="strip-registration-addstrip"></a>
## Strip Registration — `addStrip()` (LiteLEDpioGroup)

**Applies to:** `LiteLEDpioGroup`

Registers a strip (GPIO pin → lane mapping) before `begin()` is called. Returns a `LiteLEDpioLane` reference for per-strip pixel manipulation. The two forms differ only in how the lane index (bit-lane position in the shared DMA buffer) is assigned.

<a name="sequential-lane-assignment"></a>
### Sequential lane assignment

```cpp
LiteLEDpioLane &addStrip(uint8_t gpio);
```

Auto-assigns the next available lane (0, 1, 2 …). This is the typical form when all strips are registered in a fixed order and no lane gaps are needed.

Returns a silent null lane (with an error log) if all lanes are already assigned.

<a name="explicit-lane-assignment"></a>
### Explicit lane assignment

```cpp
template<uint8_t LANE>
LiteLEDpioLane &addStrip(uint8_t gpio);
```

Assigns the strip to `LANE` specifically. `LANE` is verified against `PARLIO_TX_UNIT_MAX_DATA_WIDTH` at compile time via `static_assert`.

Both forms give equal freedom to choose any GPIO pin for the strip. The explicit form additionally guarantees:

- **Sparse assignment** — deliberate gaps in lane numbering for future expansion or reserved index positions
- **External index storage** — code that records lane numbers and retrieves strips via `operator[]` later gets a guaranteed, init-order-independent index
- **Documentation of intent** — making the lane-to-strip mapping explicit in the source

```cpp
LiteLEDpioGroup strips(LED_STRIP_WS2812, 64, false);

// Sequential — lanes assigned 0, 1 in order
LiteLEDpioLane &left  = strips.addStrip(21);
LiteLEDpioLane &right = strips.addStrip(19);

// Explicit — lanes 0 and 3; lanes 1 and 2 left empty for future use
LiteLEDpioLane &panelA = strips.addStrip<0>(21);
LiteLEDpioLane &panelB = strips.addStrip<3>(19);
```

---

<a name="display-control-methods"></a>
## Display Control Methods

<a name="show"></a>
### `show()`

**Applies to:** `LiteLED` · `LiteLEDpio` · `LiteLEDpioGroup` · `LiteLEDpioLane`

```cpp
esp_err_t show();
```

Transmits the pixel buffer to the physical LED strip. This is a blocking call that waits until transmission (including the reset/latch period) is complete.

**Returns:**

- `ESP_OK` — success
- `LiteLED` / `LiteLEDpio`: error codes from the RMT / PARLIO transmit operation

**Behavioural differences by driver:**

| Driver | What `show()` does |
|--------|--------------------|
| `LiteLED` | Runs the RMT encoder callback; clocks out the waveform via RMT |
| `LiteLEDpio` | Pre-encodes the pixel buffer into the DMA bitstream buffer, then issues a GDMA transfer to the PARLIO TX unit |
| `LiteLEDpioGroup` | Pre-encodes **all** lane pixel buffers into the shared DMA bitstream buffer (OR-merging each lane's bits), then issues one GDMA transfer — all strips update simultaneously |
| `LiteLEDpioLane` | Identical to calling `show()` on the parent group — all lanes always transmit together |

**Example:**

```cpp
strip.setPixel(0, 0xFF0000);
strip.setPixel(1, 0x00FF00);
strip.show();   // update the strip
```

---

<a name="clear"></a>
### `clear()`

**Applies to:** `LiteLED` · `LiteLEDpio` · `LiteLEDpioLane`

> **Note:** `LiteLEDpioGroup` does not have a `clear()` method directly; call `clear()` on each `LiteLEDpioLane` reference individually.

```cpp
esp_err_t clear(bool show = false);
```

Sets all pixels in the buffer to black (off).

**Parameters:**

- `show` — if `true`, calls `show()` automatically after clearing

**Returns:** `ESP_OK` on success

**Example:**

```cpp
strip.clear();        // clear buffer, don't update strip yet
strip.clear(true);    // clear buffer and update strip immediately

// LiteLEDpioGroup — clear each lane, then transmit once
panelA.clear();
panelB.clear();
strips.show();
```

---

<a name="brightness-methods"></a>
## Brightness Methods

<a name="brightness"></a>
### `brightness()`

**Applies to:** `LiteLED` · `LiteLEDpio` · `LiteLEDpioGroup` · `LiteLEDpioLane`

```cpp
esp_err_t brightness(uint8_t bright, bool show = false);
```

Sets the global brightness level. Brightness is applied non-destructively during `show()` encoding using `scale8_video()`; pixel buffer values are not modified.

**Parameters:**

- `bright` — brightness level (0–255, where 0 = off, 255 = full)
- `show` — if `true`, calls `show()` automatically after updating the brightness

**Returns:** `ESP_OK` on success

**Behavioural differences:**

| Driver | Scope |
|--------|-------|
| `LiteLED` / `LiteLEDpio` | Applies to the single strip |
| `LiteLEDpioGroup` | Sets a group-wide brightness; individual lanes can independently override it via their `LiteLEDpioLane` reference |
| `LiteLEDpioLane` | Sets the brightness for that lane only, independently of both the group setting and other lanes |

**Example:**

```cpp
strip.brightness(128);          // 50% — update on next show()
strip.brightness(51, true);     // ~20% — update immediately

strips.brightness(20);          // group: all lanes at 20%
panelA.brightness(100);         // individual lane override
```

---

<a name="getbrightness"></a>
### `getBrightness()`

**Applies to:** `LiteLED` · `LiteLEDpio` · `LiteLEDpioGroup` · `LiteLEDpioLane`

```cpp
uint8_t getBrightness();
```

Returns the current brightness value (0–255).

**Notes:**

- Returns the brightness value used the last time `show()` was called. If `brightness()` has been set since the last `show()`, the return value reflects the pending value (what will be applied on the next `show()`).
- On `LiteLEDpioGroup`, returns the group-wide brightness setting. Each lane can have its own independent brightness set via its `LiteLEDpioLane` reference; `LiteLEDpioLane::getBrightness()` returns that lane's individual value.

**Returns:** Current brightness (0–255)

**Example:**

```cpp
uint8_t b = strip.getBrightness();
Serial.printf("Brightness: %d
", b);
```

---

<a name="pixel-manipulation-methods"></a>
## Pixel Manipulation Methods

**Applies to:** `LiteLED` · `LiteLEDpio` · `LiteLEDpioLane`

> `LiteLEDpioGroup` does not expose pixel methods directly — access pixels through the `LiteLEDpioLane` references returned by `addStrip()` or `operator[]`.

<a name="setpixel-rgb_t"></a>
### `setPixel()` - `rgb_t`

Set single LED colour using `rgb_t` structure.

```cpp
esp_err_t setPixel(size_t num, rgb_t color, bool show = false);
```

**Parameters:**

- `num` — LED index (0-based)
- `color` — colour as `rgb_t` structure
- `show` — if `true`, immediately update the strip

**Returns:**

- `ESP_OK` — success
- `ESP_ERR_INVALID_ARG` — LED index out of bounds or strip not initialized

**Example:**

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

Set the colour of a single LED using a 32-bit colour code.

```cpp
esp_err_t setPixel(size_t num, crgb_t color, bool show = false);
```

**Parameters:**

- `num` — LED index (0-based)
- `color` — colour as `crgb_t` 32-bit code (`0x00RRGGBB`)
- `show` — if `true`, immediately update the strip

**Returns:** Same as `rgb_t` version

**Example:**

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

**Parameters:**

- `start` — first LED index (0-based)
- `len` — number of LEDs to set
- `data` — pointer to array of `rgb_t` colours
- `show` — if `true`, immediately update the strip

**Returns:**

- `ESP_OK` — success
- `ESP_ERR_INVALID_ARG` — invalid parameters or out of bounds

**Example:**

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

**Parameters:**

- `start` — first LED index (0-based)
- `len` — number of LEDs to set
- `data` — pointer to array of 32-bit colour codes
- `show` — if `true`, immediately update the strip

**Returns:** Same as `rgb_t` version

**Example:**

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

**Parameters:**

- `color` — colour as `rgb_t` structure
- `show` — if `true`, immediately update the strip

**Returns:** `ESP_OK` on success

**Example:**

```cpp
strip.fill(rgb_from_values(0, 0, 255), true);   // blue, show immediately

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

**Parameters:**

- `color` — colour as 32-bit code (`0x00RRGGBB`)
- `show` — if `true`, immediately update the strip

**Returns:** `ESP_OK` on success

**Example:**

```cpp
strip.fill(0xFF0000, true);   // red, show immediately
```

---

<a name="fillrandom"></a>
### `fillRandom()`

Fill strip with random colours.

```cpp
esp_err_t fillRandom(bool show = false);
```

**Parameters:**

- `show` — if `true`, immediately update the strip

**Returns:** `ESP_OK` on success

**Description:**

Fills each LED with a random RGB colour using the ESP32's hardware random number generator. Each colour channel is set independently to a random value between 5 and 255, so the result can be quite bright. Use `brightness()` beforehand to control overall intensity.

**Example:**

```cpp
strip.fillRandom(true);

void loop() {
    strip.fillRandom(true);
    delay(1000);
}
```

---

<a name="pixel-reading-methods"></a>
## Pixel Reading Methods

**Applies to:** `LiteLED` · `LiteLEDpio` · `LiteLEDpioLane`

> Returns the colour value stored in the pixel buffer — not accounting for brightness scaling.

<a name="getpixel"></a>
### `getPixel()`

Get colour of a single LED as `rgb_t` structure.

```cpp
rgb_t getPixel(size_t num);
```

**Parameters:**

- `num` — LED index (0-based)

**Returns:** `rgb_t` structure with LED colour, or `{0, 0, 0}` if the index is invalid

**Example:**

```cpp
rgb_t color = strip.getPixel(5);
Serial.printf("LED 5: R=%d, G=%d, B=%d
", color.r, color.g, color.b);
```

---

<a name="getpixelc"></a>
### `getPixelC()`

Get colour of a single LED as a 32-bit colour code.

```cpp
crgb_t getPixelC(size_t num);
```

**Parameters:**

- `num` — LED index (0-based)

**Returns:** 32-bit colour code (`0x00RRGGBB`), or `0x000000` if the index is invalid

**Example:**

```cpp
crgb_t color = strip.getPixelC(10);
if (color == 0xFF0000) {
    Serial.println("LED 10 is red");
}
```

---

<a name="colour-order-methods"></a>
## Colour Order Methods

**Applies to:** `LiteLED` · `LiteLEDpio` · `LiteLEDpioLane`

<a name="setorder"></a>
### `setOrder()`

Set custom colour byte order for the LED strip.

```cpp
esp_err_t setOrder(color_order_t led_order = ORDER_GRB);
```

**Parameters:**

- `led_order` — colour order (see [`color_order_t`](#color_order_t))

**Returns:** `ESP_OK` on success

**Description:**

Overrides the default colour order for the LED strip type. Useful for LED strips that don't match standard specifications. Takes effect after the next `show()` call and remains in effect until another `setOrder()` or `resetOrder()` is called. Can be called any time after the object is declared.

**Example:**

```cpp
strip.setOrder(ORDER_RGB);   // override default GRB
strip.setOrder(ORDER_BGR);   // some non-standard strips need this
```

---

<a name="resetorder"></a>
### `resetOrder()`

Reset colour order to the driver default for the LED strip type.

```cpp
esp_err_t resetOrder();
```

**Returns:** `ESP_OK` on success

Takes effect after the next `show()` call and remains in effect until another `setOrder()` or `resetOrder()` is called. Can be called any time after the object is declared.

**Example:**

```cpp
strip.resetOrder();   // restore default colour order
```

---

<a name="instance-management-methods"></a>
## Instance Management Methods

<a name="isvalid"></a>
### `isValid()`

**Applies to:** `LiteLED` · `LiteLEDpio` · `LiteLEDpioGroup` · `LiteLEDpioLane`

```cpp
bool isValid() const;
```

**Returns:**

| Driver | Returns `true` when… |
|--------|-----------------------|
| `LiteLED` | GPIO is still registered to this instance (checks registry and Peripheral Manager) |
| `LiteLEDpio` | Same as `LiteLED` |
| `LiteLEDpioGroup` | The group was successfully initialised via `begin()` |
| `LiteLEDpioLane` | The lane was registered via `addStrip()` (not a null-lane sentinel) |

**Example:**

```cpp
if (!strip.isValid()) {
    Serial.println("Strip instance is no longer valid!");
}

// LiteLEDpioGroup
if (!panelA.isValid()) {
    // lane was never registered or group not yet initialised
}
```

---

<a name="getgpiopin"></a>
### `getGpioPin()`

**Applies to:** `LiteLED` · `LiteLEDpio`

```cpp
int getGpioPin() const;
```

**Returns:**

- GPIO pin number if successfully initialised
- `-1` if not initialised

**Example:**

```cpp
int pin = strip.getGpioPin();
if (pin >= 0) {
    Serial.printf("Strip using GPIO %d
", pin);
}
```

---

<a name="isgpioavailable-static"></a>
### `isGpioAvailable()` — Static

**Applies to:** `LiteLED` · `LiteLEDpio`

```cpp
static bool isGpioAvailable(uint8_t gpio_pin);
```

Queries the ESP32 Peripheral Manager to check whether the given GPIO is free.

**Parameters:**

- `gpio_pin` — GPIO pin number to check

**Returns:** `true` if the GPIO is available; `false` if already in use by another peripheral

**Example:**

```cpp
if (LiteLED::isGpioAvailable(14)) {
    strip.begin(14, 60);
} else {
    Serial.println("GPIO 14 is already in use");
}
```

---

<a name="getactiveinstancecount-static"></a>
### `getActiveInstanceCount()` — Static

**Applies to:** `LiteLED` · `LiteLEDpio`

```cpp
static uint8_t getActiveInstanceCount();
```

Returns the number of currently active instances for that driver class.

**Example:**

```cpp
Serial.printf("Active RMT strips: %d
", LiteLED::getActiveInstanceCount());
Serial.printf("Active PARLIO strips: %d
", LiteLEDpio::getActiveInstanceCount());
```

---

<a name="operatorlane"></a>
### `operator[](lane)` — LiteLEDpioGroup

**Applies to:** `LiteLEDpioGroup`

```cpp
LiteLEDpioLane &operator[](uint8_t lane);
```

Returns the `LiteLEDpioLane` reference for the given bit-lane index. Returns a silent null lane (with an error log) if the index is out of range or was never registered via `addStrip()`. Use `isValid()` on the returned reference to verify it.

**Parameters:**

- `lane` — bit-lane index (0-based)

**Example:**

```cpp
LiteLEDpioGroup panels(LED_STRIP_WS2812, 64, false);
panels.addStrip<0>(21);
panels.addStrip<1>(19);
panels.begin();

// Access via operator[]
panels[0].fill(0xFF0000);   // lane 0 red
panels[1].fill(0x0000FF);   // lane 1 blue
panels.show();

// Null-lane guard
LiteLEDpioLane &lane = panels[5];   // lane 5 was never registered
if (!lane.isValid()) {
    // handle gracefully
}
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
