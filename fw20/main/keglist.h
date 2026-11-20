/*
 * Known keg names for the web autocomplete (keg.html fetches /keglist.txt).
 * Stored as keglist.txt in the LittleFS storage partition, one name per line,
 * newest first. Note: flashing the storage image (idf.py flash) resets it to
 * web20/keglist.txt.
 */
#pragma once

#include "esp_err.h"

#define KEGLIST_MAX     20      /* names kept */

/* Move/add name to the top of the list. Call after the FS is mounted. */
esp_err_t keglist_add(const char *name);
