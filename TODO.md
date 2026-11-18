# TODO

# v3.3.1 plan

**Rule (decided 2026-11-17):** never change the timing an existing LED type sends. If a datasheet shows a significant deviation, add a new type with the datasheet timing and `R2` appended to the name (e.g. `LED_STRIP_APA106R2`), and document which type suits which part. `R` = LiteLED's own revision; `V` in a name (`LED_STRIP_WS281XV7`) always means an OEM datasheet version (decided 2026-11-17: `R2`, not `V2`, to avoid mixing the two).

## Status (2026-11-17, branch `fix/v3.3.1`)

Done, loopback-verified on an XIAO ESP32-C6 (details in `custom_timing_test/SESSION_NOTES.md`):

- `LED_STRIP_SK6812` has its own constants and sends the v3.2.0 timing again (300/900/900/300 ns, 210 us). The datasheet (Shenzhen Normand Rev. 06) limits are met; v3.3.0 had changed it to a 1-bit of 600/400 ns and a 90 us reset (below the 1.2 us minimum bit period). The 3.3.1 release notes must say so.
- New `LED_STRIP_APA106R2` (350/1360/1360/350 ns, 50 us): RMT 350/1350/1350/350, PARLIO 5 MHz 400/1400/1400/400. `LED_STRIP_APA106` is unchanged (its PARLIO timing is out of the datasheet window).
- SM16703 (2024-02-28 datasheet) is within every limit on RMT and PARLIO; no change. WS2812 and WS2812_RGB stay as they are (decided); newer WS2812B V6/V7 want reset > 280 us, so they should use `LED_STRIP_WS281XV7` (documented).
- "Choosing an LED type" table in `Using LiteLED.md`.
- Peripheral Manager deinit leak: after a forced reassignment the RMT channel / PARLIO unit stayed allocated. `begin()`, `free()` and the destructor now release it, and `free()` clears the pin only if it is still ours (RMT: bus handle; PARLIO: `ll_parlio_pin_owned()`). Verified on the C6: RMT 0 failures (v3.3.0 fails from round 3, `no free tx channels`; this also explained the `intr_priority conflict` log); PARLIO 0 failures on the dedicated bus type (patched core) and no new failures on the stock core.
- `LiteLEDpio::getActiveInstanceCount()` now counts valid strips and groups (was a stub returning 0).

Still to do before 3.3.1:

1. Version bump (`LITELED_VERSION_*`, `library.properties`, README / docs history headings).
2. Release notes: SK6812 restore, `LED_STRIP_APA106R2`, leak and counter fixes.
3. Optional: RMT loopback of the new cases on the PICO-D4 and S3; real SK6812 / APA106 strips; an APA106R2 example.
4. PR `fix/v3.3.1` to `dev`, then release per `WORKFLOW.md`.

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
