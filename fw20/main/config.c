#include "flash_data.h"
#include "config.h"
#include "settings.h"

net_config_t g_net_config;
mqtt_config_t g_mqtt_config;
disp_config_t g_disp_config = {
    .idle_ms = IDLE_DIM_MS, .return_ms = RETURN_MAIN_MS, .dim = BRIGHT_DIM,
};

esp_err_t config_load(void)
{
    esp_err_t err = flash_data_load(CONFIG_SLOT_NET, NET_CONFIG_VERSION,
                                    &g_net_config, sizeof(g_net_config));
    if (err == ESP_OK) {
        /* guarantee termination even if stored data is odd */
        g_net_config.ssid[sizeof(g_net_config.ssid) - 1] = '\0';
        g_net_config.password[sizeof(g_net_config.password) - 1] = '\0';
    }
    return err;
}

esp_err_t config_save(void)
{
    return flash_data_save(CONFIG_SLOT_NET, NET_CONFIG_VERSION,
                           &g_net_config, sizeof(g_net_config));
}

esp_err_t mqtt_config_load(void)
{
    esp_err_t err = flash_data_load(CONFIG_SLOT_MQTT, MQTT_CONFIG_VERSION,
                                    &g_mqtt_config, sizeof(g_mqtt_config));
    if (err == ESP_OK) {
        g_mqtt_config.url[sizeof(g_mqtt_config.url) - 1] = '\0';
        g_mqtt_config.topic[sizeof(g_mqtt_config.topic) - 1] = '\0';
        g_mqtt_config.user[sizeof(g_mqtt_config.user) - 1] = '\0';
        g_mqtt_config.password[sizeof(g_mqtt_config.password) - 1] = '\0';
    }
    return err;
}

esp_err_t mqtt_config_save(void)
{
    return flash_data_save(CONFIG_SLOT_MQTT, MQTT_CONFIG_VERSION,
                           &g_mqtt_config, sizeof(g_mqtt_config));
}

esp_err_t disp_config_load(void)
{
    disp_config_t dc;
    esp_err_t err = flash_data_load(CONFIG_SLOT_DISP, DISP_CONFIG_VERSION, &dc, sizeof(dc));
    if (err == ESP_OK)
        g_disp_config = dc;
    return err;     /* on error g_disp_config keeps the defaults */
}

esp_err_t disp_config_save(void)
{
    return flash_data_save(CONFIG_SLOT_DISP, DISP_CONFIG_VERSION,
                           &g_disp_config, sizeof(g_disp_config));
}
