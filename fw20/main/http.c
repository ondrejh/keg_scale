#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_littlefs.h"
#include "esp_app_desc.h"
#include "esp_netif.h"
#include "net.h"
#include "scale.h"
#include "http.h"

static const char *TAG = "http";

#define WWW_BASE        "/www"
#define WWW_PARTITION   "storage"
#define CHUNK_SIZE      1024

static httpd_handle_t s_server;
static char s_chunk[CHUNK_SIZE];    /* httpd handlers run in one task */

static void fs_mount(void)
{
    esp_vfs_littlefs_conf_t conf = {
        .base_path = WWW_BASE,
        .partition_label = WWW_PARTITION,
        .format_if_mount_failed = false,
    };
    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "littlefs mount failed (%s)", esp_err_to_name(err));
        return;
    }
    size_t total = 0, used = 0;
    esp_littlefs_info(WWW_PARTITION, &total, &used);
    ESP_LOGI(TAG, "littlefs mounted on " WWW_BASE ", %u / %u bytes used",
             (unsigned)used, (unsigned)total);
}

static const char *content_type(const char *path, bool *no_cache)
{
    static const struct { const char *ext, *type; bool no_cache; } types[] = {
        { ".html", "text/html; charset=utf-8",  false },
        { ".css",  "text/css",                  false },
        { ".js",   "application/javascript",    false },
        { ".json", "application/json",          true  },
        { ".txt",  "text/plain; charset=utf-8", true  },
        { ".svg",  "image/svg+xml",             false },
        { ".jpg",  "image/jpeg",                false },
        { ".png",  "image/png",                 false },
        { ".ico",  "image/x-icon",              false },
    };
    const char *ext = strrchr(path, '.');
    if (ext) {
        for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
            if (strcasecmp(ext, types[i].ext) == 0) {
                *no_cache = types[i].no_cache;
                return types[i].type;
            }
        }
    }
    *no_cache = false;
    return "application/octet-stream";
}

static esp_err_t root_get(httpd_req_t *req)
{
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/index.html");
    return httpd_resp_send(req, NULL, 0);
}

/* Stub for the web20 action endpoint: swallow the form, answer OK. */
static esp_err_t do_post(httpd_req_t *req)
{
    size_t left = req->content_len;
    while (left > 0) {
        int n = httpd_req_recv(req, s_chunk, left < CHUNK_SIZE ? left : CHUNK_SIZE);
        if (n == HTTPD_SOCK_ERR_TIMEOUT)
            continue;
        if (n <= 0)
            return ESP_FAIL;
        left -= n;
    }
    ESP_LOGI(TAG, "do.php (%u bytes, ignored)", (unsigned)req->content_len);
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_sendstr(req, "OK");
}

/* Append printf-formatted text; on overflow pos runs past len and stays there. */
static void jcat(char *buf, size_t len, size_t *pos, const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));
static void jcat(char *buf, size_t len, size_t *pos, const char *fmt, ...)
{
    if (*pos >= len)
        return;
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf + *pos, len - *pos, fmt, ap);
    va_end(ap);
    *pos += n > 0 ? (size_t)n : 0;
}

/* Append a quoted JSON string, escaping quotes, backslashes and control chars. */
static void jstr(char *buf, size_t len, size_t *pos, const char *s)
{
    jcat(buf, len, pos, "\"");
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\')
            jcat(buf, len, pos, "\\%c", c);
        else if (c < 0x20)
            jcat(buf, len, pos, "\\u%04x", c);
        else
            jcat(buf, len, pos, "%c", c);
    }
    jcat(buf, len, pos, "\"");
}

