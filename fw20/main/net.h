#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    NET_MODE_NONE,
    NET_MODE_STA,
    NET_MODE_AP,
} net_mode_t;

extern char g_dev_id[16];   /* e.g. "kegA1B2C3" */

net_mode_t net_get_mode(void);

/* Current IP as text (STA lease or AP address). False if no network yet. */
bool net_get_ip(char *buf, size_t len);
