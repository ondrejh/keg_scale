#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "lvgl.h"
#include "ui.h"
#include "ui_gauge.h"

static const char *TAG = "ui";

#define QR_SIZE     240
#define QR_BORDER   12

static lv_obj_t *s_gauge;

/* Clean screen with the beer gauge as background (level 100 %). */
static lv_obj_t *new_screen(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);    /* scrolling would block swipes */
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    s_gauge = ui_gauge_create(scr);
    ui_gauge_set_level(s_gauge, 100);
    return scr;
}

void ui_set_level(int pct)
{
    if (s_gauge)
        ui_gauge_set_level(s_gauge, pct);
}

static lv_obj_t *new_label(lv_obj_t *parent, const char *text, const lv_font_t *font)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_color(l, lv_color_black(), 0);   /* bright background */
    lv_obj_set_style_text_font(l, font, 0);
    return l;
}

/* Escape \ ; , : " for the WIFI: QR payload. Returns length written. */
static size_t wifi_qr_escape(char *dst, size_t n, const char *src)
{
    size_t i = 0;
    for (; *src && i + 2 < n; src++) {
        if (strchr("\\;,:\"", *src))
            dst[i++] = '\\';
        dst[i++] = *src;
    }
    dst[i] = '\0';
    return i;
}

void ui_show_hello(void)
{
    lv_obj_t *scr = new_screen();
    lv_obj_center(new_label(scr, "Hello world", &lv_font_montserrat_32));
}

/* New screen with a centred QR code (white quiet zone) and two labels below. */
static void show_qr(const char *payload, const char *title, const char *subtitle)
{
    lv_obj_t *scr = new_screen();

    lv_obj_t *qr = lv_qrcode_create(scr);
    lv_qrcode_set_size(qr, QR_SIZE);
    lv_qrcode_set_dark_color(qr, lv_color_black());
    lv_qrcode_set_light_color(qr, lv_color_white());
    if (lv_qrcode_update(qr, payload, strlen(payload)) != LV_RESULT_OK)
        ESP_LOGE(TAG, "qr encode failed");
    lv_obj_set_style_border_color(qr, lv_color_white(), 0);
    lv_obj_set_style_border_width(qr, QR_BORDER, 0);
    lv_obj_align(qr, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *l_title = new_label(scr, title, &lv_font_montserrat_32);
    lv_obj_align_to(l_title, qr, LV_ALIGN_OUT_BOTTOM_MID, 0, 14);

    lv_obj_t *l_sub = new_label(scr, subtitle, &lv_font_montserrat_16);
    lv_obj_align_to(l_sub, l_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
}

void ui_show_ap(const char *ssid, const char *password, const char *ip)
{
    char s[2 * 32 + 1], p[2 * 64 + 1], payload[sizeof(s) + sizeof(p) + 24];
    wifi_qr_escape(s, sizeof(s), ssid);
    wifi_qr_escape(p, sizeof(p), password);
    snprintf(payload, sizeof(payload), "WIFI:T:WPA;S:%s;P:%s;;", s, p);
    show_qr(payload, ssid, ip);
}

void ui_show_webi(const char *ip)
{
    if (!ip) {
        lv_obj_t *scr = new_screen();
        lv_obj_center(new_label(scr, "No network", &lv_font_montserrat_32));
        return;
    }
    char url[32];
    snprintf(url, sizeof(url), "http://%s/", ip);
    show_qr(url, "Web", url);
}
