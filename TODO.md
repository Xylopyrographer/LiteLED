# TODO

# v3.3.1 plan

**Rule (decided 2026-11-17):** never change the timing an existing LED type sends. If a datasheet shows a significant deviation, add a new type with the datasheet timing and `R2` appended to the name (e.g. `LED_STRIP_APA106R2`), and document which type suits which part. `R` = LiteLED's own revision; `V` in a name (`LED_STRIP_WS281XV7`) always means an OEM datasheet version (decided 2026-11-17: `R2`, not `V2`, to avoid mixing the two).

## SK6812: restore the v3.2.0 timing, with its own constants

Audit result, 2026-11-17 (Shenzhen Normand SK6812 datasheet Rev. 06, page 6, `Discussions/OtherDataSheets/SK6812 LED  Datasheet.pdf`): code period min 1.20 us; T0H 0.2 / 0.32 / 0.4; T0L min 0.8; T1H 0.58 / 0.64 / 1.0; T1L min 0.2; reset > 80 us.

v3.2.0 sent T0H 300, T0L 900, T1H 900, T1L 300 ns and a 210 us reset (the 1-bit and reset were borrowed from SM16703 and the SK6812 constants were unused). That is within every limit. `d0e542e` (in v3.3.0) changed it to a 1-bit of 600/400 ns (period 1000 ns, below the 1200 ns minimum; T1H margin 20 ns) and a 90 us reset (margin 10 us). That broke backwards compatibility and is worse against the datasheet.

Fix (`src/ll_led_timings.h`):

1. Give `led_sk6812` its own constants that send exactly what v3.2.0 sent (300/900, 900/300, 210 us), compliant with the datasheet and not borrowed from any other LED type. Remove or correct the unused `sk6812_one` / `sk6812_reset`.
2. Verify with the RX loopback rig: RMT output must be identical to v3.2.0 (build v3.2.0 and compare).
3. Release notes (3.3.1): say v3.3.0 changed SK6812 timing and 3.3.1 restores the v3.2.0 timing.
4. Test on real SK6812 (RGB and RGBW) strips if available.

Note for the docs: `LED_STRIP_WS281XV7` timing (325/925/625/625, 400 us) also lies inside the SK6812 windows.

## New type LED_STRIP_APA106R2

APA106 datasheet (2014, `Discussions/OtherDataSheets/APA-106 F8 F5 LED 2014.pdf`): 0.35 / 1.36 / 1.36 / 0.35 us, each +/-150 ns, reset 50 us. The RMT output of `LED_STRIP_APA106` (300/1300/1300/300 ns) is within tolerance. The PARLIO output (shared 2.5 MHz timing, 400/800/800/400 ns) is not: T0L and T1H are about 400 ns below the 1210 ns minimum (noted in a code comment since v3.1.0).

Add `LED_STRIP_APA106R2` with 350/1360/1360/350 ns, reset 50 us, ORDER_RGB, built from a preset like `LED_STRIP_WS281XV7`. Leave `LED_STRIP_APA106` unchanged. Check PARLIO picks a clock that meets the windows, add to the docs (`Using LiteLED.md`, README, Architecture, `keywords.txt`), add an example or extend one, and loopback-verify RMT and PARLIO.

## SM16703 and WS2812

SM16703 (SM16703AP datasheet, 2024-02-28) is within every limit on RMT and PARLIO (RMT reset 210 us against a 200 us minimum; PARLIO T0H 400 ns sits on the 400 ns maximum). No change.

WS2812 and WS2812_RGB stay as they are (decided 2026-11-17). Per `Discussions/YInWorldSemi/WS281X_Datasheets/TIMING_SUMMARY.md`, newer WS2812B parts (V6, V7) want a reset above 280 us while the RMT reset is 50 us; document that those parts should use `LED_STRIP_WS281XV7`.

## Document which type suits which part

In `Using LiteLED.md` / README: a table of LED parts and the LED type to use (WS2812, WS2812_RGB, SK6812 / WS281XV7, APA106 / APA106R2, SM16703, WS281XV7 for the WS281x family, custom timing otherwise).

## Remaining audit

Built-in types are checked against OEM datasheets: SM16703, SK6812 and APA106 done 2026-11-17. WS2812 kept as is. Keep new sheets in `Discussions/OtherDataSheets/`.

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
