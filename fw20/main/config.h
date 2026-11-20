#pragma once

#include <stdint.h>
#include "esp_err.h"

/* flash_data slots in the "config" partition (4 KB each) */
#define CONFIG_SLOT_NET     0       /* net_config_t (config.c) */
#define CONFIG_SLOT_KEG     1       /* scale_keg_t (scale.c) */
#define CONFIG_SLOT_CALIB   2       /* scale_calib_t (scale.c) */
#define CONFIG_SLOT_MQTT    3       /* mqtt_config_t */
#define CONFIG_SLOT_DISP    4       /* disp_config_t */

#define NET_CONFIG_VERSION  1
#define MQTT_CONFIG_VERSION 1
#define DISP_CONFIG_VERSION 1

typedef struct {
    char ssid[33];      /* max 32 chars + NUL */
    char password[65];  /* max 64 chars + NUL */
} net_config_t;

extern net_config_t g_net_config;

esp_err_t config_load(void);
esp_err_t config_save(void);

/* MQTT settings from the web config page. Stored only - no MQTT client yet. */
typedef struct {
    char url[65];
    char topic[65];
    char user[33];
    char password[65];
} mqtt_config_t;

extern mqtt_config_t g_mqtt_config;

esp_err_t mqtt_config_load(void);
esp_err_t mqtt_config_save(void);

/* Display idle dimming (web config page). Defaults: settings.h. */
typedef struct {
    uint32_t idle_ms;   /* dim after this long without touch, 0 = never */
    uint8_t dim;        /* brightness when dimmed, 0..255 */
} disp_config_t;

extern disp_config_t g_disp_config;

esp_err_t disp_config_load(void);   /* falls back to defaults */
esp_err_t disp_config_save(void);
