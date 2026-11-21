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

/* Display: fade to BRIGHT_DIM after IDLE_DIM_MS without touch (0..255, 0 = black) */
#define BRIGHT_ON               255
#define BRIGHT_DIM              64
#define IDLE_DIM_MS             15000
/* Display: back to the main screen after RETURN_MAIN_MS without touch (0 = never) */
#define RETURN_MAIN_MS          30000
