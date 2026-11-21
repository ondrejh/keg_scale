#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_app_desc.h"
#include "onewire_bus.h"
#include "ds18b20.h"
#include "board.h"
#include "hx711.h"
#include "scale.h"
#include "tasks.h"

static const char *TAG = "scale";

#define POLL_MS         20      /* HX711 ready poll (10 SPS -> every 100 ms) */
#define UPDATE_MS       1000    /* scale_update() + DS18B20 step period */
#define LOG_MS          5000    /* periodic reading log */
#define HX_RING         16      /* raw = sum of the last HX_RING readings (as the sketch) */
#define HX_MISSING_MS   1000    /* no valid reading this long = sensor missing */
#define HX_SETTLE       4       /* readings dropped after reset / recovery (400 ms at 10 SPS) */

static void print_info(void)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    uint32_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);

    ESP_LOGI(TAG, "%s %s, IDF %s", esp_app_get_description()->project_name,
             esp_app_get_description()->version, esp_get_idf_version());
    ESP_LOGI(TAG, "chip %s rev %d.%d, %d core(s), flash %lu MB",
             CONFIG_IDF_TARGET, chip.revision / 100, chip.revision % 100,
             chip.cores, (unsigned long)(flash_size / (1024 * 1024)));
}

/* ---- HX711: moving sum of the last HX_RING readings ---- */

static int32_t s_ring[HX_RING];
static int s_ring_pos, s_ring_cnt;
static int64_t s_ring_sum;

static void ring_push(int32_t v)
{
    if (s_ring_cnt == HX_RING)
        s_ring_sum -= s_ring[s_ring_pos];
    else
        s_ring_cnt++;
    s_ring[s_ring_pos] = v;
    s_ring_sum += v;
    s_ring_pos = (s_ring_pos + 1) % HX_RING;
}

/* Sum scaled to HX_RING readings, so it is right before the ring fills. */
static int32_t ring_raw(void)
{
    return s_ring_cnt ? (int32_t)(s_ring_sum * HX_RING / s_ring_cnt) : 0;
}

/* ---- DS18B20: trigger on one step, read on the next (12 bit = 750 ms) ---- */

static onewire_bus_handle_t s_bus;
static ds18b20_device_handle_t s_ds;
static bool s_temp_valid;
static float s_temp;

static void temp_init(void)
{
    onewire_bus_config_t bus_cfg = {
        .bus_gpio_num = BOARD_DS18B20_DQ,
        .flags.en_pull_up = 1,      /* external 4k7 does the real work */
    };
    onewire_bus_rmt_config_t rmt_cfg = {
        .max_rx_bytes = 10,         /* 9 byte scratchpad */
    };
    esp_err_t err = onewire_new_bus_rmt(&bus_cfg, &rmt_cfg, &s_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "1-wire bus init failed (%s)", esp_err_to_name(err));
        return;
    }
    ds18b20_config_t ds_cfg = {};
    ESP_ERROR_CHECK(ds18b20_new_device_from_bus(s_bus, &ds_cfg, &s_ds)); /* single sensor, skip ROM */
}

/* Convert T to all devices without the driver's blocking 800 ms wait. */
static esp_err_t temp_trigger(void)
{
    esp_err_t err = onewire_bus_reset(s_bus);
    if (err != ESP_OK)
        return err;
    const uint8_t cmd[] = {0xCC, 0x44};     /* skip ROM, convert T */
    return onewire_bus_write_bytes(s_bus, cmd, sizeof(cmd));
}

static void temp_step(void)
{
    static bool converting;
    static bool reported_missing;

    if (!s_ds)
        return;

    esp_err_t err;
    if (!converting) {
        err = temp_trigger();
        converting = (err == ESP_OK);
    } else {
        float t;
        converting = false;
        err = ds18b20_get_temperature(s_ds, &t);
        if (err == ESP_OK) {
            if (!s_temp_valid)
                ESP_LOGI(TAG, "DS18B20 ok, %.2f C", t);
            s_temp = t;
            s_temp_valid = true;
            reported_missing = false;
            return;
        }
    }
    if (err != ESP_OK) {
        s_temp_valid = false;
        if (!reported_missing)
            ESP_LOGW(TAG, "DS18B20 not responding (%s)", esp_err_to_name(err));
        reported_missing = true;
    }
}

void task_scale(void *arg)
{
    print_info();

    hx711_init(BOARD_HX711_DOUT, BOARD_HX711_SCK);
    temp_init();

    TickType_t last_ok = xTaskGetTickCount();
    TickType_t last_update = last_ok, last_log = last_ok;
    bool hx_missing = false;
    int hx_settle = HX_SETTLE;
    int32_t last_val = 0;

    while (1) {
        TickType_t now = xTaskGetTickCount();

        if (hx711_ready()) {
            int32_t v;
            if (hx711_read(&v)) {
                if (hx_missing) {
                    ESP_LOGI(TAG, "HX711 back");
                    hx_missing = false;
                    s_ring_cnt = s_ring_pos = 0;    /* don't mix with stale readings */
                    s_ring_sum = 0;
                    hx_settle = HX_SETTLE;
                }
                last_ok = now;
                if (hx_settle > 0)
                    hx_settle--;        /* first conversions after reset are not valid */
                else {
                    ring_push(v);
                    last_val = v;
                }
            }
        }
        if (!hx_missing && now - last_ok > pdMS_TO_TICKS(HX_MISSING_MS)) {
            ESP_LOGW(TAG, "HX711 missing (no valid reading)");
            hx_missing = true;
        }

        if (now - last_update >= pdMS_TO_TICKS(UPDATE_MS)) {
            last_update = now;
            temp_step();
            scale_data_t d;
            scale_get(&d);
            /* sensor missing: keep the last weight, update temperature only */
            int32_t raw = (hx_missing || !s_ring_cnt) ? d.raw : ring_raw();
            scale_update(raw, s_temp_valid, s_temp);
        }

        if (now - last_log >= pdMS_TO_TICKS(LOG_MS)) {
            last_log = now;
            scale_data_t d;
            scale_get(&d);
            ESP_LOGI(TAG, "hx %ld raw %ld (%d) %.2f %s, temp %s%.2f",
                     (long)last_val, (long)d.raw, s_ring_cnt, d.units, d.calib.uprim,
                     d.temp_valid ? "" : "invalid ", d.temp);
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    }
}
