#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_app_desc.h"
#include "esp_random.h"
#include "scale.h"
#include "tasks.h"

static const char *TAG = "scale";

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

/* ======================================================================
 * SIMULATION - no load cell yet. TO BE REMOVED with the HX711 driver
 * (also the scale_sim_* declarations in scale.h and their callers).
 *
 * s_sim_kg is the weight on the scale. Without a keg it is an empty keg.
 * scale_sim_keg_full() puts a full 15 l keg (30 beers) on and taps it;
 * while tapped, one beer is poured every SIM_DRAIN_S seconds until empty.
 * scale_sim_keg_off() untaps it and leaves the empty keg on the scale.
 * Readings go through the real path: kg -> raw (inverse calibration,
 * plus noise) -> scale_update() -> units, keg_left.
 * ==================================================================== */
#define SIM_KEG_NAME    "Prim\u00e1tor 13\u00b0 polotmav\u00fd"
#define SIM_KEG_VOLUME  30.0f       /* beers (0.5 l) = 15 l keg */
#define SIM_EMPTY_KG    5.0f
#define SIM_DRAIN_S     2
#define SIM_NOISE       300         /* +- raw counts */

static volatile float s_sim_kg = SIM_EMPTY_KG;

static int32_t sim_raw(void)
{
    scale_data_t d;
    scale_get(&d);
    return scale_units_to_raw(&d.calib, s_sim_kg)
         + (int32_t)(esp_random() % (2 * SIM_NOISE + 1)) - SIM_NOISE;
}

static float sim_beer_kg(void)
{
    scale_data_t d;
    scale_get(&d);
    return d.calib.ratio;
}

void scale_sim_keg_load(float volume)
{
    s_sim_kg = SIM_EMPTY_KG + volume * sim_beer_kg();
    scale_update(sim_raw(), false, 0.0f);       /* tap after this: fullraw = full */
}

void scale_sim_keg_full(void)
{
    scale_sim_keg_load(SIM_KEG_VOLUME);
    scale_keg_tap(SIM_KEG_NAME, SIM_KEG_VOLUME);
    ESP_LOGI(TAG, "sim: full keg tapped (%.1f kg)", s_sim_kg);
}

void scale_sim_keg_off(void)
{
    scale_keg_untap();
    s_sim_kg = SIM_EMPTY_KG;
    scale_update(sim_raw(), false, 0.0f);
    ESP_LOGI(TAG, "sim: keg untapped");
}

static void sim_step(void)
{
    static int tick;
    scale_data_t d;
    scale_get(&d);
    if (d.keg.tapped && ++tick >= SIM_DRAIN_S) {
        tick = 0;
        float kg = s_sim_kg - d.calib.ratio;    /* one beer poured */
        s_sim_kg = kg > SIM_EMPTY_KG ? kg : SIM_EMPTY_KG;
    }
    scale_update(sim_raw(), false, 0.0f);
}
/* ============================ end SIMULATION ======================== */

void task_scale(void *arg)
{
    print_info();

    while (1) {
        sim_step();     /* TODO: HX711 reading instead */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
