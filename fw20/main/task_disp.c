#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "display.h"
#include "tasks.h"

static const char *TAG = "disp";

/* All LVGL calls must stay in this task (LVGL is not thread safe). */
void task_disp(void *arg)
{
    ESP_ERROR_CHECK(display_init());

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Hello world");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    lv_obj_center(label);

    ESP_LOGI(TAG, "started");

    while (1) {
        uint32_t ms = lv_timer_handler();
        if (ms < 5)
            ms = 5;
        else if (ms > 100)
            ms = 100;
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
}
