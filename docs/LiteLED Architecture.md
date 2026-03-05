# LiteLED Architecture

## Overview

LiteLED starting with v3.0.0 uses a modular architecture to separate concerns and improve maintainability. The library consists of five specialized modules, each handling a specific aspect of the LED strip driver functionality.

---

## Module Hierarchy and Dependencies

```
LiteLED.h (Main Public API — RMT and PARLIO drivers)
    ├─> llrmt.h (RMT Compatibility Header)
    │   ├─> ll_led_timings.h (LED Timing Constants — RMT + PARLIO tables)
    │   ├─> ll_priority.h/.cpp (RMT Priority Management)
    │   ├─> ll_encoder.h/.cpp (RMT Encoder Callback)
    │   ├─> ll_strip_core.h/.cpp (RMT Core Operations)
    │   └─> ll_strip_pixels.h/.cpp (Shared Pixel Manipulation)
    ├─> llparlio.h (PARLIO Compatibility Header — SOC_PARLIO_SUPPORTED only)
    │   ├─> ll_led_timings.h (shared — PARLIO timing table)
    │   └─> ll_parlio_core.h/.cpp (PARLIO Core Operations — single-strip and group)
    ├─> LiteLEDpioGroup.cpp (LiteLEDpioGroup / LiteLEDpioLane — multi-strip PARLIO)
    ├─> ll_registry.h/.cpp (Minimal Instance Tracking)
    ├─> esp32-hal-periman.h (ESP32 Peripheral Manager - Direct GPIO Management)
    └─> llrgb.h (RGB Color Utilities)
```

---

## Core Files

### `LiteLED.h` / `LiteLED.cpp`

**Purpose:** Main public API and high-level class interface

**Responsibilities:**

- Defines the `led_strip_t` structure (LED strip object)
- Provides the user-facing C++ class interface
- Platform compatibility checks (ESP32, Arduino core version)
- Hardware capability detection (RMT, DMA, interrupt priority support)
- Public methods for LED strip control (begin, show, setPixel, fill, etc.)

**Key Dependencies:**

- `llrmt.h` - Low-level RMT operations
- `ll_registry.h` - Minimal instance tracking for cleanup callbacks
- `esp32-hal-periman.h` - ESP32 Peripheral Manager for GPIO conflict prevention
- `llrgb.h` - Color manipulation utilities

**Relationship:** This is the top-level interface that users interact with. It wraps all lower-level modules into an easy-to-use API.

---

### `llrmt.h` (RMT Compatibility Header)

**Purpose:** Single include point for all low-level RMT modules

**Responsibilities:**

- Includes all five RMT modular headers in correct order
- Maintains backward compatibility with existing code
- Acts as the abstraction layer between high-level API and RMT implementation

**Contents:**

```cpp
#include "ll_led_timings.h"   // Timing constants (RMT + PARLIO)
#include "ll_priority.h"       // Priority management
#include "ll_encoder.h"        // Encoder callback
#include "ll_strip_core.h"     // Core operations
#include "ll_strip_pixels.h"   // Pixel operations (shared)
```

**Relationship:** Central hub that aggregates all RMT low-level functionality.

---

## Low-Level Modules

### 1. `ll_led_timings.h`

**Purpose:** LED strip timing definitions and RMT symbol constants

**Responsibilities:**

- Defines RMT symbol timing for different LED strip types:
  - WS2812/WS2812B (GRB)
  - APA106 (RGB)
  - SM16703 (RGB)
  - SK6812 (RGBW)
- Stores timing values for logical 0, logical 1, and reset pulses
- Provides `led_params[]` lookup array indexed by LED type
- Manages colour order configuration (standard and custom)

**Key Data Structures:**

```cpp
typedef struct {
    rmt_symbol_word_t led_0;     // Symbol for bit 0
    rmt_symbol_word_t led_1;     // Symbol for bit 1
    rmt_symbol_word_t led_reset; // Reset/latch symbol
} led_timing_params_t;

extern led_timing_params_t led_params[4];  // Indexed by led_type_t
```

