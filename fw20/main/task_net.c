#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "lwip/ip4_addr.h"
#include "config.h"
#include "settings.h"
#include "net.h"
#include "http.h"
#include "tasks.h"

static const char *TAG = "net";

#define CONNECTED_BIT BIT0

char g_dev_id[16];
static volatile net_mode_t s_mode = NET_MODE_NONE;
static EventGroupHandle_t s_events;

net_mode_t net_get_mode(void)
{
    return s_mode;
}

static void make_dev_id(void)
{
    /* Wi-Fi STA MAC == base MAC, always 6 bytes. (esp_efuse_mac_get_default()
     * returns 8-byte EUI-64 on 802.15.4 capable chips like the C6.) */
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
    snprintf(g_dev_id, sizeof(g_dev_id), DEV_ID_PREFIX "%02X%02X%02X",
             mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "device id %s (mac " MACSTR ")", g_dev_id, MAC2STR(mac));
}

bool net_get_ip(char *buf, size_t len)
{
    switch (s_mode) {
    case NET_MODE_AP:
        snprintf(buf, len, "%s", AP_IP);
        return true;
    case NET_MODE_STA: {
        esp_netif_ip_info_t ip;
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (!netif || esp_netif_get_ip_info(netif, &ip) != ESP_OK || ip.ip.addr == 0)
            return false;
        snprintf(buf, len, IPSTR, IP2STR(&ip.ip));
        return true;
    }
    default:
        return false;
    }
}

/* ---------------- STA ---------------- */

static void sta_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_events, CONNECTED_BIT);
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *ev = data;
        ESP_LOGI(TAG, "connected to \"%s\", ip " IPSTR,
                 g_net_config.ssid, IP2STR(&ev->ip_info.ip));
        xEventGroupSetBits(s_events, CONNECTED_BIT);
    }
}

static bool sta_start(void)
{
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    esp_netif_set_hostname(netif, g_dev_id);

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, sta_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_event_handler, NULL));

    wifi_config_t wc = { 0 };
    memcpy(wc.sta.ssid, g_net_config.ssid, strnlen(g_net_config.ssid, sizeof(wc.sta.ssid)));
    memcpy(wc.sta.password, g_net_config.password,
           strnlen(g_net_config.password, sizeof(wc.sta.password)));
    wc.sta.threshold.authmode = g_net_config.password[0] ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
    ESP_LOGI(TAG, "connecting to \"%s\"", g_net_config.ssid);
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_events, CONNECTED_BIT, pdFALSE, pdTRUE,
                                           pdMS_TO_TICKS(STA_CONNECT_TIMEOUT_MS));
    if (bits & CONNECTED_BIT) {
        s_mode = NET_MODE_STA;
        return true;
    }

    ESP_LOGW(TAG, "connect to \"%s\" timed out, falling back to AP", g_net_config.ssid);
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, sta_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_event_handler);
    esp_wifi_stop();
    return false;
}

/* ---------------- AP ---------------- */

static void ap_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *ev = data;
        ESP_LOGI(TAG, "station " MACSTR " joined, aid %d", MAC2STR(ev->mac), ev->aid);
    } else if (id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *ev = data;
        ESP_LOGI(TAG, "station " MACSTR " left, aid %d", MAC2STR(ev->mac), ev->aid);
    }
}

static void ap_start(void)
{
    esp_netif_t *netif = esp_netif_create_default_wifi_ap();

    esp_netif_ip_info_t ip = { 0 };
    ip.ip.addr = ipaddr_addr(AP_IP);
    ip.gw.addr = ipaddr_addr(AP_IP);
    ip.netmask.addr = ipaddr_addr(AP_NETMASK);
    esp_netif_dhcps_stop(netif);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(netif));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, ap_event_handler, NULL));

    wifi_config_t wc = {
        .ap = {
            .channel = AP_CHANNEL,
            .max_connection = AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = { .required = false },
        },
    };
    wc.ap.ssid_len = strlen(g_dev_id);
    memcpy(wc.ap.ssid, g_dev_id, wc.ap.ssid_len);
    strlcpy((char *)wc.ap.password, AP_PASSWORD, sizeof(wc.ap.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wc));
    ESP_ERROR_CHECK(esp_wifi_start());

    s_mode = NET_MODE_AP;
    ESP_LOGI(TAG, "AP \"%s\" started, ip %s", g_dev_id, AP_IP);
}

/* ---------------- task ---------------- */

void task_net(void *arg)
{
    make_dev_id();

    s_events = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    bool connected = false;
    if (g_net_config.ssid[0])
        connected = sta_start();
    else
        ESP_LOGI(TAG, "no ssid configured");

    if (!connected)
        ap_start();

    ESP_ERROR_CHECK(http_start());

    while (1) {
        /* TODO: MQTT */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
