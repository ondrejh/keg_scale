#pragma once

#include "esp_err.h"

#define WWW_BASE    "/www"      /* LittleFS "storage" mount point (web files) */

/* Start HTTP server (idempotent). Call once the network is up. */
esp_err_t http_start(void);