**Dependencies:**

- `LiteLED.h` (for type definitions)
- ESP-IDF RMT driver headers

**Relationship:** Provides timing constants consumed by encoder and core modules. Pure data, no functions.

---

### 2. `ll_priority.h` / `ll_priority.cpp`

**Purpose:** RMT interrupt priority allocation and tracking

**Responsibilities:**

- Tracks which interrupt priorities are in use across all active LED strips
- Provides priority availability checking before allocation
- Implements fallback mechanism when requested priority is unavailable
- Maintains priority usage state for up to 4 distinct priority levels
- Provides debug functions to query and display priority status

**Key Functions:**

```cpp
bool ll_is_priority_available(int priority);
int ll_find_best_available_priority(int preferred_priority);
void ll_mark_priority_used(int priority);
void ll_mark_priority_free(int priority);
void ll_reset_priority_tracking();
const char* ll_priority_to_string(int priority);
```

**Key State:**

```cpp
bool ll_priority_used[4];          // Tracks which priorities are in use
uint8_t ll_active_channels;        // Count of active LED strips
const int ll_priority_fallbacks[]; // Fallback order: DEFAULT, HIGH, MEDIUM, LOW
```

**Dependencies:**

- `Arduino.h`
- ESP32 HAL logging

**Relationship:** Used by `ll_strip_core` during initialization to prevent priority conflicts when multiple LED strips are active.

---

### 3. `ll_encoder.h` / `ll_encoder.cpp`

**Purpose:** RMT encoder callback for converting LED data to RMT symbols

**Responsibilities:**

- Implements the encoder callback invoked by ESP-IDF RMT driver
- Converts LED buffer bytes into RMT symbol sequences
- Applies brightness scaling using `scale8_video()`
- Handles data transmission completion and reset signal generation
- Manages encoder state (`enc_pos` - current position in LED buffer)

**Key Function:**

```cpp
IRAM_ATTR size_t led_encoder_cb(const void* data, size_t data_size,
                                 size_t symbols_written, size_t symbols_free,
                                 rmt_symbol_word_t *symbols, bool *done, void *arg);
```

**Algorithm:**

1. Check if sufficient symbol space is available (minimum 8 for one byte)
2. Read next byte from LED buffer
3. Apply brightness scaling via `scale8_video()`
4. Convert byte to 8 RMT symbols (one per bit)
5. Lookup correct timing from `led_params[]` based on strip type
6. Write symbols to RMT output buffer
7. When all data sent, append reset symbol and signal completion

**Dependencies:**

- `LiteLED.h` (for `led_strip_t` structure)
- `llrgb.h` (for `scale8_video()`)
- `ll_led_timings.h` (for `led_params[]`)

**Performance:** Marked `IRAM_ATTR` to place in fast instruction RAM for zero-wait-state execution during RMT transmission.

**Relationship:** Called by ESP-IDF RMT driver during transmission. This is the performance-critical path that runs in interrupt context.

---

### 4. `ll_strip_core.h` / `ll_strip_core.cpp`

**Purpose:** Core LED strip lifecycle management

**Responsibilities:**

- LED strip initialization and configuration
- RMT channel allocation and configuration
- Memory allocation (PSRAM vs internal RAM with fallback)
- Priority management integration with pre-flight conflict detection
- Channel installation, enabling, and cleanup
- Debug reporting and diagnostics

**Key Functions:**

```cpp
esp_err_t led_strip_init(led_strip_t *strip);
esp_err_t led_strip_init_modify(led_strip_t *strip);
esp_err_t led_strip_install(led_strip_t *strip);
esp_err_t led_strip_free(led_strip_t *strip);
esp_err_t led_strip_flush(led_strip_t *strip);
void led_strip_debug_dump(led_strip_t *strip);
```

**Initialization Sequence:**

