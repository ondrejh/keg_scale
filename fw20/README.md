# keg20 firmware (fw20)

ESP-IDF v5.5.6 project for the **Waveshare ESP32-C6-Touch-AMOLED-1.32**
(ESP32-C6, 16 MB flash, 466x466 CO5300 AMOLED, CST820 touch, ES8311 audio).

Self-contained: ESP-IDF is a git submodule (`fw20/esp-idf`) and its tools are
installed into `fw20/.espressif` (gitignored), independent of any system IDF.

## Setup (once)

```bash
git clone --recursive git@github.com:ondrejh/keg_scale.git   # or: git submodule update --init --recursive
cd keg_scale/fw20
./install.sh
```

## Build / flash

```bash
cd fw20
. ./env.sh
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

## Layout

- `main/board.h` – board pin map (from Waveshare demos)
- `partitions.csv` – nvs, 2x 4 MB OTA app slots, ~8 MB LittleFS `storage`
- `tools/mkfont.py` – generates the big LVGL fonts in `main/fonts/` from LVGL's
  Montserrat TTF (system python3 + Pillow, no node needed)
- `tools/mkbg.py` – cuts the display gauge textures (`main/img/*`) out of
  `web20/img/bg.jpg`; rerun with system python3 (Pillow, numpy) after it changes
- `../web20` – web UI; packed into the `storage` LittleFS image at build time and
  flashed together with `idf.py flash`
- `sdkconfig.defaults` – target, flash size, partition table

## External sensors – suggested wiring (HX711 load cell, DS18B20)

Sensors connect to the board's 12-pin header **J1** (`schematic/keg20`).
Verify the numbering against the Waveshare schematic / silkscreen.

| J1 | net     | ESP32-C6 | note                                                        |
|----|---------|----------|-------------------------------------------------------------|
| 1  | GND     | –        |                                                             |
| 2  | VSYS    | –        | battery / USB rail ~3.4–5 V, unregulated                    |
| 3  | CHIP_EN | EN       | reset – leave unconnected                                   |
| 4  | GPIO9   | GPIO9    | strapping (must be high at boot) + BOOT button; free in FW  |
| 5  | GND     | –        |                                                             |
| 6  | 3V3     | –        | regulated 3.3 V                                             |
| 7  | SCL     | GPIO7    | I2C0, shared with touch CST820 (0x15) and ES8311            |
| 8  | SDA     | GPIO8    | I2C0 (strapping pin)                                        |
| 9  | USB1_N  | GPIO12   | USB D- – **do not use** (flashing + console)                |
| 10 | USB1_P  | GPIO13   | USB D+ – **do not use**                                     |
| 11 | ESP_TXD | GPIO16   | U0TXD, free (console is USB-Serial-JTAG); ROM log toggles it at reset |
| 12 | ESP_RXD | GPIO17   | U0RXD, free; input at boot                                  |

Only GPIO9, GPIO16 and GPIO17 are free (plus the shared I2C bus). GPIO14 is
**not** brought out to J1.

Assignment:

| signal       | J1 | ESP32-C6 | why                                                          |
|--------------|----|----------|--------------------------------------------------------------|
| HX711 DOUT   | 12 | GPIO17   | input during boot → no contention with the HX711 output      |
| HX711 PD_SCK | 11 | GPIO16   | ROM log at reset idles high → HX711 just powers down; firmware resets it at init (SCK high > 60 µs, then low). Optionally `CONFIG_BOOT_ROM_LOG_ALWAYS_OFF` |
| DS18B20 DQ   | 4  | GPIO9    | 1-wire is open-drain, pull-up keeps the strap high (SPI boot); pressing BOOT just shorts the bus, harmless |

Not the other way round: a push-pull HX711 SCK on GPIO9 would short to GND
whenever BOOT is pressed, and the strap pull-up would hold the HX711 in
power-down at boot.

```
           J1-6 3V3 ──┬────────────┬──────────────┐
                      │            │             4k7
              ┌───────┴──────┐  ┌──┴─────────┐    │
              │ HX711 module │  │  DS18B20   │    │
 load cell    │          VCC │  │ VDD        │    │
 E+ (red)  ───┤ E+       VDD │  │         DQ ├────┴──── J1-4  GPIO9
 E- (blk)  ───┤ E-      DOUT ├──┼──────────────────── J1-12 GPIO17
 A- (wht)  ───┤ A-      SCK  ├──┼──────────────────── J1-11 GPIO16
 A+ (grn)  ───┤ A+       GND │  │ GND        │
              └───────┬──────┘  └──┬─────────┘
           J1-1 GND ──┴────────────┴─────────────────────
```

Long sensor cables: ~100 Ω series resistors on DQ/SCK/DOUT near J1 (+ ESD diode).

HX711 notes:
- Power the module from **3V3** (VCC and VDD both on 3V3 – logic levels then
  match the ESP32-C6, no level shifter). On typical modules the on-board AVDD
  regulator is set for ~4.3 V, so at 3V3 it just saturates (excitation ≈ 3.1 V) –
  works fine, slightly lower full-scale signal.
- Keep RATE low (10 SPS, default on most modules) – less noise, plenty for a keg.
- Channel A, gain 128 (25 clock pulses per read). Bit-bang SCK, keep each high
  pulse < 50 µs (disable preemption/IRQs during the 24-bit read, or use a short
  critical section) – SCK high > 60 µs = power-down.
- Power saving on battery: hold SCK high between readings → HX711 sleeps
  (~1 µA); the first reading after wake-up should be discarded.
- Load cell wire colors vary between vendors – if the reading goes negative with
  load, swap A+/A-.

DS18B20 notes:
- Use normal (non-parasitic) power: VDD to 3V3, 4.7 kΩ pull-up DQ→3V3.
  Long cable (> ~5 m): lower pull-up to 2.2 kΩ.
- ESP-IDF: `espressif/ds18b20` + `espressif/onewire_bus` components (RMT based).
- 12-bit conversion takes 750 ms – start conversion, read on the next cycle,
  don't block the scale task.

Pins in `main/board.h`:

```c
/* ---- External sensors ---- */
#define BOARD_HX711_DOUT        GPIO_NUM_17
#define BOARD_HX711_SCK         GPIO_NUM_16
#define BOARD_DS18B20_DQ        GPIO_NUM_9
```
