#include <stdio.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "net.h"
#include "http.h"

static const char *TAG = "http";

static httpd_handle_t s_server;

static const char *mode_str(void)
{
    switch (net_get_mode()) {
    case NET_MODE_STA: return "STA";
    case NET_MODE_AP:  return "AP";
    default:           return "none";
    }
}

static esp_err_t root_get(httpd_req_t *req)
{
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/index.html");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t index_get(httpd_req_t *req)
{
    char buf[512];
    snprintf(buf, sizeof(buf),
             "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
             "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
             "<title>Kegator %s</title></head><body>"
             "<h1>Kegator %s</h1>"
             "<p>Network mode: %s</p>"
             "<p>Dummy page, web UI not installed yet. See <a href=\"/data.json\">data.json</a>.</p>"
             "</body></html>",
             g_dev_id, g_dev_id, mode_str());
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_sendstr(req, buf);
}

static esp_err_t data_get(httpd_req_t *req)
{
    static const char dummy[] =
        "{\"raw\":-21766026,\"units\":\"37.9\",\"temp\":28.5,\"traw\":408,"
        "\"primary_unit\":\"kg\",\"secondary_unit\":\"piv\",\"ratio\":0.5,"
        "\"calib\":{\"30.0\":-18753708,\"10.0\":-11127584,\"0.0\":-7110488}}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_sendstr(req, dummy);
}

static const httpd_uri_t s_uris[] = {
    { .uri = "/",           .method = HTTP_GET, .handler = root_get  },
    { .uri = "/index.html", .method = HTTP_GET, .handler = index_get },
    { .uri = "/data.json",  .method = HTTP_GET, .handler = data_get  },
};

esp_err_t http_start(void)
{
    if (s_server)
        return ESP_OK;

    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.lru_purge_enable = true;

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
