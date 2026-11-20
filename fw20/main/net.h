#pragma once

typedef enum {
    NET_MODE_NONE,
    NET_MODE_STA,
    NET_MODE_AP,
} net_mode_t;

extern char g_dev_id[16];   /* e.g. "kegA1B2C3" */

net_mode_t net_get_mode(void);