/* Live scale data, same layout as the original sketch (build_data). */
static esp_err_t data_get(httpd_req_t *req)
{
    scale_data_t d;
    scale_get(&d);

    char buf[1024];
    size_t p = 0;
    const size_t n = sizeof(buf);

    jcat(buf, n, &p, "{\"devid\":");
    jstr(buf, n, &p, g_dev_id);
    jcat(buf, n, &p, ",\"sw\":");
    jstr(buf, n, &p, esp_app_get_description()->version);
    jcat(buf, n, &p, ",\"raw\":%ld,\"units\":%.1f", (long)d.raw, d.units);

    if (net_get_mode() == NET_MODE_STA) {
        esp_netif_ip_info_t ip;
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif && esp_netif_get_ip_info(netif, &ip) == ESP_OK)
            jcat(buf, n, &p, ",\"ip\":\"" IPSTR "\"", IP2STR(&ip.ip));
    }

    if (d.temp_valid)
        jcat(buf, n, &p, ",\"temp\":%.1f", d.temp);

    jcat(buf, n, &p, ",\"primary_unit\":");
    jstr(buf, n, &p, d.calib.uprim);
    jcat(buf, n, &p, ",\"secondary_unit\":");
    jstr(buf, n, &p, d.calib.usec);

    jcat(buf, n, &p, ",\"calib\":{");
    for (int i = 0; i < d.calib.n; i++)
        jcat(buf, n, &p, "%s\"%.2f\":%ld", i ? "," : "", d.calib.y[i], (long)d.calib.x[i]);
    jcat(buf, n, &p, "}");

    if (d.keg.tapped) {
        jcat(buf, n, &p, ",\"keg\":{\"name\":");
        jstr(buf, n, &p, d.keg.name);
        jcat(buf, n, &p, ",\"volume\":%.1f,\"left\":%.1f}", d.keg.volume, d.keg_left);
    }
    jcat(buf, n, &p, "}");

    if (p >= n) {
        ESP_LOGE(TAG, "data.json overflow (%u)", (unsigned)p);
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "overflow");
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, buf, p);
}

static esp_err_t file_get(httpd_req_t *req)
{
    char path[sizeof(WWW_BASE) + CONFIG_HTTPD_MAX_URI_LEN];
    size_t ulen = strcspn(req->uri, "?#");
    if (ulen == 0 || strstr(req->uri, "..") || ulen + sizeof(WWW_BASE) > sizeof(path))
        return httpd_resp_send_404(req);
    memcpy(path, WWW_BASE, sizeof(WWW_BASE) - 1);
    memcpy(path + sizeof(WWW_BASE) - 1, req->uri, ulen);
    path[sizeof(WWW_BASE) - 1 + ulen] = '\0';

    struct stat st;
    FILE *f = NULL;
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode) || !(f = fopen(path, "rb"))) {
        ESP_LOGD(TAG, "404 %s", req->uri);
        return httpd_resp_send_404(req);
    }

    bool no_cache;
    httpd_resp_set_type(req, content_type(path, &no_cache));
    httpd_resp_set_hdr(req, "Cache-Control", no_cache ? "no-store" : "max-age=3600");

    esp_err_t err = ESP_OK;
    size_t n;
    while ((n = fread(s_chunk, 1, CHUNK_SIZE, f)) > 0) {
        err = httpd_resp_send_chunk(req, s_chunk, n);
        if (err != ESP_OK)
            break;
    }
    fclose(f);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "send %s failed (%s)", req->uri, esp_err_to_name(err));
        return err;
    }
    return httpd_resp_send_chunk(req, NULL, 0);
}

static const httpd_uri_t s_uris[] = {
    { .uri = "/",       .method = HTTP_GET,  .handler = root_get },
    { .uri = "/data.json", .method = HTTP_GET, .handler = data_get },
    { .uri = "/do.php", .method = HTTP_POST, .handler = do_post  },
    { .uri = "/*",      .method = HTTP_GET,  .handler = file_get },
};

esp_err_t http_start(void)
{
    if (s_server)
        return ESP_OK;

    fs_mount();

    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.lru_purge_enable = true;
    cfg.uri_match_fn = httpd_uri_match_wildcard;
    cfg.stack_size = 6144;          /* data.json builds a 1 KB buffer + printf floats */

    esp_err_t err = httpd_start(&s_server, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "start failed (%s)", esp_err_to_name(err));
        s_server = NULL;
        return err;
    }

    for (size_t i = 0; i < sizeof(s_uris) / sizeof(s_uris[0]); i++)
        httpd_register_uri_handler(s_server, &s_uris[i]);

    ESP_LOGI(TAG, "http server started on port %d", cfg.server_port);
    return ESP_OK;
}
