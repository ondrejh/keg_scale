/*
 * Shared scale state: calibration, keg and the latest measurement.
 * Written by task_scale, read by others (http, display) via snapshot.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define CALIB_MAX       5
#define CALIB_MIN       2           /* can't delete below this */
#define UNIT_STR_MAX    8
#define KEG_NAME_MAX    32

typedef struct {
    uint8_t n;                      /* calibration points used */
    int32_t x[CALIB_MAX];           /* raw, sorted ascending */
    float y[CALIB_MAX];             /* primary units */
    float ratio;                    /* primary units per secondary unit (kg per beer) */
    char uprim[UNIT_STR_MAX + 1];
    char usec[UNIT_STR_MAX + 1];
} scale_calib_t;

typedef struct {
    bool tapped;
    char name[KEG_NAME_MAX + 1];
    float volume;                   /* secondary units */
    int32_t fullraw;                /* raw reading when tapped */
} scale_keg_t;

typedef struct {
    int32_t raw;
    float units;
    bool temp_valid;
    float temp;                     /* deg C */
    scale_calib_t calib;
    scale_keg_t keg;
    float keg_left;                 /* secondary units */
} scale_data_t;

/* Load defaults. Call once before any task uses the scale state. */
void scale_init(void);

float scale_interpolate(const scale_calib_t *c, int32_t raw);

/* Factory calibration. */
void scale_calib_default(scale_calib_t *c);

/* Calibration point `units` at raw (NULL = current reading). An existing point
 * with the same units is re-measured, otherwise a new one is added. False when
 * the table is full. Saved to flash, restored by scale_init(). */
bool scale_calib_set(float units, const int32_t *raw);

/* Delete point at index (0-based, table order = data.json order). False if out
 * of range or only CALIB_MIN points left. */
bool scale_calib_delete(int index);

/* Store new measurement and recompute derived values. */
void scale_update(int32_t raw, bool temp_valid, float temp);

/* Copy current state. */
void scale_get(scale_data_t *out);

/* Inverse of scale_interpolate: raw reading for a weight (extrapolates). */
int32_t scale_units_to_raw(const scale_calib_t *c, float units);

/* Tap a keg: full at the current reading, `volume` in secondary units.
 * Tap/untap state is saved to flash and restored by scale_init(). */
void scale_keg_tap(const char *name, float volume);
void scale_keg_untap(void);

/* Keg fullness 0..100 %, 0 when no keg is tapped. */
int scale_keg_pct(const scale_data_t *d);
