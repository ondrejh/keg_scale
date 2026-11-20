#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "board.h"
#include "config.h"
#include "tasks.h"
#include "scale.h"

static const char *TAG = "config";

static void board_power_hold(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BOARD_SYS_POWER_HOLD,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io);
    gpio_set_level(BOARD_SYS_POWER_HOLD, 1);
}

void app_main(void)
{
    board_power_hold();

    esp_err_t nerr = nvs_flash_init();
    if (nerr == ESP_ERR_NVS_NO_FREE_PAGES || nerr == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nerr = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nerr);

    esp_err_t err = config_load();
    if (err == ESP_OK)
        ESP_LOGI(TAG, "ssid \"%s\"", g_net_config.ssid);
    else
        ESP_LOGE(TAG, "load failed (%s)", esp_err_to_name(err));

    scale_init();

    BaseType_t ok;
    ok = xTaskCreate(task_disp,  "task_disp",  TASK_DISP_STACK,  NULL, TASK_DISP_PRIO,  NULL);
    configASSERT(ok == pdPASS);
    ok = xTaskCreate(task_scale, "task_scale", TASK_SCALE_STACK, NULL, TASK_SCALE_PRIO, NULL);
    configASSERT(ok == pdPASS);
    ok = xTaskCreate(task_net,   "task_net",   TASK_NET_STACK,   NULL, TASK_NET_PRIO,   NULL);
    configASSERT(ok == pdPASS);
}
