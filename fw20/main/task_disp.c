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

/* ======================================================================
 * DEMO - dummy scale level, TO BE REMOVED once real keg data drives the
 * gauge (scale_get() -> keg_left / volume).
 *
 * s_demo_pct stands in for "keg fullness in %". Swipe up animates it to
 * 100, swipe down to 0; a full 0..100 run takes DEMO_FULL_MS, shorter
 * moves take proportionally less. Every step pushes the value to the
 * gauge of the current screen (ui_set_level), and show_screen() applies
 * it after a screen rebuild so the level survives screen switching.
 * Removal: delete this block, the two SWIPE_UP/DOWN cases in on_input()
 * and the ui_set_level(s_demo_pct) call in show_screen().
 * ==================================================================== */
#define DEMO_FULL_MS    3000

static int32_t s_demo_pct = 100;

static void demo_anim_cb(void *var, int32_t v)
{
    s_demo_pct = v;
    ui_set_level(v);
}

static void demo_level_to(int32_t target)
{
    lv_anim_delete(&s_demo_pct, demo_anim_cb);
    int32_t dist = LV_ABS(target - s_demo_pct);
    if (dist == 0)
        return;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, &s_demo_pct);
    lv_anim_set_exec_cb(&a, demo_anim_cb);
    lv_anim_set_values(&a, s_demo_pct, target);
    lv_anim_set_duration(&a, DEMO_FULL_MS * dist / 100);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}
/* ============================ end DEMO ============================== */

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
    ui_set_level(s_demo_pct);   /* DEMO */
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
    case UI_SWIPE_UP:           /* DEMO */
        demo_level_to(100);
        break;
    case UI_SWIPE_DOWN:         /* DEMO */
        demo_level_to(0);
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
