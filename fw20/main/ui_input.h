/*
 * Touch gestures on the round screen: tap zones + swipes.
 * Runs inside lv_timer_handler (task_disp).
 */
#pragma once

#include "lvgl.h"

typedef enum {
    UI_TAP_LEFT,
    UI_TAP_RIGHT,
    UI_TAP_TOP,
    UI_TAP_BOTTOM,
    UI_TAP_CENTER,
    UI_DTAP_CENTER,
    UI_SWIPE_LEFT,
    UI_SWIPE_RIGHT,
    UI_SWIPE_UP,
    UI_SWIPE_DOWN,
} ui_input_t;

typedef void (*ui_input_cb_t)(ui_input_t ev);

void ui_input_init(lv_indev_t *indev, ui_input_cb_t cb);

const char *ui_input_name(ui_input_t ev);
