/* Minimal HTML form decoding for POST bodies (multipart/form-data, urlencoded). */
#pragma once

#include <stdbool.h>
#include <stddef.h>

/*
 * Find field `name` in a NUL-terminated request body of the given Content-Type
 * and copy its value to out (always NUL-terminated).
 * Returns false if the field is missing, the type is unsupported or the value
 * does not fit into out_len - 1 bytes.
 */
bool form_get(const char *body, const char *ctype, const char *name,
              char *out, size_t out_len);

/* True if field `name` is present (any length, possibly empty). */
bool form_has(const char *body, const char *ctype, const char *name);
