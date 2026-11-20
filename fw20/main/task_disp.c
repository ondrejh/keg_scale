#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "tasks.h"

static const char *TAG = "disp";

void task_disp(void *arg)
{
    ESP_LOGI(TAG, "started");

    while (1) {
        /* TODO: display / LVGL refresh */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
