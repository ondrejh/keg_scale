#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "http.h"
#include "scale.h"
#include "keglist.h"

static const char *TAG = "keglist";

#define PATH        WWW_BASE "/keglist.txt"
#define PATH_TMP    WWW_BASE "/keglist.tmp"
#define LINE_MAX    (KEG_NAME_MAX * 2)  /* tolerate longer lines in the seed file */

esp_err_t keglist_add(const char *name)
{
    if (!name[0])
        return ESP_ERR_INVALID_ARG;

    FILE *out = fopen(PATH_TMP, "w");
    if (!out) {
        ESP_LOGE(TAG, "can't write " PATH_TMP);
        return ESP_FAIL;
    }
    fprintf(out, "%s\n", name);
    int n = 1;

    FILE *in = fopen(PATH, "r");
    if (in) {
        char line[LINE_MAX + 2];
        while (n < KEGLIST_MAX && fgets(line, sizeof(line), in)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (line[0] && strcmp(line, name) != 0) {
                fprintf(out, "%s\n", line);
                n++;
            }
        }
        fclose(in);
    }

    if (fclose(out) != 0) {
        ESP_LOGE(TAG, "write failed");
        remove(PATH_TMP);
        return ESP_FAIL;
    }
    remove(PATH);
    if (rename(PATH_TMP, PATH) != 0) {
        ESP_LOGE(TAG, "rename failed");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "\"%s\" added (%d names)", name, n);
    return ESP_OK;
}