1. **`led_strip_init()`:** Configure RMT encoder and transmit settings
2. **`led_strip_init_modify()`:** Apply user customizations (priority, DMA, memory)
3. **`led_strip_install()`:** 
   - Allocate LED buffer (PSRAM preferred, internal RAM fallback)
   - Check priority availability (pre-flight checks)
   - Create RMT TX channel with ESP-IDF driver
   - Handle priority conflicts with automatic fallback
   - Register priority usage
   - Enable RMT channel

**Memory Management:**

- Attempts PSRAM allocation first (preferred for large buffers)
- Falls back to internal RAM if PSRAM unavailable
- Tracks allocation location for debugging

**Priority Conflict Handling:**

- Pre-flight check using `ll_is_priority_available()`
- Automatic fallback to best available priority if conflict detected
- Logs all priority decisions for debugging

**Dependencies:**

- `LiteLED.h`
- `ll_led_timings.h`
- `ll_priority.h`
- `ll_encoder.h`
- ESP-IDF RMT driver

**Relationship:** This is the main initialization and setup module. Called by `LiteLED.cpp` during strip creation. Uses all other low-level modules.

---

### 5. `ll_strip_pixels.h` / `ll_strip_pixels.cpp`

**Purpose:** Pixel-level colour manipulation operations

**Responsibilities:**

- Individual pixel get/set operations
- Bulk pixel operations (fill, fill random, clear)
- Color order translation (RGB, RBG, GRB, GBR, BRG, BGR)
- RGBW white channel handling with auto-white calculation
- Brightness control (global and per-operation)

**Key Functions:**

```cpp
// Individual pixel operations
esp_err_t led_strip_set_pixel(led_strip_t *strip, uint16_t pixel_num, uint8_t r, uint8_t g, uint8_t b);
esp_err_t led_strip_set_pixel_rgbw(led_strip_t *strip, uint16_t pixel_num, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
esp_err_t led_strip_get_pixel(led_strip_t *strip, uint16_t pixel_num, uint8_t *r, uint8_t *g, uint8_t *b);

// Bulk operations
esp_err_t led_strip_set_pixels(led_strip_t *strip, uint16_t start_pixel, uint16_t count, const uint8_t *pixels);
esp_err_t led_strip_fill(led_strip_t *strip, uint16_t start, uint16_t count, uint8_t r, uint8_t g, uint8_t b);
esp_err_t led_strip_fill_random(led_strip_t *strip, uint16_t start, uint16_t count);
esp_err_t led_strip_clear(led_strip_t *strip);

// Brightness control
esp_err_t led_strip_set_brightness(led_strip_t *strip, uint8_t brightness);
uint8_t led_strip_get_brightness(led_strip_t *strip);

// Color order management
void led_strip_set_color_order(color_order_t order);
```

**Color Order Translation:**

- Supports the six possible colour orders (RGB, RBG, GRB, GBR, BRG, BGR)
- Uses lookup table for efficient translation
- Custom colour order overrides strip-specific defaults
- Essential for hardware compatibility (eg: WS2812 uses GRB, APA106 uses RGB)

**RGBW Support:**

- Auto-white calculation: extracts common component from RGB
- Manual white channel control available
- Handles 4-byte RGBW buffer layout

**Dependencies:**

- `LiteLED.h`
- `ll_led_timings.h` (for colour order state)

**Relationship:** Provides all pixel manipulation functionality called by `LiteLED.cpp` public methods. Works with the LED buffer allocated by `ll_strip_core`.

---

## PARLIO Modules

The following modules implement the PARLIO-based `LiteLEDpio` driver.  All are
guarded by `#if SOC_PARLIO_SUPPORTED` so they compile only on SoCs that have the
PARLIO peripheral (e.g., ESP32-C6, ESP32-H2).

---

### `llparlio.h` (PARLIO Compatibility Header)

**Purpose:** Single include point for all PARLIO low-level modules — mirrors `llrmt.h`

**Responsibilities:**

- Emits `#error` if included on a target without `SOC_PARLIO_SUPPORTED`
- Includes timing and core headers in correct order

**Contents:**

```cpp
#include "ll_led_timings.h"   // Shared timing constants (PARLIO table)
#include "ll_parlio_core.h"   // PARLIO lifecycle functions
```

