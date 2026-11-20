#pragma once

/* Screen builders. Call only from task_disp (LVGL is not thread safe). */
void ui_show_hello(void);
void ui_show_ap(const char *ssid, const char *password, const char *ip);

/* Beer gauge level of the current screen, 0..100 % (stops the demo animation). */
void ui_set_level(int pct);
