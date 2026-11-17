# TODO

# v3.3.1 plan

## Audit built-in timings against OEM datasheets

Source the OEM datasheet for every built-in type (WS2812, WS2812_RGB, SK6812, APA106, SM16703; WS281XV7 was checked against 13 sheets, see `Discussions/YInWorldSemi/WS281X_Datasheets/TIMING_SUMMARY.md`) and check each one's RMT and PARLIO timing against the source material: T0H, T0L, T1H, T1L, code period and reset, after the 10 MHz tick truncation (RMT) and the shared 2.5 MHz / 3 sample pattern (PARLIO). Record the sheet (maker, revision) and the result per type. Keep sheets in `Discussions/OtherDataSheets/`.

## SK6812 timing (RMT)

**Status: `d0e542e` (in v3.3.0) switched `led_sk6812` to its own 1-bit and reset values. Checked against the Shenzhen Normand SK6812 datasheet Rev. 06, page 6 (`Discussions/OtherDataSheets/SK6812 LED  Datasheet.pdf`); the changes below are not yet made. No test on real SK6812 strips yet.**

**File:** `src/ll_led_timings.h`

Datasheet (section 12): code period min 1.20 µs; T0H 0.2 / **0.32** / 0.4; T0L min 0.8; T1H 0.58 / **0.64** / 1.0; T1L min 0.2; reset **>80 µs**. (The heading says "1.25 µs ± 600 ns", which contradicts the table's 1.20 µs minimum.) The LiteLED values are the datasheet's "standard" column; the reset of 90 µs is above the minimum.

Findings, at the 10 MHz RMT tick (1 tick = 100 ns):

| | Sent now | Datasheet | Problem |
|---|---|---|---|
| "0" bit | 300 / 900 ns, period 1200 | T0H 200-400, T0L >= 800, T >= 1200 | none |
| "1" bit | 600 / 400 ns, **period 1000** | T1H 580-1000, T1L >= 200, **T >= 1200** | period below minimum; T1H margin 20 ns |
| Reset | 90 µs | > 80 µs | margin 10 µs |

Before `d0e542e` the "1" bit was the SM16703 900 / 300 (period 1200), which met every row; the fix made the high time match the "standard" value but shortened the period.

**Proposed fix:**

1. "1" bit 700 / 500 ns (T1H 700 inside 580-1000, T1L 500 >= 200, period 1200). The "0" bit stays 300 / 900.
2. Reset 280 µs (esp-idf's `led_strip` uses 280 µs; 100 µs would also pass).
3. Test on real SK6812 (RGB and RGBW) with RMT, ideally with scope captures. Check long strips latch and show no colour glitches.
4. Update the timing table in the docs if one is published and add a version history line (the fix in `d0e542e` was not mentioned in the v3.3.0 notes).

**PARLIO:** the shared timing (2.5 MHz, 1/2/2/1 samples) sends T0H 400 ns and T0L 800 ns, exactly on the SK6812 limits (T0H max 0.4 µs, T0L min 0.8 µs). C6 measured about 391 / 809 ns. Part of the audit above.

**For reference:** the esp-idf `led_strip` component's SK6812 values are T0H 300, T0L 900, T1H 700, T1L 600 ns, reset 280 µs (not the source of LiteLED's values).

**Related:** the WorldSemi 325 ns T0H discussion, https://github.com/Xylopyrographer/LiteLED/discussions/33

## Peripheral Manager deinit callbacks leak the hardware (PARLIO, probably RMT)

Found 2026-11-17 by running the PARLIO Peripheral Manager check on an XIAO ESP32-C6 against arduino-esp32 3.3.11 with the `ESP32_BUS_TYPE_PARLIO_TX` patch applied (the dedicated path had never run before). Same result on 3.3.2. Test sketch: `custom_timing_test/src_periman/main_periman_pio.cpp`.

1. **Leak.** `ll_parlio_deinit_cb()` (`ll_parlio_core.cpp`) only sets the instance's valid flag to false and drops the registry entry. It never disables or deletes the PARLIO TX unit. `LiteLEDpio::begin()` only calls `free()` when `valid_instance` is true, so a `begin()` after a forced reassignment (for example `pinMode()` on the strip's pin) allocates a second unit while the first is still held. The C6 has one TX unit, so it fails with `no free tx unit`, and so does everything after it. Fix: free the unit when the instance is invalidated, or have `begin()`/`free()`/the destructor free it whenever `parlioCfg.parlio_chan` is still set. Same check for `LiteLEDpioGroup`.
2. **`free()` clears a pin it no longer owns.** After a forced reassignment, `free()` still calls `perimanSetPinBus( gpio, ESP32_BUS_TYPE_INIT, ... )`, which would release the new owner's registration. Only release the pin if `perimanGetPinBus()` still returns our handle.
3. **RMT has the same shape.** `ll_periman_deinit_callback()` (`ll_registry.cpp`) calls `invalidate()` and leaves the channel allocated ("cleanup is handled by the peripheral taking over" is not true, because LiteLED replaced the RMT callback). Untested: the RMT periman check passes only because the chip has spare TX channels. Test by forcing reassignment more times than there are channels.
4. **`LiteLEDpio::getActiveInstanceCount()` is a stub that returns 0** (a `TODO` in `LiteLEDpio.cpp`), so the periman check "active instances +1" fails on every core. Implement the counter or remove the check and document the stub.

