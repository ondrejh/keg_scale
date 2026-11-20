#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "config.h"
#include "flash_data.h"
#include "scale.h"

static const char *TAG = "scale";

#define KEG_VERSION     1       /* bump when scale_keg_t changes */
#define CALIB_VERSION   1       /* bump when scale_calib_t changes */

static scale_data_t s_data;
static SemaphoreHandle_t s_lock;
static StaticSemaphore_t s_lock_buf;

static void calib_sort(scale_calib_t *c)
{
    for (int i = 1; i < c->n; i++) {
        for (int j = i; j > 0 && c->x[j - 1] > c->x[j]; j--) {
            int32_t x = c->x[j]; c->x[j] = c->x[j - 1]; c->x[j - 1] = x;
            float y = c->y[j];   c->y[j] = c->y[j - 1]; c->y[j - 1] = y;
        }
    }
}

/* Defaults from the original sketch (set_calib_default) */
void scale_calib_default(scale_calib_t *c)
{
    memset(c, 0, sizeof(*c));
    c->n = 2;
    c->x[0] = -18689192; c->y[0] = 30.0f;
    c->x[1] = -7110488;  c->y[1] = 0.0f;
    c->ratio = 0.5f;     /* 1 piv = 0.5 kg */
    strcpy(c->uprim, "kg");
    strcpy(c->usec, "piv");
    calib_sort(c);
}

void scale_init(void)
{
    s_lock = xSemaphoreCreateMutexStatic(&s_lock_buf);
    memset(&s_data, 0, sizeof(s_data));

    scale_calib_t cal;
    esp_err_t cerr = flash_data_load(CONFIG_SLOT_CALIB, CALIB_VERSION, &cal, sizeof(cal));
    if (cerr == ESP_OK && cal.n >= 2 && cal.n <= CALIB_MAX) {
        cal.uprim[sizeof(cal.uprim) - 1] = '\0';
        cal.usec[sizeof(cal.usec) - 1] = '\0';
        calib_sort(&cal);
        s_data.calib = cal;
        ESP_LOGI(TAG, "calibration restored (%d points)", cal.n);
    } else {
        if (cerr != ESP_ERR_NOT_FOUND)
            ESP_LOGW(TAG, "calibration load failed (%s), defaults", esp_err_to_name(cerr));
        scale_calib_default(&s_data.calib);
    }

    scale_keg_t keg;
    esp_err_t err = flash_data_load(CONFIG_SLOT_KEG, KEG_VERSION, &keg, sizeof(keg));
    if (err == ESP_OK) {
        keg.name[sizeof(keg.name) - 1] = '\0';
        s_data.keg = keg;
        if (keg.tapped)
            ESP_LOGI(TAG, "keg \"%s\" (%.1f) restored", keg.name, keg.volume);
    } else if (err != ESP_ERR_NOT_FOUND) {
        ESP_LOGW(TAG, "keg load failed (%s)", esp_err_to_name(err));
    }
}

static void calib_save(const scale_calib_t *c)
{
    esp_err_t err = flash_data_save(CONFIG_SLOT_CALIB, CALIB_VERSION, c, sizeof(*c));
    if (err != ESP_OK)
        ESP_LOGE(TAG, "calibration save failed (%s)", esp_err_to_name(err));
}

/* Persist keg state (call without the lock held; flash erase is slow). */
static void keg_save(const scale_keg_t *keg)
{
    esp_err_t err = flash_data_save(CONFIG_SLOT_KEG, KEG_VERSION, keg, sizeof(*keg));
    if (err != ESP_OK)
        ESP_LOGE(TAG, "keg save failed (%s)", esp_err_to_name(err));
}

/* Piecewise linear, extrapolates past the end points (as the sketch did). */
float scale_interpolate(const scale_calib_t *c, int32_t raw)
{
    if (c->n < 2)
        return 0.0f;
    int i;
    for (i = 1; i < c->n - 1; i++) {
        if (raw < c->x[i])
            break;
    }
    int32_t x1 = c->x[i - 1], x2 = c->x[i];
    float y1 = c->y[i - 1], y2 = c->y[i];
    if (x2 == x1)
        return y1;
    return y1 + (y2 - y1) * (float)(raw - x1) / (float)(x2 - x1);
}

