#pragma once

#include "esp_err.h"

/* Init CO5300 AMOLED (QSPI) and LVGL display. Call from the task that runs LVGL. */
esp_err_t display_init(void);

/* Set AMOLED brightness 0..255 */
void display_set_brightness(uint8_t level);
