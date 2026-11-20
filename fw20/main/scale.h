/*
 * Shared scale state: calibration, keg and the latest measurement.
 * Written by task_scale, read by others (http, display) via snapshot.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define CALIB_MAX       5
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

/* Store new measurement and recompute derived values. */
void scale_update(int32_t raw, bool temp_valid, float temp);

/* Copy current state. */
void scale_get(scale_data_t *out);

/* Inverse of scale_interpolate: raw reading for a weight (extrapolates). */
int32_t scale_units_to_raw(const scale_calib_t *c, float units);

/* Tap a keg: full at the current reading, `volume` in secondary units. */
void scale_keg_tap(const char *name, float volume);
void scale_keg_untap(void);

/* Keg fullness 0..100 %, 0 when no keg is tapped. */
int scale_keg_pct(const scale_data_t *d);

/* SIMULATION (task_scale.c) - remove together with the fake readings once the
 * HX711 driver exists. Full keg = tapped at 100 %, off = untapped, empty keg. */
void scale_sim_keg_full(void);
void scale_sim_keg_off(void);
/* Put a full keg of `volume` beers on the (simulated) scale, not tapped yet. */
void scale_sim_keg_load(float volume);