**Relationship:** Included by `LiteLED.h` (conditionally) and by `LiteLEDpio.cpp`.

---

### `ll_parlio_core.h` / `ll_parlio_core.cpp`

**Purpose:** PARLIO strip lifecycle management — equivalent to `ll_strip_core` for RMT

**Responsibilities:**

- PARLIO TX unit allocation and configuration
- DMA bitstream buffer allocation (must be internal DMA-capable RAM)
- Pixel colour buffer allocation (PSRAM-aware, matches RMT behaviour)
- Brightness-scaled encoding of pixel data into the DMA buffer
- Transmit, block-until-done, and cleanup

**Key Functions:**

```cpp
// Single-strip lifecycle
esp_err_t parlio_strip_init   (led_strip_t *strip, parlio_strip_cfg_t *cfg);
esp_err_t parlio_strip_install(led_strip_t *strip, parlio_strip_cfg_t *cfg);
esp_err_t parlio_strip_flush  (led_strip_t *strip, parlio_strip_cfg_t *cfg);
esp_err_t parlio_strip_free   (led_strip_t *strip, parlio_strip_cfg_t *cfg);
void      parlio_strip_debug_dump(led_strip_t *strip, parlio_strip_cfg_t *cfg);

// Multi-strip group lifecycle
esp_err_t parlio_group_install(parlio_group_cfg_t *gcfg);
esp_err_t parlio_group_flush  (parlio_group_cfg_t *gcfg);
esp_err_t parlio_group_free   (parlio_group_cfg_t *gcfg);
```

**Encoding (`data_width=8`, single-strip):**

Each input byte is expanded to 24 DMA bytes (3 bytes per bit, 8 bits per byte).
Only `data_gpio_nums[0]` (bit 0) is wired to the LED strip; bits 1–7 are
routed to `GPIO_NUM_NC`, so no bit-packing or DMA endianness issues arise.

```
LED bit 0  →  DMA bytes: 0x01 0x00 0x00  (T0H=400 ns, T0L=800 ns)
LED bit 1  →  DMA bytes: 0x01 0x01 0x00  (T1H=800 ns, T1L=400 ns)
```

**Multi-lane encoding (`parlio_group_flush`):**

The group encoder zeroes the shared DMA buffer, then for each assigned lane
ORs the lane's brightness-scaled sample bits into the appropriate bit position
of each DMA byte:

```
buf[offset + sample] |= (sample_bit << lane_index)
```

This produces a single DMA bitstream where bit N of each byte carries the
waveform for lane N's LED strip.  Each lane's `data_gpio_nums[N]` pin
therefore outputs an independent signal, but all lanes are transmitted
simultaneously from a single `parlio_tx_unit_transmit()` call.

**Supported hardware:** ESP32-C6 (1 PARLIO TX unit, up to 8 data lines).
The ESP32-C6 PARLIO TX unit has `SOC_PARLIO_TX_UNITS_PER_GROUP = 1`, meaning
only one `LiteLEDpio` or `LiteLEDpioGroup` instance can be active at a time.
The `parlio_encode_byte()` helper (single-strip path) is `IRAM_ATTR`-qualified.
The `parlio_group_flush()` multi-lane encoder runs in the Arduino loop task.

**Dependencies:**

- `ll_led_timings.h` (PARLIO timing table)
- `llrgb.h` (`scale8_video()` for brightness)
- `LiteLED.h` (`led_strip_t`, `parlio_strip_cfg_t`, `parlio_group_cfg_t`)
- ESP-IDF `driver/parlio_tx.h`

---

### `LiteLEDpio.cpp`

**Purpose:** `LiteLEDpio` class implementation — PARLIO equivalent of `LiteLED.cpp`

**Responsibilities:**

- Implements all public methods with identical signatures to `LiteLED`
- Delegates pixel operations to the shared `ll_strip_pixels` layer
- Manages `parlio_strip_cfg_t` state (DMA buffer, channel handle)
- Registers/unregisters the GPIO with the ESP32 Peripheral Manager

