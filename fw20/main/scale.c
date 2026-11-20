#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "scale.h"

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
static void calib_default(scale_calib_t *c)
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
    calib_default(&s_data.calib);
    /* TODO: load calibration and keg from flash */
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

void scale_update(int32_t raw, bool temp_valid, float temp)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_data.raw = raw;
    s_data.units = scale_interpolate(&s_data.calib, raw);
    s_data.temp_valid = temp_valid;
    s_data.temp = temp;
    if (s_data.keg.tapped && s_data.calib.ratio > 0.0f) {
        float full = scale_interpolate(&s_data.calib, s_data.keg.fullraw);
        s_data.keg_left = s_data.keg.volume - (full - s_data.units) / s_data.calib.ratio;
    }
    xSemaphoreGive(s_lock);
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
    xSemaphoreGive(s_lock);
}

void scale_keg_untap(void)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_data.keg.tapped = false;
    s_data.keg_left = 0.0f;
    xSemaphoreGive(s_lock);
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
