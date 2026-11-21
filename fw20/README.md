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
| 2  | VSYS    | –        | USB 5 V rail, unregulated                    |
| 3  | CHIP_EN | EN       | reset – leave unconnected                                   |
| 4  | GPIO9   | GPIO9    | strapping (must be high at boot) + BOOT button, 100 nF to GND; free in FW |
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
| HX711 PD_SCK | 4  | GPIO9    | push-pull output; strap pull-up holds SCK high at boot → HX711 just powers down, firmware resets it at init |
| DS18B20 DQ   | 11 | GPIO16   | open-drain 1-wire; ROM log toggling U0TXD at reset is harmless |

GPIO9 (BOOT) has a **100 nF capacitor** to GND on the board. With the 4.7 kΩ
1-wire pull-up that is RC ≈ 470 µs, while 1-wire samples each bit ~15 µs after
the edge – 1-wire cannot work on GPIO9. A push-pull output can drive it (edges
of ~10 µs); `hx711.c` reads the SCK pin back and waits for each edge, keeping
every high phase < 50 µs. Don't press BOOT while running – it shorts the SCK
output to GND.

Long sensor cables: ~100 Ω series resistors on DQ/SCK/DOUT near J1 (+ ESD diode).

HX711 notes:
- Module (VCC and VDD) powered from **5 V**. DOUT is then a 5 V signal →
  **resistor divider** into GPIO17 (the ESP32-C6 is not 5 V tolerant):
  10 kΩ series (DOUT → GPIO17) + 20 kΩ GPIO17 → GND → 5 V × 20/30 = 3.33 V.
  The 20 kΩ is also the GPIO17 pull-down – fine, GPIO17 is not a strapping pin.
  Side effect: with the HX711 unpowered/unplugged DOUT reads low = "ready" and
  returns all zeros – firmware should treat a constant 0 as "sensor missing".
- The divider needs a **stable 5 V**: ESP VIH ≈ 0.75 × 3.3 V = 2.5 V → the
  HX711 supply must stay above ~3.7 V.
- **SCK (GPIO9 → HX711) is not shifted**: HX711 VIH = 0.7 × VDD = 3.5 V at 5 V,
  a 3.3 V output is below spec. Often works in practice, but marginal – if
  readings glitch, add a 5 V buffer with TTL inputs (74HCT1G125 / 74AHCT1G125)
  or a BSS138 stage on SCK.
- Keep RATE low (10 SPS, default on most modules) – less noise, plenty for a keg.
- Channel A, gain 128 (25 clock pulses per read). Bit-bang SCK, keep each high
  pulse < 50 µs (short critical section around each high phase) – SCK high
  > 60 µs = power-down.
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
#define BOARD_HX711_SCK         GPIO_NUM_9
#define BOARD_DS18B20_DQ        GPIO_NUM_16
```
