#pragma once

#include "esp_err.h"

#define CONFIG_SLOT_NET     0
#define NET_CONFIG_VERSION  1

typedef struct {
    char ssid[33];      /* max 32 chars + NUL */
    char password[65];  /* max 64 chars + NUL */
} net_config_t;

extern net_config_t g_net_config;

esp_err_t config_load(void);
esp_err_t config_save(void);
