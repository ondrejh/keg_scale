#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "display.h"
#include "net.h"
#include "settings.h"
#include "config.h"
#include "ui.h"
#include "ui_input.h"
#include "scale.h"
#include "tasks.h"

static const char *TAG = "disp";

enum { SCR_MAIN, SCR_WEBI, SCR_COUNT };

static int s_screen = SCR_MAIN;

#define REFRESH_MS  500

/* Gauge level while a keg is tapped: keg 0..100 % maps to GAUGE_TAPPED_MIN..100,
 * so some beer stays visible below the keg panel until the keg is untapped
 * (20 % = clear band at the bottom of the round screen). No keg = foam only. */
#define GAUGE_TAPPED_MIN    20

static int gauge_level(const scale_data_t *d)
{
    if (!d->keg.tapped)
        return 0;
    return GAUGE_TAPPED_MIN + scale_keg_pct(d) * (100 - GAUGE_TAPPED_MIN) / 100;
}

/* Push current scale data into the visible screen. */
static void refresh_values(void)
{
    scale_data_t d;
    scale_get(&d);
    ui_keg_update(&d);
    ui_set_level(gauge_level(&d));
}

static void show_screen(void)
{
    char ip[16];
    switch (s_screen) {
    case SCR_MAIN:
        if (net_get_mode() == NET_MODE_AP)
            ui_show_ap(g_dev_id, AP_PASSWORD, AP_IP);
        else
            ui_show_keg();
        break;
    case SCR_WEBI:
        ui_show_webi(net_get_ip(ip, sizeof(ip)) ? ip : NULL);
        break;
    }
    refresh_values();
}

/* ---- idle dimming: fade down after IDLE_DIM_MS without touch ---- */
/* BRIGHT_ON: settings.h; timeout and dim level: g_disp_config (web config page,
 * defaults IDLE_DIM_MS / BRIGHT_DIM from settings.h) */
#define FADE_OUT_MS     1500
#define FADE_IN_MS      200
#define WAKE_IGNORE_MS  500     /* the waking touch must not swipe/tap too */

static int32_t s_bright = BRIGHT_ON;
static int32_t s_bright_target = BRIGHT_ON;
static bool s_dimmed;
static uint32_t s_wake_ms;

static void bright_anim_cb(void *var, int32_t v)
{
    if (v != s_bright) {
        s_bright = v;
        display_set_brightness((uint8_t)v);
    }
}

static void fade_to(int32_t level, uint32_t ms)
{
    s_bright_target = level;
    lv_anim_delete(&s_bright, bright_anim_cb);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, &s_bright);
    lv_anim_set_exec_cb(&a, bright_anim_cb);
    lv_anim_set_values(&a, s_bright, level);
    lv_anim_set_duration(&a, ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

static void idle_check(void)
{
    uint32_t idle = lv_display_get_inactive_time(NULL);
    uint32_t timeout = g_disp_config.idle_ms;   /* may change from the web */
    bool want_dim = timeout && idle >= timeout;

    if (!s_dimmed && want_dim) {
        s_dimmed = true;
        fade_to(g_disp_config.dim, FADE_OUT_MS);
    } else if (s_dimmed && !want_dim) {
        s_dimmed = false;
        if (idle < WAKE_IGNORE_MS)              /* woken by touch, not by config */
            s_wake_ms = lv_tick_get();
        fade_to(BRIGHT_ON, FADE_IN_MS);
    } else if (s_dimmed && s_bright_target != g_disp_config.dim) {
        fade_to(g_disp_config.dim, FADE_IN_MS); /* level changed on the web */
    }
}

static void on_input(ui_input_t ev)
{
    /* the touch that wakes the display only wakes it (gesture events can
     * arrive before idle_check() notices the touch, hence s_dimmed too) */
    if (s_dimmed || lv_tick_elaps(s_wake_ms) < WAKE_IGNORE_MS) {
        ESP_LOGI(TAG, "%s (wake, ignored)", ui_input_name(ev));
        return;
    }
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
    case UI_SWIPE_UP:           /* SIMULATION: put a full keg on and tap it */
        scale_sim_keg_full();
        refresh_values();
        break;
    case UI_SWIPE_DOWN:         /* SIMULATION: untap, empty keg stays */
        scale_sim_keg_off();
        refresh_values();
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
    TickType_t last_refresh = xTaskGetTickCount();

    while (1) {
        net_mode_t mode = net_get_mode();
        if (mode != shown) {
            shown = mode;
            show_screen();      /* AP QR / web URL depend on network state */
        }

        if (xTaskGetTickCount() - last_refresh >= pdMS_TO_TICKS(REFRESH_MS)) {
            last_refresh = xTaskGetTickCount();
            refresh_values();
        }

        uint32_t ms = lv_timer_handler();
        idle_check();
        if (ms < 5)
            ms = 5;
        else if (ms > 100)
            ms = 100;
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
}