**Public API:** Identical to `LiteLED` — same method names, parameter types, and
return types.  User code can switch between RMT and PARLIO drivers by changing
one type declaration.

**Limitations vs `LiteLED`:**

- No DMA flag (`DMA_ON`/`DMA_OFF`): PARLIO always uses DMA internally
- No interrupt priority setting: no ISR callback in the PARLIO path
- `getActiveInstanceCount()` reflects Peripheral Manager registration only
- Only one instance active at a time on ESP32-C6

---

### `LiteLEDpioGroup.cpp`

**Purpose:** `LiteLEDpioGroup` and `LiteLEDpioLane` class implementations — multi-strip PARLIO driver

**Responsibilities:**

- Owns one PARLIO TX unit and one shared DMA bitstream buffer
- Each strip is registered via `addStrip(gpio)` or `addStrip<LANE>(gpio)` before `begin()`
- `begin()` allocates per-lane pixel colour buffers (PSRAM-aware) and the shared DMA buffer, creates the PARLIO TX unit, and registers all GPIOs with the Peripheral Manager
- `show()` calls `parlio_group_flush()` and syncs brightness state
- `LiteLEDpioLane` is a thin handle: all pixel methods delegate to `ll_strip_pixels`; `show()` delegates to the parent group

**Key Classes:**

```cpp
class LiteLEDpioGroup {
    LiteLEDpioGroup(led_strip_type_t led_type, size_t length, bool rgbw);
    LiteLEDpioLane &addStrip(uint8_t gpio);            // sequential lane assignment
    template<uint8_t LANE>
    LiteLEDpioLane &addStrip(uint8_t gpio);            // explicit lane (compile-time check)
    esp_err_t begin(ll_psram_t psram_flag = PSRAM_DISABLE);
    esp_err_t show();
    esp_err_t brightness(uint8_t bright, bool show = false);
    LiteLEDpioLane &operator[](uint8_t lane);
};

class LiteLEDpioLane {
    esp_err_t show();                // delegates to parent group
    esp_err_t setPixel(...);
    esp_err_t fill(...);
    esp_err_t clear(bool show = false);
    // ... same pixel API as LiteLED / LiteLEDpio
};
```

**Constraints:**

- All strips must share the same LED type, length, and RGBW flag
- Maximum strips: `PARLIO_TX_UNIT_MAX_DATA_WIDTH` (8 on C6/H2, 16 on P4)
- Only one `LiteLEDpioGroup` (or `LiteLEDpio`) active at a time on C6/H2

**Dependencies:**

- `LiteLED.h` (class declarations, `parlio_group_cfg_t`)
- `ll_parlio_core.h` (`parlio_group_install`, `parlio_group_flush`, `parlio_group_free`)
- `ll_strip_pixels.h` (pixel operations delegated from `LiteLEDpioLane`)
- `esp32-hal-periman.h` (GPIO registration)

---

## Supporting Modules

### `ll_registry.h` / `ll_registry.cpp`

**Purpose:** Minimal instance tracking for cleanup callbacks

**Size:** ~44 lines (header) + ~200 lines (implementation)

**Responsibilities:**

- Maintains minimal mapping of RMT channels to LiteLED instances
- Provides deinit callback for ESP32 Peripheral Manager integration
- Handles forced cleanup when GPIO pins are reassigned to other peripherals
- Thread-safe access to instance mappings
- GPIO availability checking and instance counting (delegates to Peripheral Manager)

**Key Functions:**

```cpp
esp_err_t ll_registry_init(void);                                              // Initialize registry
esp_err_t ll_register_channel_instance(rmt_channel_handle_t channel, LiteLED* instance);  // Register mapping
void ll_unregister_channel_instance(rmt_channel_handle_t channel);            // Unregister mapping
LiteLED* ll_get_instance_by_gpio(uint8_t gpio);                               // Query by GPIO
uint8_t ll_registry_get_active_count(void);                                   // Count active instances
bool ll_periman_deinit_callback(void *bus_handle);                            // Cleanup callback
```

