#pragma once

#include "esp_err.h"

/* flash_data slots in the "config" partition (4 KB each) */
#define CONFIG_SLOT_NET     0       /* net_config_t (config.c) */
#define CONFIG_SLOT_KEG     1       /* scale_keg_t (scale.c) */
#define CONFIG_SLOT_CALIB   2       /* scale_calib_t (scale.c) */
#define CONFIG_SLOT_MQTT    3       /* mqtt_config_t */

#define NET_CONFIG_VERSION  1
#define MQTT_CONFIG_VERSION 1

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
