#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "tasks.h"

static const char *TAG = "net";

void task_net(void *arg)
{
    ESP_LOGI(TAG, "started");

    while (1) {
        /* TODO: Wi-Fi / MQTT */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
