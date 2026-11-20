#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "display.h"
#include "net.h"
#include "settings.h"
#include "ui.h"
#include "ui_input.h"
#include "tasks.h"

static const char *TAG = "disp";

enum { SCR_MAIN, SCR_WEBI, SCR_COUNT };

static int s_screen = SCR_MAIN;

static void show_screen(void)
{
    char ip[16];
    switch (s_screen) {
    case SCR_MAIN:
        if (net_get_mode() == NET_MODE_AP)
            ui_show_ap(g_dev_id, AP_PASSWORD, AP_IP);
        else
            ui_show_hello();
        break;
    case SCR_WEBI:
        ui_show_webi(net_get_ip(ip, sizeof(ip)) ? ip : NULL);
        break;
    }
}

static void on_input(ui_input_t ev)
{
    ESP_LOGI(TAG, "%s", ui_input_name(ev));
    switch (ev) {
    case UI_SWIPE_LEFT:
        s_screen = (s_screen + 1) % SCR_COUNT;
        show_screen();
        break;
    case UI_SWIPE_RIGHT:
        s_screen = (s_screen + SCR_COUNT - 1) % SCR_COUNT;
        show_screen();
        break;
    default:    /* taps: reserved for later */
        break;
    }
}

/* All LVGL calls must stay in this task (LVGL is not thread safe). */
void task_disp(void *arg)
{
    ESP_ERROR_CHECK(display_init());
    ui_input_init(display_get_indev(), on_input);
    show_screen();
    ESP_LOGI(TAG, "started");

    net_mode_t shown = NET_MODE_NONE;

    while (1) {
        net_mode_t mode = net_get_mode();
        if (mode != shown) {
            shown = mode;
            show_screen();      /* AP QR / web URL depend on network state */
        }

        uint32_t ms = lv_timer_handler();
        if (ms < 5)
            ms = 5;
        else if (ms > 100)
            ms = 100;
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
}
