#pragma once

/* Screen builders. Call only from task_disp (LVGL is not thread safe). */
void ui_show_hello(void);
void ui_show_ap(const char *ssid, const char *password, const char *ip);
