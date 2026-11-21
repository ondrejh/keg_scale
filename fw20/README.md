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

Free GPIOs on the board (everything else is taken, see `main/board.h`):

| GPIO | default role        | note                                                  |
|------|---------------------|-------------------------------------------------------|
| 14   | –                   | plain GPIO, not strapping                             |
| 16   | U0TXD               | UART0 free (console is USB-Serial-JTAG); ROM boot log toggles it at reset |
| 17   | U0RXD               | UART0 free; input at boot                             |
| 12/13| USB D-/D+           | **do not use** – flashing + console                   |

Avoid strapping pins (4, 5, 8, 9, 15) – all already used anyway.
Check the board schematic / header pinout that 14/16/17 are actually brought out.

Suggested assignment:

| signal       | ESP32-C6 | why                                                          |
|--------------|----------|--------------------------------------------------------------|
| HX711 DOUT   | GPIO17   | input; GPIO17 is input during boot → no contention with HX711 output |
| HX711 PD_SCK | GPIO14   | output; quiet during boot (GPIO16 would toggle from ROM log and could put the HX711 into power-down / clock garbage) |
| DS18B20 DQ   | GPIO16   | 1-wire open-drain; ROM log pulses at boot are harmless (no valid ROM command) |

```
               3V3 ──┬────────────┬──────────────┐
                     │            │             4k7
             ┌───────┴──────┐  ┌──┴─────────┐    │
             │ HX711 module │  │  DS18B20   │    │
 load cell   │          VCC │  │ VDD        │    │
 E+ (red)  ──┤ E+       VDD │  │         DQ ├────┴──── GPIO16
 E- (blk)  ──┤ E-      DOUT ├──┼──────────── GPIO17
 A- (wht)  ──┤ A-      SCK  ├──┼──────────── GPIO14
 A+ (grn)  ──┤ A+       GND │  │ GND        │
             └───────┬──────┘  └──┬─────────┘
               GND ──┴────────────┴───────────────────── GND
```

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

Add the pins to `main/board.h` once confirmed:

```c
/* ---- External sensors ---- */
#define BOARD_HX711_DOUT        GPIO_NUM_17
#define BOARD_HX711_SCK         GPIO_NUM_14
#define BOARD_DS18B20_DQ        GPIO_NUM_16
```