/* Derived values from raw + calibration + keg. Lock held. */
static void recompute(void)
{
    s_data.units = scale_interpolate(&s_data.calib, s_data.raw);
    if (s_data.keg.tapped && s_data.calib.ratio > 0.0f) {
        float full = scale_interpolate(&s_data.calib, s_data.keg.fullraw);
        float left = s_data.keg.volume - (full - s_data.units) / s_data.calib.ratio;
        s_data.keg_left = left > 0.0f ? left : 0.0f;   /* noise near empty, no "-0.0" */
    }
}

void scale_update(int32_t raw, bool temp_valid, float temp)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_data.raw = raw;
    s_data.temp_valid = temp_valid;
    s_data.temp = temp;
    recompute();
    xSemaphoreGive(s_lock);
}

bool scale_calib_set(float units, const int32_t *raw)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    scale_calib_t *c = &s_data.calib;
    int32_t x = raw ? *raw : s_data.raw;
    int i;
    for (i = 0; i < c->n; i++) {
        if (c->y[i] == units)
            break;          /* same value: re-measure that point */
    }
    if (i == c->n) {
        if (c->n >= CALIB_MAX) {
            xSemaphoreGive(s_lock);
            return false;
        }
        c->n++;
    }
    c->x[i] = x;
    c->y[i] = units;
    calib_sort(c);
    recompute();
    scale_calib_t copy = *c;
    xSemaphoreGive(s_lock);

    calib_save(&copy);
    ESP_LOGI(TAG, "calib: %.2f = %ld (%d points)", units, (long)x, copy.n);
    return true;
}

bool scale_calib_delete(int index)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    scale_calib_t *c = &s_data.calib;
    if (index < 0 || index >= c->n || c->n <= CALIB_MIN) {
        xSemaphoreGive(s_lock);
        return false;
    }
    float y = c->y[index];
    for (int i = index; i < c->n - 1; i++) {
        c->x[i] = c->x[i + 1];
        c->y[i] = c->y[i + 1];
    }
    c->n--;
    recompute();
    scale_calib_t copy = *c;
    xSemaphoreGive(s_lock);

    calib_save(&copy);
    ESP_LOGI(TAG, "calib: %.2f deleted (%d points)", y, copy.n);
    return true;
}

void scale_get(scale_data_t *out)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    *out = s_data;
    xSemaphoreGive(s_lock);
}

int32_t scale_units_to_raw(const scale_calib_t *c, float units)
{
    if (c->n < 2)
        return 0;
    /* points are sorted by raw; units may run either way, so search segments
     * in raw order and use the first one that brackets the value */
    int seg = c->n - 2;
    for (int i = 0; i < c->n - 1; i++) {
        float lo = c->y[i] < c->y[i + 1] ? c->y[i] : c->y[i + 1];
        float hi = c->y[i] < c->y[i + 1] ? c->y[i + 1] : c->y[i];
        if (units >= lo && units <= hi) {
            seg = i;
            break;
        }
    }
    float y1 = c->y[seg], y2 = c->y[seg + 1];
    int32_t x1 = c->x[seg], x2 = c->x[seg + 1];
    if (y2 == y1)
        return x1;
    return x1 + (int32_t)((float)(x2 - x1) * (units - y1) / (y2 - y1));
}

void scale_keg_tap(const char *name, float volume)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    scale_keg_t *k = &s_data.keg;
    snprintf(k->name, sizeof(k->name), "%s", name);
    k->volume = volume;
    k->fullraw = s_data.raw;
    k->tapped = true;
    s_data.keg_left = volume;
    scale_keg_t copy = *k;
    xSemaphoreGive(s_lock);
    keg_save(&copy);
}

void scale_keg_untap(void)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_data.keg.tapped = false;
    s_data.keg_left = 0.0f;
    scale_keg_t copy = s_data.keg;
    xSemaphoreGive(s_lock);
    keg_save(&copy);
}

int scale_keg_pct(const scale_data_t *d)
{
    if (!d->keg.tapped || d->keg.volume <= 0.0f)
        return 0;
    float pct = d->keg_left * 100.0f / d->keg.volume;
    if (pct < 0.0f)
        return 0;
    if (pct > 100.0f)
        return 100;
    return (int)(pct + 0.5f);
}
