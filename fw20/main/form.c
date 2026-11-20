#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "form.h"

#define BOUNDARY_MAX    72      /* RFC 2046 limit is 70 */

static bool copy_out(const char *src, size_t n, char *out, size_t out_len)
{
    if (n >= out_len)
        return false;
    memcpy(out, src, n);
    out[n] = '\0';
    return true;
}

/* ---- multipart/form-data ---- */

static bool get_boundary(const char *ctype, char *delim, size_t delim_len)
{
    const char *b = strcasestr(ctype, "boundary=");
    if (!b)
        return false;
    b += strlen("boundary=");
    bool quoted = *b == '"';
    if (quoted)
        b++;
    size_t n = quoted ? strcspn(b, "\"") : strcspn(b, "; \t");
    if (n == 0 || n > BOUNDARY_MAX)
        return false;
    /* values end with CRLF followed by "--boundary" */
    return snprintf(delim, delim_len, "\r\n--%.*s", (int)n, b) < (int)delim_len;
}

static bool multipart_get(const char *body, const char *ctype, const char *name,
                          char *out, size_t out_len)
{
    char delim[BOUNDARY_MAX + 5];
    if (!get_boundary(ctype, delim, sizeof(delim)))
        return false;

    char key[48];
    if (snprintf(key, sizeof(key), "name=\"%s\"", name) >= (int)sizeof(key))
        return false;

    /* parts start right after a delimiter line ("--boundary" at body start) */
    for (const char *p = strstr(body, delim + 2); p; p = strstr(p + 1, delim)) {
        const char *hdr_end = strstr(p, "\r\n\r\n");
        if (!hdr_end)
            return false;
        const char *k = strstr(p, key);
        if (!k || k > hdr_end)
            continue;
        const char *val = hdr_end + 4;
        const char *end = strstr(val, delim);
        if (!end)
            return false;
        return copy_out(val, end - val, out, out_len);
    }
    return false;
}

/* ---- application/x-www-form-urlencoded ---- */

static int hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    c = tolower((unsigned char)c);
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static bool urlencoded_get(const char *body, const char *name, char *out, size_t out_len)
{
    size_t name_len = strlen(name);
    for (const char *p = body; *p; ) {
        size_t pair = strcspn(p, "&");
        if (pair > name_len && strncmp(p, name, name_len) == 0 && p[name_len] == '=') {
            const char *v = p + name_len + 1, *end = p + pair;
            size_t o = 0;
            while (v < end) {
                char c = *v++;
                if (c == '+') {
                    c = ' ';
                } else if (c == '%' && end - v >= 2 && hexval(v[0]) >= 0 && hexval(v[1]) >= 0) {
                    c = (char)(hexval(v[0]) << 4 | hexval(v[1]));
                    v += 2;
                }
                if (o + 1 >= out_len)
                    return false;
                out[o++] = c;
            }
            out[o] = '\0';
            return true;
        }
        if (p[pair] == '\0')
            break;
        p += pair + 1;
    }
    return false;
}

bool form_get(const char *body, const char *ctype, const char *name,
              char *out, size_t out_len)
{
    if (out_len == 0)
        return false;
    out[0] = '\0';
    if (strncasecmp(ctype, "multipart/form-data", 19) == 0)
        return multipart_get(body, ctype, name, out, out_len);
    if (strncasecmp(ctype, "application/x-www-form-urlencoded", 33) == 0)
        return urlencoded_get(body, name, out, out_len);
    return false;
}

bool form_has(const char *body, const char *ctype, const char *name)
{
    /* a value can't be longer than the body itself */
    size_t n = strlen(body) + 1;
    char buf[n];
    return form_get(body, ctype, name, buf, n);
}