**Architecture Changes (v3.0.0):**

- **Simplified Design:** Registry now only tracks RMT channel → instance mappings
- **Delegates GPIO Management:** All GPIO tracking/conflict detection handled by ESP32 Peripheral Manager
- **Minimal Memory Footprint:** Reduced from full registry entries to simple channel mapping
- **Direct Peripheral Manager Integration:** `LiteLED.cpp` calls `perimanSetPinBus()` directly

**Integration with ESP32 Peripheral Manager:**

- Registers pins as `ESP32_BUS_TYPE_RMT_TX` with extra type "LiteLED"
- Provides `ll_periman_deinit_callback()` for forced cleanup scenarios
- Automatically invalidates LiteLED instances when GPIO pins are reassigned
- Ensures system stability during peripheral conflicts

**Dependencies:**

- `LiteLED.h` (forward declaration)
- `esp32-hal-periman.h` (ESP32 Peripheral Manager)
- ESP32 HAL logging and threading primitives

**Relationship:** Called by `LiteLED.cpp` during initialization and cleanup. Provides the bridge between LiteLED instances and the ESP32 Peripheral Manager for conflict detection and forced cleanup handling.

---

### `llrgb.h`

**Purpose:** RGB colour manipulation utilities

**Size:** ~100 lines (header only, inline functions)

**Responsibilities:**

- Fast 8-bit integer math for colour operations
- Brightness scaling with gamma correction
- Color blending and interpolation
- HSV to RGB conversion
- Bit manipulation utilities

**Key Functions:**

```cpp
uint8_t scale8(uint8_t value, uint8_t scale);         // Linear scaling
uint8_t scale8_video(uint8_t value, uint8_t scale);   // Video-style dimming
uint8_t qadd8(uint8_t a, uint8_t b);                  // Saturating add
uint8_t qsub8(uint8_t a, uint8_t b);                  // Saturating subtract
```

**Performance:** All inline for zero-overhead abstraction.

**Relationship:** Used by `ll_encoder` for brightness scaling. Can be used by user code for colour manipulation.

---

## Data Flow

### Initialization Flow
```
User Code
  └─> LiteLED::begin()
      ├─> Check GPIO availability [perimanPinIsValid, perimanGetPinBusType]
      ├─> led_strip_init() [ll_strip_core]
      │   ├─> Configure RMT encoder [ll_encoder]
      │   └─> Configure RMT transmit
      ├─> led_strip_init_modify() [ll_strip_core]
      │   ├─> Set priority
      │   ├─> Set DMA usage
      │   └─> Set memory block size
      ├─> led_strip_install() [ll_strip_core]
      │   ├─> Allocate LED buffer (PSRAM/internal)
      │   ├─> Check priority availability [ll_priority]
      │   ├─> Create RMT TX channel (ESP-IDF)
      │   ├─> Mark priority used [ll_priority]
      │   └─> Enable RMT channel
      ├─> Register with Peripheral Manager [perimanSetPinBus, perimanSetPinBusExtraType]
      └─> Register channel mapping [ll_register_channel_instance]
```

### Pixel Update Flow (RMT)

```
User Code
  └─> LiteLED::setPixel(n, r, g, b)
      └─> led_strip_set_pixel() [ll_strip_pixels]
          ├─> Translate color order [ll_led_timings]
          ├─> Calculate buffer offset
          └─> Write RGB to buffer
  └─> LiteLED::show()
      └─> led_strip_flush() [ll_strip_core]
          └─> rmt_transmit() (ESP-IDF)
              └─> led_encoder_cb() [ll_encoder] (interrupt context)
                  ├─> Read from LED buffer
                  ├─> Apply brightness [llrgb::scale8_video]
                  ├─> Lookup timing [ll_led_timings]
                  └─> Write RMT symbols
```

### Pixel Update Flow (PARLIO)

