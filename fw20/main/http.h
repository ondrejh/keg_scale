#pragma once

#include "esp_err.h"

/* Start HTTP server (idempotent). Call once the network is up. */
esp_err_t http_start(void);
