#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "display.h"
#include "net.h"
#include "settings.h"
#include "ui.h"
#include "tasks.h"

static const char *TAG = "disp";

/* All LVGL calls must stay in this task (LVGL is not thread safe). */
void task_disp(void *arg)
{
    ESP_ERROR_CHECK(display_init());
    ui_show_hello();
    ESP_LOGI(TAG, "started");

    net_mode_t shown = NET_MODE_NONE;

    while (1) {
        net_mode_t mode = net_get_mode();
        if (mode != shown) {
            shown = mode;
            if (mode == NET_MODE_AP)
                ui_show_ap(g_dev_id, AP_PASSWORD, AP_IP);
        }

        uint32_t ms = lv_timer_handler();
        if (ms < 5)
            ms = 5;
        else if (ms > 100)
            ms = 100;
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
}
