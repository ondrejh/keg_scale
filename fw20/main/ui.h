#pragma once

#include "scale.h"

/* Screen builders. Call only from task_disp (LVGL is not thread safe). */
/* Main keg screen: rounded box with the weight, or with keg name and beers
 * left when a keg is tapped. */
void ui_show_keg(void);
/* Refresh keg screen values (no-op when another screen is shown). */
void ui_keg_update(const scale_data_t *d);
void ui_show_ap(const char *ssid, const char *password, const char *ip);
/* QR code with the web UI URL; ip NULL shows "No network". */
void ui_show_webi(const char *ip);

/* Beer gauge level 0..100 %; animates there (3 s for the full range) and
 * keeps the level across screen switches. */
void ui_set_level(int pct);
