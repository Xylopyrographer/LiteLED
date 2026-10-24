# TODO

## Fix SK6812 timing definition (RMT)

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
