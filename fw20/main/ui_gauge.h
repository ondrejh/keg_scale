/*
 * Beer level gauge used as screen background: foam on top, beer below,
 * boundary moves with the level. Textures from tools/mkbg.py (web20 bg.jpg).
 * LVGL calls - use from task_disp only.
 */
#pragma once

#include "lvgl.h"

/* Full-screen gauge object, created as the parent's first child. */
lv_obj_t *ui_gauge_create(lv_obj_t *parent);

/* 0 = foam only, 100 = beer with a thin foam rim. Clamped. */
void ui_gauge_set_level(lv_obj_t *gauge, int pct);
