#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_app_desc.h"
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

void task_scale(void *arg)
{
    print_info();

    uint32_t n = 0;
    while (1) {
        ESP_LOGI(TAG, "heartbeat %lu", (unsigned long)n++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
