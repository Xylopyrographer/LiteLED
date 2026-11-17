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

## Define custom timing parameter limits

Add to the docs valid ranges for each of the parameters used in the custom timing structure.

## Verify LED_STRIP_WS281XV7 on hardware (before the v3.3.0 release)

`LED_STRIP_WS281XV7` (T0 325/925 ns, T1 625/625 ns, reset 400 µs). 3.2 MHz was added to `ll_pio_candidate_clk_hz` (the preset is sent as 4 samples per bit, 32 DMA bytes per colour byte).

Done on the XIAO ESP32-C6 (RMT RX loopback, 2026-11-16), results in *Verified Results* in `Using LiteLED.md`: RMT, PARLIO and a `LiteLEDpioGroup` lane all measure as expected, and the same values sent through the custom timing constructor match the built-in type.

Still to do before the v3.3.0 release:

1. **Test PARLIO on an ESP32-P4 (required before the GitHub release). WS281XV7 part DONE 2026-11-16** (Guition JC4880P443, P4 rev v3.2, loopback GPIO51 to GPIO52): RMT 325/925/625/625; PARLIO single, group lane and custom constructor (tol 15 and 25) 312.5/937.5/625/625 at 3.2 MHz; group built-in WS2812 400/800; reset seen 416 µs (RMT) and 424 µs (PARLIO). The full rig also ran on the P4 (all cases, including 2.5, 3.2, 10 and 40 MHz PARLIO, the failure cases and the built-in types): every edge exact, no one-tick deviations. Original item: The 3.2 MHz clock was only tried on the C6. Confirm that the P4 produces 3.2 MHz exactly (expect 312.5/937.5/625/625 ns for `LED_STRIP_WS281XV7`, in `LiteLEDpio` and `LiteLEDpioGroup`), and that the other clocks in `ll_pio_candidate_clk_hz` still match their requests. There is no ESP32-H2 available to test; say so in the release notes if H2 is claimed as supported.
2. RMT loopback is done on the C6, ESP32-PICO-D4 and ESP32-S3 (2026-11-16, 325/925/625/625 exact; reset seen 436.5 µs on the PICO-D4, 428.9 µs on the S3; custom constructor identical). Visual check on the ATOM Matrix's 25 GRB LEDs (GPIO 27) passed: red, green, blue, white, per-row colours and the custom constructor all correct.
3. Run on more real WS281x strips and check that colours and latching are correct (the ATOM Matrix 25-LED GRB matrix is done).
4. ~~Scope check of the PARLIO edge jitter~~ Dropped: it is a fleeting effect that would be hard to capture, and the P4 showed none. Seen only on the C6 (built-in 2.5 MHz types about 391/809 ns instead of 400/800, the 40 MHz custom case about 316/933 instead of 325/925; bit period unchanged; the `LED_STRIP_WS281XV7` values were exact in every run). The PARLIO setup follows the IDF sequence (new TX unit, enable, `idle_value = 0` on every transmit); the C6 default clock is PLL_F240M with an integer divider, and every candidate clock divides it exactly, so clock rounding is not the cause.
5. ESP32-H2 is not tested. Its default PARLIO source is PLL_F96M with an integer divider only, so 2.5, 5 and 20 MHz do not divide exactly (about 2.53, 5.26 and 19.2 MHz). Say so in the release notes if H2 is claimed. The driver reports a precision loss in `parlio_tx.c`; the library does not check it.

## WS2814 (WRGB colour order)

The WS2814A/B/F send the white channel first (WRGB). `color_order_t` has only the six RGB permutations and the library always sends W last, so these parts can't be driven correctly. Add an `ORDER_WRGB` value (valid only with `rgbw = true`) with W-first cases in `led_strip_set_pixel()` and `led_strip_get_pixel()`, the `col_ord[]` table, the PARLIO path and `keywords.txt`. Planned for a release after v3.3.0.

## Colour order is not per strip

`led_strip_set_color_order()` and `led_strip_set_default_color_order()` set `custom_color_order` and `use_custom_color_order` without using the strip, so they appear to apply to every strip rather than the one `setOrder()` was called on. Not yet confirmed. Check, and either fix or document.
