#pragma once

/* Screen builders. Call only from task_disp (LVGL is not thread safe). */
/* Main keg screen. No keg tapped: current weight in a rounded box. */
void ui_show_keg(void);
/* Refresh keg screen values (no-op when another screen is shown). */
void ui_keg_update(float units, const char *unit);
void ui_show_ap(const char *ssid, const char *password, const char *ip);
/* QR code with the web UI URL; ip NULL shows "No network". */
void ui_show_webi(const char *ip);

/* Beer gauge level of the current screen, 0..100 %. */
void ui_set_level(int pct);
