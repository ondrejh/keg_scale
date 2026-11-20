#include "esp_log.h"
#include "board.h"
#include "touch.h"

static const char *TAG = "touch";

/* Panel is rotated 180 deg in display.c (MADCTL 0xC0); mirror touch to match. */
#define TOUCH_MIRROR_XY     1

#define I2C_HZ              400000
#define I2C_TIMEOUT_MS      20

#define CST_REG_GESTURE     0x01    /* gesture, points, xh, xl, yh, yl */
#define CST_REG_DIS_AUTOSLEEP 0xFE

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;

i2c_master_bus_handle_t board_i2c_bus(void)
{
    return s_bus;
}

esp_err_t touch_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = BOARD_I2C_PORT,
        .sda_io_num = BOARD_I2C_SDA,
        .scl_io_num = BOARD_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c bus init failed (%s)", esp_err_to_name(err));
        return err;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BOARD_TOUCH_I2C_ADDR,
        .scl_speed_hz = I2C_HZ,
    };
    err = i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "add device failed (%s)", esp_err_to_name(err));
        return err;
    }

    /* CST8xx may sleep and NACK until touched - not fatal, keep the device. */
    if (i2c_master_probe(s_bus, BOARD_TOUCH_I2C_ADDR, I2C_TIMEOUT_MS) == ESP_OK)
        ESP_LOGI(TAG, "CST820 found at 0x%02x", BOARD_TOUCH_I2C_ADDR);
    else
        ESP_LOGW(TAG, "CST820 not answering at 0x%02x (asleep?)", BOARD_TOUCH_I2C_ADDR);

    /* keep it awake so polling works without the INT line */
    uint8_t cmd[2] = { CST_REG_DIS_AUTOSLEEP, 0x01 };
    i2c_master_transmit(s_dev, cmd, sizeof(cmd), I2C_TIMEOUT_MS);
    return ESP_OK;
}

bool touch_read(uint16_t *x, uint16_t *y)
{
    if (!s_dev)
        return false;

    uint8_t reg = CST_REG_GESTURE, d[6];
    if (i2c_master_transmit_receive(s_dev, &reg, 1, d, sizeof(d), I2C_TIMEOUT_MS) != ESP_OK)
        return false;
    if ((d[1] & 0x0F) == 0)
        return false;

    uint16_t tx = ((d[2] & 0x0F) << 8) | d[3];
    uint16_t ty = ((d[4] & 0x0F) << 8) | d[5];
    if (tx >= BOARD_LCD_H_RES)
        tx = BOARD_LCD_H_RES - 1;
    if (ty >= BOARD_LCD_V_RES)
        ty = BOARD_LCD_V_RES - 1;
#if TOUCH_MIRROR_XY
    tx = BOARD_LCD_H_RES - 1 - tx;
    ty = BOARD_LCD_V_RES - 1 - ty;
#endif
    *x = tx;
    *y = ty;
    return true;
}
