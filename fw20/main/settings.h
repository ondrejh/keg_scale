/* Build-time settings */
#pragma once

/* Device id / AP SSID: DEV_ID_PREFIX + last 3 bytes of base MAC, e.g. "kegA1B2C3" */
#define DEV_ID_PREFIX           "keg"

/* Access point (used when no SSID is configured or STA connect fails) */
#define AP_PASSWORD             "k3Gat0rr"
#define AP_IP                   "192.168.42.1"
#define AP_NETMASK              "255.255.255.0"
#define AP_CHANNEL              1
#define AP_MAX_CONN             4

/* Station */
#define STA_CONNECT_TIMEOUT_MS  15000
