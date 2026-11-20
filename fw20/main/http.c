#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_littlefs.h"
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