```
User Code
  └─> LiteLEDpio::setPixel(n, r, g, b)
      └─> led_strip_set_pixel() [ll_strip_pixels]   ← same shared layer
          ├─> Translate color order [ll_led_timings]
          ├─> Calculate buffer offset
          └─> Write RGB to buffer
  └─> LiteLEDpio::show()
      └─> parlio_strip_flush() [ll_parlio_core]
          ├─> For each pixel byte:
          │   ├─> scale8_video() [llrgb]
          │   └─> parlio_encode_byte() → 24 DMA bytes
          └─> parlio_tx_unit_transmit() (ESP-IDF DMA)
              └─> parlio_tx_unit_wait_all_done() (blocking)
```

### Pixel Update Flow (PARLIO Group)

```
User Code
  └─> laneRef.setPixel(n, r, g, b)            ← per-lane pixel write
      └─> led_strip_set_pixel() [ll_strip_pixels]   ← same shared layer
  └─> strips.show()  (or laneRef.show())
      └─> LiteLEDpioGroup::show() [LiteLEDpioGroup.cpp]
          └─> parlio_group_flush() [ll_parlio_core]
              ├─> memset(DMA buffer, 0)
              ├─> For each assigned lane N:
              │   └─> For each pixel byte:
              │       ├─> scale8_video() [llrgb]
              │       └─> buf[offset+s] |= (sample_bit << N)
              └─> parlio_tx_unit_transmit() (ESP-IDF DMA, all lanes)
                  └─> parlio_tx_unit_wait_all_done() (blocking)
```

### Cleanup Flow (RMT)

```
User Code
  └─> LiteLED::~LiteLED() or LiteLED::free()
      ├─> Mark instance invalid [valid_instance = false]
      ├─> Unregister channel mapping [ll_unregister_channel_instance]
      ├─> Unregister from Peripheral Manager [perimanSetPinBus]
      └─> led_strip_free() [ll_strip_core]
          ├─> Disable and delete RMT channel (ESP-IDF)
          ├─> Mark priority free [ll_priority]
          └─> Free LED buffer
```

### Cleanup Flow (PARLIO)

```
User Code
  └─> LiteLEDpio::~LiteLEDpio()
      ├─> Unregister from Peripheral Manager [perimanSetPinBus]
      └─> parlio_strip_free() [ll_parlio_core]
          ├─> parlio_tx_unit_wait_all_done()
          ├─> parlio_tx_unit_disable()
          ├─> parlio_del_tx_unit()
          ├─> Free DMA bitstream buffer
          └─> Free pixel colour buffer
```
---

## Module Interaction Summary

| Module | Calls | Called By | Key Responsibility |
|--------|-------|-----------|-------------------|
| `LiteLED` | All RMT modules, Peripheral Manager | User code | RMT public API |
| `LiteLEDpio` | PARLIO modules, `ll_strip_pixels`, Peripheral Manager | User code | PARLIO single-strip API |
| `LiteLEDpioGroup` | `ll_parlio_core` (group), `ll_strip_pixels`, Peripheral Manager | User code | PARLIO multi-strip API |
| `LiteLEDpioLane` | `ll_strip_pixels`, parent `LiteLEDpioGroup` | User code / `LiteLEDpioGroup` | Per-lane pixel handle |
| `llrmt` | All ll_* RMT modules | `LiteLED` | RMT module aggregation |
| `llparlio` | `ll_led_timings`, `ll_parlio_core` | `LiteLEDpio` | PARLIO module aggregation |
| `ll_led_timings` | None | `ll_encoder`, `ll_strip_pixels`, `ll_parlio_core` | Data provider (RMT + PARLIO) |
| `ll_priority` | None | `ll_strip_core` | RMT priority tracking |
| `ll_encoder` | `llrgb`, `ll_led_timings` | ESP-IDF RMT (interrupt) | RMT data encoding |
| `ll_strip_core` | `ll_priority`, `ll_encoder` | `LiteLED` | RMT lifecycle management |
| `ll_parlio_core` | `llrgb`, `ll_led_timings` | `LiteLEDpio`, `LiteLEDpioGroup` | PARLIO lifecycle + DMA encoding |
| `ll_strip_pixels` | `ll_led_timings` | `LiteLED`, `LiteLEDpio`, `LiteLEDpioLane` | Shared pixel operations |
| `ll_registry` | Peripheral Manager | `LiteLED` | Minimal RMT instance tracking |
| `Peripheral Manager` | None | `LiteLED`, `LiteLEDpio`, `LiteLEDpioGroup`, `ll_registry` | GPIO conflict prevention |
| `llrgb` | None | `ll_encoder`, `ll_parlio_core`, User code | Color math |

