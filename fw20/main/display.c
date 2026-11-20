#include "freertos/FreeRTOS.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_sh8601.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "board.h"
#include "display.h"

static const char *TAG = "display";

#define LCD_BPP          16
#define LCD_X_OFFSET     6        /* panel RAM columns 6..471 */
#define LCD_PCLK_HZ      (40 * 1000 * 1000)
#define LVGL_BUF_LINES   40
#define LVGL_BUF_SIZE    (BOARD_LCD_H_RES * LVGL_BUF_LINES * LCD_BPP / 8)

/* CO5300 init sequence (from Waveshare demo) */
static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xFE, (uint8_t[]){0x00}, 1, 0},
    {0xC4, (uint8_t[]){0x80}, 1, 0},
    {0x3A, (uint8_t[]){0x55}, 1, 0},                     /* RGB565 */
    {0x35, (uint8_t[]){0x00}, 1, 0},                     /* TE on */
    {0x53, (uint8_t[]){0x20}, 1, 0},
    {0x51, (uint8_t[]){0xFF}, 1, 0},                     /* brightness */
    {0x63, (uint8_t[]){0xFF}, 1, 0},
    {0x2A, (uint8_t[]){0x00, 0x06, 0x01, 0xD7}, 4, 0},   /* columns 6..471 */
    {0x2B, (uint8_t[]){0x00, 0x00, 0x01, 0xD1}, 4, 0},   /* rows 0..465 */
    {0x11, (uint8_t[]){0x00}, 0, 100},                   /* sleep out */
    {0x29, (uint8_t[]){0x00}, 0, 0},                     /* display on */
};

static esp_lcd_panel_io_handle_t s_io;
static esp_lcd_panel_handle_t s_panel;

/* QSPI command word: 0x02 = write opcode, cmd in bits 15..8 */
static void lcd_tx_param(uint8_t cmd, uint8_t param)
{
    esp_lcd_panel_io_tx_param(s_io, (0x02u << 24) | ((uint32_t)cmd << 8), &param, 1);
}

void display_set_brightness(uint8_t level)
{
    lcd_tx_param(0x51, level);
}

static bool on_color_trans_done(esp_lcd_panel_io_handle_t io,
                                esp_lcd_panel_io_event_data_t *edata, void *ctx)
{
    lv_display_flush_ready((lv_display_t *)ctx);
    return false;
}

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px)
{
    lv_draw_sw_rgb565_swap(px, lv_area_get_width(area) * lv_area_get_height(area));
    esp_lcd_panel_draw_bitmap(s_panel,
                              area->x1 + LCD_X_OFFSET, area->y1,
                              area->x2 + LCD_X_OFFSET + 1, area->y2 + 1, px);
}

/* CO5300 needs even start / odd end coordinates */
static void rounder_cb(lv_event_t *e)
{
    lv_area_t *a = lv_event_get_param(e);
    a->x1 &= ~1;
    a->y1 &= ~1;
    a->x2 |= 1;
    a->y2 |= 1;
}

static uint32_t tick_cb(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

esp_err_t display_init(void)
{
    ESP_LOGI(TAG, "init CO5300 %dx%d", BOARD_LCD_H_RES, BOARD_LCD_V_RES);

    lv_init();
    lv_tick_set_cb(tick_cb);
    lv_display_t *disp = lv_display_create(BOARD_LCD_H_RES, BOARD_LCD_V_RES);

    spi_bus_config_t bus = {
        .sclk_io_num = BOARD_LCD_PCLK,
        .data0_io_num = BOARD_LCD_D0,
        .data1_io_num = BOARD_LCD_D1,
        .data2_io_num = BOARD_LCD_D2,
        .data3_io_num = BOARD_LCD_D3,
        .max_transfer_sz = LVGL_BUF_SIZE,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(BOARD_LCD_HOST, &bus, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = BOARD_LCD_CS,
        .dc_gpio_num = -1,
        .spi_mode = 0,
        .pclk_hz = LCD_PCLK_HZ,
        .trans_queue_depth = 10,
        .on_color_trans_done = on_color_trans_done,
        .user_ctx = disp,
        .lcd_cmd_bits = 32,
        .lcd_param_bits = 8,
        .flags.quad_mode = true,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BOARD_LCD_HOST, &io_cfg, &s_io));

    sh8601_vendor_config_t vendor = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
        .flags.use_qspi_interface = 1,
    };
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = BOARD_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BPP,
        .vendor_config = &vendor,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(s_io, &panel_cfg, &s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));
    lcd_tx_param(0x36, 0xC0);   /* MADCTL: rotate 180 deg (as in Waveshare demo) */

    void *buf1 = heap_caps_malloc(LVGL_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    void *buf2 = heap_caps_malloc(LVGL_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!buf1 || !buf2) {
        ESP_LOGE(TAG, "no memory for draw buffers");
        return ESP_ERR_NO_MEM;
    }
    lv_display_set_buffers(disp, buf1, buf2, LVGL_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_add_event_cb(disp, rounder_cb, LV_EVENT_INVALIDATE_AREA, NULL);

    return ESP_OK;
}
