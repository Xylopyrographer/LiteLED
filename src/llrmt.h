
/*
 * @file llrmt.h
 *
 * RMT-based ESP-IDF driver for WS2812B/SK6812/APA106/SM16703 LED strips
 *  - Copyright (c) 2020 Ruslan V. Uss <unclerus@gmail.com>
 *  - Copyright (c) 2023 Xylopyrographer <xylopyrographer@gmail.com>
 *  - MIT Licensed as described in the file LICENSE
 */

#include <esp_log.h>
#include <esp_attr.h>

#ifndef RMT_DEFAULT_CONFIG_TX
#define RMT_DEFAULT_CONFIG_TX(gpio, channel_id)      \
    {                                                \
        .rmt_mode = RMT_MODE_TX,                     \
        .channel = channel_id,                       \
        .gpio_num = gpio,                            \
        .clk_div = 80,                               \
        .mem_block_num = 1,                          \
        .tx_config = {                               \
            .carrier_freq_hz = 38000,                \
            .carrier_level = RMT_CARRIER_LEVEL_HIGH, \
            .idle_level = RMT_IDLE_LEVEL_LOW,        \
            .carrier_duty_percent = 33,              \
            .carrier_en = false,                     \
            .loop_en = false,                        \
            .idle_output_en = true,                  \
        }                                            \
    }
#endif

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)
#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

// #define COLOR_SIZE(strip) ( 3 + ( (strip)->is_rgbw != 0 ) )
#define COLOR_SIZE(strip) ( 3 + ( (strip)->is_rgbw != 0 ) )

#define LED_STRIP_RMT_CLK_DIV 2

static void IRAM_ATTR _rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
                                   size_t wanted_num, size_t *translated_size, size_t *item_num,
                                   const rmt_item32_t *bit0, const rmt_item32_t *bit1) 
{
    if (!src || !dest)
    {
        *translated_size = 0;
        *item_num = 0;
        return;
    }
    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t *pdest = dest;
    #ifdef LED_STRIP_BRIGHTNESS
        led_strip_t *strip;
        esp_err_t r = rmt_translator_get_context(item_num, (void **)&strip);
        uint8_t brightness = r == ESP_OK ? strip->brightness : 255;
    #endif
    while (size < src_size && num < wanted_num)
    {
        #ifdef LED_STRIP_BRIGHTNESS
            uint8_t b = brightness != 255 ? scale8_video(*psrc, brightness) : *psrc;
        #else
            uint8_t b = *psrc;
        #endif
        for (int i = 0; i < 8; i++)
        {
            // MSB first
            pdest->val = b & (1 << (7 - i)) ? bit1->val : bit0->val;
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}

typedef struct {
    rmt_item32_t bit0, bit1;
} led_rmt_t;

static led_rmt_t rmt_items[LED_STRIP_TYPE_MAX] = { 0 };

static void IRAM_ATTR ws2812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
        size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    _rmt_adapter(src, dest, src_size, wanted_num, translated_size, item_num, &rmt_items[LED_STRIP_WS2812].bit0, &rmt_items[LED_STRIP_WS2812].bit1);
}

static void IRAM_ATTR sk6812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
        size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    _rmt_adapter(src, dest, src_size, wanted_num, translated_size, item_num, &rmt_items[LED_STRIP_SK6812].bit0, &rmt_items[LED_STRIP_SK6812].bit1);
}

static void IRAM_ATTR apa106_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
        size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    _rmt_adapter(src, dest, src_size, wanted_num, translated_size, item_num, &rmt_items[LED_STRIP_APA106].bit0, &rmt_items[LED_STRIP_APA106].bit1);
}

static void IRAM_ATTR sm16703_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
                                         size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    _rmt_adapter(src, dest, src_size, wanted_num, translated_size, item_num, &rmt_items[LED_STRIP_SM16703].bit0, &rmt_items[LED_STRIP_SM16703].bit1);
}

typedef enum {
    ORDER_GRB,
    ORDER_RGB,
} color_order_t;

typedef struct {
    uint32_t t0h, t0l, t1h, t1l;
    color_order_t order;
    sample_to_rmt_t adapter;
} led_params_t;

static const led_params_t led_params[] = {
    [LED_STRIP_WS2812]  = { .t0h = 400, .t0l = 1000, .t1h = 1000, .t1l = 400, .order = ORDER_GRB, .adapter = ws2812_rmt_adapter },
    [LED_STRIP_SK6812]  = { .t0h = 300, .t0l = 900,  .t1h = 600,  .t1l = 600, .order = ORDER_GRB, .adapter = sk6812_rmt_adapter },
    [LED_STRIP_APA106]  = { .t0h = 350, .t0l = 1360, .t1h = 1360, .t1l = 350, .order = ORDER_RGB, .adapter = apa106_rmt_adapter },
    [LED_STRIP_SM16703] = { .t0h = 300, .t0l = 900,  .t1h = 1360, .t1l = 350, .order = ORDER_RGB, .adapter = sm16703_rmt_adapter },
};

