#include <string.h>
#include "esp_partition.h"
#include "esp_rom_crc.h"
#include "esp_log.h"
#include "flash_data.h"

#define FLASH_DATA_PART_LABEL   "config"
#define FLASH_DATA_PART_SUBTYPE 0x40
#define FLASH_DATA_MAGIC        0x4746434B  /* "KCFG" little endian */

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t len;
    uint32_t crc32;
} flash_data_hdr_t;

static const char *TAG = "flash_data";
static const esp_partition_t *s_part;

static esp_err_t get_slot(uint8_t slot, size_t len, size_t *offset)
{
    if (!s_part) {
        s_part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                          FLASH_DATA_PART_SUBTYPE,
                                          FLASH_DATA_PART_LABEL);
        if (!s_part) {
            ESP_LOGE(TAG, "partition '%s' not found", FLASH_DATA_PART_LABEL);
            return ESP_ERR_NOT_FOUND;
        }
    }
    if (len > FLASH_DATA_SLOT_SIZE - sizeof(flash_data_hdr_t) || len > UINT16_MAX)
        return ESP_ERR_INVALID_SIZE;
    *offset = (size_t)slot * FLASH_DATA_SLOT_SIZE;
    if (*offset + FLASH_DATA_SLOT_SIZE > s_part->size)
        return ESP_ERR_INVALID_ARG;
    return ESP_OK;
}

static esp_err_t load(uint8_t slot, uint16_t version, void *data, size_t len)
{
    size_t off;
    esp_err_t err = get_slot(slot, len, &off);
    if (err != ESP_OK)
        return err;

    flash_data_hdr_t hdr;
    err = esp_partition_read(s_part, off, &hdr, sizeof(hdr));
    if (err != ESP_OK)
        return err;
    if (hdr.magic != FLASH_DATA_MAGIC)
        return ESP_ERR_NOT_FOUND;
    if (hdr.len != len)
        return ESP_ERR_INVALID_SIZE;
    if (hdr.version != version)
        return ESP_ERR_INVALID_VERSION;

    err = esp_partition_read(s_part, off + sizeof(hdr), data, len);
    if (err != ESP_OK)
        return err;
    if (esp_rom_crc32_le(0, data, len) != hdr.crc32)
        return ESP_ERR_INVALID_CRC;
    return ESP_OK;
}

esp_err_t flash_data_load(uint8_t slot, uint16_t version, void *data, size_t len)
{
    esp_err_t err = load(slot, version, data, len);
    if (err != ESP_OK)
        memset(data, 0, len);
    return err;
}

esp_err_t flash_data_save(uint8_t slot, uint16_t version, const void *data, size_t len)
{
    size_t off;
    esp_err_t err = get_slot(slot, len, &off);
    if (err != ESP_OK)
        return err;

    flash_data_hdr_t hdr = {
        .magic = FLASH_DATA_MAGIC,
        .version = version,
        .len = (uint16_t)len,
        .crc32 = esp_rom_crc32_le(0, data, len),
    };

    err = esp_partition_erase_range(s_part, off, FLASH_DATA_SLOT_SIZE);
    if (err == ESP_OK)
        err = esp_partition_write(s_part, off + sizeof(hdr), data, len);
    if (err == ESP_OK)  /* header last: record valid only when fully written */
        err = esp_partition_write(s_part, off, &hdr, sizeof(hdr));
    if (err != ESP_OK)
        ESP_LOGE(TAG, "save slot %u failed: %s", slot, esp_err_to_name(err));
    return err;
}
