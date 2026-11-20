#include <stdbool.h>
#include "esp_log.h"
#include "board.h"
#include "ui_gauge.h"

static const char *TAG = "gauge";

#define W   BOARD_LCD_H_RES
#define H   BOARD_LCD_V_RES

extern const uint8_t foam_start[] asm("_binary_foam_rgb565_start");
extern const uint8_t foam_end[]   asm("_binary_foam_rgb565_end");
extern const uint8_t beer_start[] asm("_binary_beer_rgb565_start");
extern const uint8_t beer_end[]   asm("_binary_beer_rgb565_end");
extern const uint8_t edge_start[] asm("_binary_edge_rgb565a8_start");
extern const uint8_t edge_end[]   asm("_binary_edge_rgb565a8_end");

static lv_image_dsc_t s_foam, s_beer, s_edge;

/* Height is derived from the blob size, so mkbg.py row ranges can change freely. */
static bool dsc_init(lv_image_dsc_t *d, lv_color_format_t cf, uint32_t bpp_bytes,
                     const uint8_t *start, const uint8_t *end, const char *name)
{
    uint32_t size = end - start;
    uint32_t row = W * bpp_bytes;
    if (size == 0 || size % row) {
        ESP_LOGE(TAG, "%s: size %lu not a multiple of %lu", name,
                 (unsigned long)size, (unsigned long)row);
        return false;
    }
    *d = (lv_image_dsc_t) {
        .header = {
            .magic = LV_IMAGE_HEADER_MAGIC,
            .cf = cf,
            .w = W,
            .h = size / row,
            .stride = W * 2,    /* RGB565 plane stride (A8 plane follows for RGB565A8) */
        },
        .data_size = size,
        .data = start,
    };
    return true;
}

static bool textures_init(void)
{
    static int ok = -1;
    if (ok < 0) {
        ok = dsc_init(&s_foam, LV_COLOR_FORMAT_RGB565, 2, foam_start, foam_end, "foam")
          && dsc_init(&s_beer, LV_COLOR_FORMAT_RGB565, 2, beer_start, beer_end, "beer")
          && dsc_init(&s_edge, LV_COLOR_FORMAT_RGB565A8, 3, edge_start, edge_end, "edge");
    }
    return ok;
}

static lv_obj_t *plain_obj(lv_obj_t *parent)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    return o;
}

lv_obj_t *ui_gauge_create(lv_obj_t *parent)
{
    lv_obj_t *g = plain_obj(parent);
    lv_obj_set_size(g, W, H);
    lv_obj_set_pos(g, 0, 0);
    lv_obj_move_background(g);
    lv_obj_set_style_bg_color(g, lv_color_hex(0xEDE6D2), 0);   /* foam-ish fallback */

    lv_obj_t *beer = plain_obj(g);                               /* child 0 */
    lv_obj_set_width(beer, W);
    lv_obj_set_style_bg_color(beer, lv_color_hex(0xDEB94B), 0); /* beer-ish fallback */

    lv_obj_t *edge = lv_image_create(g);                         /* child 1 */
    lv_obj_remove_flag(edge, LV_OBJ_FLAG_CLICKABLE);

    if (textures_init()) {
        lv_obj_set_style_bg_image_src(g, &s_foam, 0);
        lv_obj_set_style_bg_image_tiled(g, true, 0);
        lv_obj_set_style_bg_image_src(beer, &s_beer, 0);
        lv_obj_set_style_bg_image_tiled(beer, true, 0);
        lv_image_set_src(edge, &s_edge);
    }

    ui_gauge_set_level(g, 0);
    return g;
}

void ui_gauge_set_level(lv_obj_t *g, int pct)
{
    if (pct < 0)
        pct = 0;
    else if (pct > 100)
        pct = 100;

    lv_obj_t *beer = lv_obj_get_child(g, 0);
    lv_obj_t *edge = lv_obj_get_child(g, 1);
    int32_t e = s_edge.header.h ? s_edge.header.h : 0;

    /* boundary: just below the screen at 0 % (strip hidden), strip at the top at 100 % */
    int32_t y = e / 2 + H * (100 - pct) / 100;

    if (y < H) {
        lv_obj_remove_flag(beer, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(beer, 0, y);
        lv_obj_set_height(beer, H - y);
    } else {
        lv_obj_add_flag(beer, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_set_pos(edge, 0, y - e / 2);
}