Rig: env `xiao-c6-periman` (PlatformIO, platform 55.03.32 = core 3.3.2) or the arduino-cli patched-core copy method in `custom_timing_test/SESSION_NOTES.md` (update 2026-11-17). Fix before LiteLED relies on the dedicated bus type.

# Open items (v3.3.0 shipped 2026-11-16)

## Remaining checks for LED_STRIP_WS281XV7 and custom timing (v3.3.0 shipped 2026-11-16)

Loopback-verified on ESP32-PICO-D4, S3, C6 and P4 (RMT, and PARLIO on C6 and P4), with a visual check on the ATOM Matrix's 25 GRB LEDs. Still open:

1. Test on more real WS281x strips and check colours and latching.
2. ESP32-H2 is untested. Its default PARLIO source is PLL_F96M with an integer divider only, so 2.5, 5 and 20 MHz do not divide exactly (about 2.53, 5.26 and 19.2 MHz). The driver reports a precision loss in `parlio_tx.c`; the library does not check it.
3. One-tick (12.5 ns) PARLIO edge deviations were seen only on the C6, on some runs (built-in 2.5 MHz types about 391/809 ns instead of 400/800, the 40 MHz custom case about 316/933 instead of 325/925; bit period unchanged; `LED_STRIP_WS281XV7` was exact every time). The P4 showed none. A scope check was dropped as hard to capture. The PARLIO setup follows the IDF sequence and every C6 candidate clock divides its source exactly, so clock rounding is not the cause. Cause unknown.
4. The `DebugLevel=verbose` check of the `LiteLED version:` line was skipped for 3.3.0.

## PARLIO Peripheral Manager bus type

arduino-esp32 has no `ESP32_BUS_TYPE_PARLIO_TX` (checked through 3.3.12 and `master`), so PARLIO pins are registered as `ESP32_BUS_TYPE_GPIO` with no deinit callback. Requested upstream: https://github.com/espressif/arduino-esp32/issues/13130. PR adding the type: https://github.com/espressif/arduino-esp32/pull/13131 (opened 2026-11-17; the maintainers said it will not be in 3.3.13 or 4.0.0). The PR adds TX and RX types and, at the maintainers' request, no detection macro: use the version check below.

When a core ships the type: the `#ifdef ESP32_BUS_TYPE_PARLIO_TX` tests in `ll_parlio_core.h`, `LiteLEDpio.cpp`, `LiteLEDpioGroup.cpp` and `ll_parlio_core.cpp` can never be true (bus types are enum constants), so replace them with an `ESP_ARDUINO_VERSION_VAL()` check against the first release that has it, then test the dedicated path on a PARLIO chip.

## WS2814 (WRGB colour order)

The WS2814A/B/F send the white channel first (WRGB). `color_order_t` has only the six RGB permutations and the library always sends W last, so these parts can't be driven correctly. Add an `ORDER_WRGB` value (valid only with `rgbw = true`) with W-first cases in `led_strip_set_pixel()` and `led_strip_get_pixel()`, the `col_ord[]` table, the PARLIO path and `keywords.txt`. Not supported in v3.3.0; planned for a later release.

## Colour order is not per strip

`led_strip_set_color_order()` and `led_strip_set_default_color_order()` set `custom_color_order` and `use_custom_color_order` without using the strip, so they appear to apply to every strip rather than the one `setOrder()` was called on. Not yet confirmed. Check, and either fix or document.
