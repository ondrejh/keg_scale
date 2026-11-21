#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "hx711.h"

static const char *TAG = "hx711";

/* SCK may carry a large capacitance (GPIO9 has 100 nF on the board), so each
 * edge is waited for by reading the pin back, up to this long. */
#define EDGE_TIMEOUT_US     40

static gpio_num_t s_dout = GPIO_NUM_NC;
static gpio_num_t s_sck = GPIO_NUM_NC;
static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;

/* Drive SCK and wait until the pin really is at that level. */
static bool sck_set(int level)
{
    gpio_set_level(s_sck, level);
    for (int i = 0; i < EDGE_TIMEOUT_US; i++) {
        if (gpio_get_level(s_sck) == level)
            return true;
        esp_rom_delay_us(1);
    }
    return gpio_get_level(s_sck) == level;
}

void hx711_init(gpio_num_t dout, gpio_num_t sck)
{
    s_dout = dout;
    s_sck = sck;

    gpio_config_t io = {
        .pin_bit_mask = 1ULL << sck,
        .mode = GPIO_MODE_INPUT_OUTPUT,     /* read back for the edge wait */
    };
    gpio_config(&io);
    /* DOUT: external 10k/20k divider is the pull-down, no internal pulls */
    io.pin_bit_mask = 1ULL << dout;
    io.mode = GPIO_MODE_INPUT;
    gpio_config(&io);

    /* reset: power down (SCK high > 60 us) and back up; measure the edge */
    int64_t t0 = esp_timer_get_time();
    bool ok = sck_set(1);
    int rise = (int)(esp_timer_get_time() - t0);
    esp_rom_delay_us(100);
    t0 = esp_timer_get_time();
    ok = sck_set(0) && ok;
    int fall = (int)(esp_timer_get_time() - t0);
    if (ok)
        ESP_LOGI(TAG, "SCK edges: rise %d us, fall %d us", rise, fall);
    else
        ESP_LOGE(TAG, "SCK edge timeout (rise %d us, fall %d us) - shorted / BOOT pressed?", rise, fall);
}

bool hx711_ready(void)
{
    return gpio_get_level(s_dout) == 0;
}

/* One SCK pulse; returns DOUT sampled just before the falling edge. Only the
 * high phase (must stay < 50 us, > 60 us = power down) runs with interrupts
 * off; the low phase has no upper limit. */
static bool sck_pulse(uint32_t *bit)
{
    bool ok;
    portENTER_CRITICAL(&s_mux);
    ok = sck_set(1);
    esp_rom_delay_us(1);
    *bit = (uint32_t)gpio_get_level(s_dout);
    ok = sck_set(0) && ok;
    portEXIT_CRITICAL(&s_mux);
    return ok;
}

bool hx711_read(int32_t *out)
{
    uint32_t v = 0, bit;

    for (int i = 0; i < 24; i++) {
        if (!sck_pulse(&bit))
            return false;
        v = (v << 1) | bit;
    }
    /* 25th pulse: next conversion channel A, gain 128. It also forces DOUT
     * high until the next conversion - still low means nothing clocked out
     * (HX711 unpowered / unplugged: the divider holds DOUT low). */
    if (!sck_pulse(&bit))
        return false;
    esp_rom_delay_us(2);
    if (!gpio_get_level(s_dout))
        return false;

    if (v & 0x800000)
        v |= 0xFF000000;        /* sign extend 24 -> 32 bit */
    *out = (int32_t)v;
    return true;
}
