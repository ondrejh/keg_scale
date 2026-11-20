/* CST820 capacitive touch on the shared I2C bus (polled, INT/RST not wired). */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

esp_err_t touch_init(void);

/* Current touch point in display coordinates. Returns false if not pressed. */
bool touch_read(uint16_t *x, uint16_t *y);

/* Shared I2C bus (touch, ES8311 codec). NULL before touch_init(). */
i2c_master_bus_handle_t board_i2c_bus(void);
