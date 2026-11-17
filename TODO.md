# TODO

## Fix SK6812 timing definition (RMT)

**Status: code fix done in `d0e542e` (dev). Hardware test on real SK6812 strips (steps 2 to 4 below) still open.**

**File:** `src/ll_led_timings.h`

**Problem:** `led_sk6812` is built from a mix of SK6812 and SM16703 symbols. The SK6812-specific `sk6812_one` and `sk6812_reset` are defined but never used.

```cpp
static const led_params_t led_sk6812 = {
    .led_0 = sk6812_zero,
    .led_1 = sm16703_one,       // should be sk6812_one
    .led_reset = sm16703_reset, // should be sk6812_reset
    .order = ORDER_GRB
};
```

**Effect today:**

| | As coded for SK6812 | Actually sent |
|---|---|---|
| "1" bit | T1H 0.64 µs / T1L 0.40 µs | T1H 0.90 µs / T1L 0.30 µs (SM16703) |
| Reset | 90 µs | 210 µs (SM16703) |

**Fix:**

1. In `led_sk6812`, change `.led_1` to `sk6812_one` and `.led_reset` to `sk6812_reset`.
2. Check the tick truncation at the 10 MHz RMT resolution (1 tick = 100 ns). `sk6812_zero` T0H of 0.32 µs truncates to 3 ticks (300 ns). `sk6812_one` is 6 ticks high and 4 ticks low (600/400 ns), so the "1" bit period is 1000 ns against 1200 ns for the "0" bit. Confirm these are within the SK6812 datasheet limits, or set the values to whole-tick figures.
3. Test on real SK6812 (RGB and RGBW) hardware with RMT. Ideally take scope captures of T0H/T0L/T1H/T1L and the reset gap. Check that long strips latch correctly and show no colour glitches. The 90 µs reset is shorter than the 210 µs now in use, so watch for strips that fail to latch.
4. Update the timing table in the docs if one is published.

**Not affected:** PARLIO uses one shared timing set for all types (2.5 MHz, 3 samples per bit), so this bug only applies to the RMT driver. Whether the shared PARLIO timing suits SK6812 is a separate question.

**Related:** the WorldSemi 325 ns T0H discussion, https://github.com/Xylopyrographer/LiteLED/discussions/33

## Remaining checks for LED_STRIP_WS281XV7 and custom timing (v3.3.0 shipped 2026-11-16)

Loopback-verified on ESP32-PICO-D4, S3, C6 and P4 (RMT, and PARLIO on C6 and P4), with a visual check on the ATOM Matrix's 25 GRB LEDs. Still open:

1. Test on more real WS281x strips and check colours and latching.
2. ESP32-H2 is untested. Its default PARLIO source is PLL_F96M with an integer divider only, so 2.5, 5 and 20 MHz do not divide exactly (about 2.53, 5.26 and 19.2 MHz). The driver reports a precision loss in `parlio_tx.c`; the library does not check it.
3. One-tick (12.5 ns) PARLIO edge deviations were seen only on the C6, on some runs (built-in 2.5 MHz types about 391/809 ns instead of 400/800, the 40 MHz custom case about 316/933 instead of 325/925; bit period unchanged; `LED_STRIP_WS281XV7` was exact every time). The P4 showed none. A scope check was dropped as hard to capture. The PARLIO setup follows the IDF sequence and every C6 candidate clock divides its source exactly, so clock rounding is not the cause. Cause unknown.
4. The `DebugLevel=verbose` check of the `LiteLED version:` line was skipped for 3.3.0.

## PARLIO Peripheral Manager bus type

arduino-esp32 has no `ESP32_BUS_TYPE_PARLIO_TX` (checked through 3.3.12 and `master`), so PARLIO pins are registered as `ESP32_BUS_TYPE_GPIO` with no deinit callback. Requested upstream: https://github.com/espressif/arduino-esp32/issues/13130

When a core ships the type: the `#ifdef ESP32_BUS_TYPE_PARLIO_TX` tests in `ll_parlio_core.h`, `LiteLEDpio.cpp`, `LiteLEDpioGroup.cpp` and `ll_parlio_core.cpp` can never be true (bus types are enum constants), so replace them with an `ESP_ARDUINO_VERSION_VAL()` check against the first release that has it, then test the dedicated path on a PARLIO chip.

## WS2814 (WRGB colour order)

The WS2814A/B/F send the white channel first (WRGB). `color_order_t` has only the six RGB permutations and the library always sends W last, so these parts can't be driven correctly. Add an `ORDER_WRGB` value (valid only with `rgbw = true`) with W-first cases in `led_strip_set_pixel()` and `led_strip_get_pixel()`, the `col_ord[]` table, the PARLIO path and `keywords.txt`. Not supported in v3.3.0; planned for a later release.

## Colour order is not per strip

`led_strip_set_color_order()` and `led_strip_set_default_color_order()` set `custom_color_order` and `use_custom_color_order` without using the strip, so they appear to apply to every strip rather than the one `setOrder()` was called on. Not yet confirmed. Check, and either fix or document.