---

## Design Principles

### Separation of Concerns

Each module has a single, well-defined responsibility:

- **Timings:** Static data
- **Priority:** State tracking
- **Encoder:** Data transformation
- **Core:** Lifecycle management
- **Pixels:** Buffer manipulation

### Minimal Dependencies

Modules depend only on what they need:

- **Timings:** Header-only, no dependencies
- **Priority:** Self-contained state management
- **Encoder:** Depends only on timing data and color math
- **Core:** Orchestrates all modules
- **Pixels:** Minimal dependency on timing data

### Performance-Critical Paths

- **Encoder:** Placed in IRAM for zero-wait-state execution
- **RGB utilities:** Inline functions for zero overhead
- **Timing data:** Constant lookup (compile-time optimized)

### Thread Safety

- **Registry:** Mutex-protected for multi-threaded access (channel mappings only)
- **Peripheral Manager:** Built-in thread safety for GPIO tracking and conflict detection
- **Priority:** Single-threaded during initialization (by design)
- **Encoder:** Read-only access to timing data (no locking needed)

---

## Benefits of Modular Architecture

1. **Maintainability:** Each module can be understood and modified independently
2. **Testability:** Modules can be unit tested in isolation
3. **Reusability:** Modules can be used in other projects (e.g., `llrgb.h`)
4. **Conflict Prevention:** Direct ESP32 Peripheral Manager integration prevents GPIO conflicts with other peripherals
5. **System Stability:** Forced cleanup callbacks ensure graceful handling when pins are reassigned
6. **Compile Time:** Only changed modules need recompilation
7. **Documentation:** Each module can be documented separately
8. **Extensibility:** New LED types can be added to `ll_led_timings.h` without modifying other code

---

## Version History

**v3.1.0**
- Added PARLIO single-strip driver (`LiteLEDpio`) targeting ESP32-C6 and other PARLIO-capable SoCs
- Added PARLIO multi-strip driver (`LiteLEDpioGroup` / `LiteLEDpioLane`) — up to 8 independent strips driven simultaneously from one PARLIO TX unit via per-bit-lane DMA encoding
- New files: `LiteLEDpio.cpp`, `LiteLEDpioGroup.cpp`, `ll_parlio_core.h`, `ll_parlio_core.cpp`, `llparlio.h`
- Modified: `LiteLED.h` (added `parlio_strip_cfg_t`, `parlio_lane_t`, `parlio_group_cfg_t`, `LiteLEDpio`, `LiteLEDpioLane`, `LiteLEDpioGroup` classes and structs), `ll_led_timings.h` (added PARLIO timing table)
- Single-strip encoding uses `data_width=8` to avoid DMA endianness corruption
- Group encoder ORs per-lane sample bits into shared DMA bytes (`buf[off+s] |= bit << lane`)
- All PARLIO code guarded by `#if SOC_PARLIO_SUPPORTED` for safe multi-target builds
- Hardware-validated on XIAO ESP32-C6, arduino-esp32 3.3.7, IDF v5.5.2

**v3.0.0**
- Initial release for library version 3.0.0
- Modular architecture implementation
- ESP32 Peripheral Manager integration for GPIO conflict prevention
- Simplified `ll_registry` to minimal instance tracking (delegates GPIO management to Peripheral Manager)
- Direct `perimanSetPinBus()` calls from `LiteLED.cpp` for resource management


<!-- //  --- EOF --- // -->

