/*
 * HX711 24-bit load cell ADC, bit-banged. Channel A, gain 128.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"

/* Configure pins and reset the chip (SCK high > 60 us, then low). */
void hx711_init(gpio_num_t dout, gpio_num_t sck);

/* Conversion ready (DOUT low). */
bool hx711_ready(void);

/* Read one conversion (call when ready). False when the chip did not respond
 * (DOUT not released after the 25th pulse = missing / unpowered) or an SCK
 * edge timed out. */
bool hx711_read(int32_t *out);
