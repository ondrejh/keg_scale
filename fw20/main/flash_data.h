/*
 * Generic CRC32-protected records in the "config" flash partition.
 *
 * The partition is split into 4 KB slots (one erase sector each).
 * Each slot holds one record: header + data. Saving erases the slot
 * (~100k erase cycles per sector).
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#define FLASH_DATA_SLOT_SIZE  4096

/*
 * Load record from slot into data (len bytes).
 * Returns ESP_OK, ESP_ERR_NOT_FOUND (empty/no magic), ESP_ERR_INVALID_SIZE,
 * ESP_ERR_INVALID_VERSION, ESP_ERR_INVALID_CRC or a flash error.
 * On any failure data is zeroed.
 */
esp_err_t flash_data_load(uint8_t slot, uint16_t version, void *data, size_t len);

/* Erase slot and write record (header + data). */
esp_err_t flash_data_save(uint8_t slot, uint16_t version, const void *data, size_t len);
