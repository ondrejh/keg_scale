#include "flash_data.h"
#include "config.h"

net_config_t g_net_config;

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
