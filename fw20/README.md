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
- `sdkconfig.defaults` – target, flash size, partition table
