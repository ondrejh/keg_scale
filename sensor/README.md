# keg20 sensor bridge (MSP430FR2100)

Small MCU between the keg20 display board (ESP32-C6, header J1) and the sensors.
It reads the sensors (HX711 load cell and DS18B20 now, more later) and sends **raw
data** over the UART. The ESP does the rest: CRC checks, filtering, calibration
and conversion to units.

Status: firmware written (`sensor/fw/`), builds, **not tested on hardware yet**.

## Why

Only GPIO9, GPIO16 and GPIO17 are free on J1 (see `fw20/README.md`):

- GPIO9 is the BOOT strap and has **100 nF to GND** on the board. 1-wire can't
  work on it, and HX711 SCK needs workarounds (slow edges, read-back).
- The UART pins (GPIO16 TX, GPIO17 RX) work well.

So the sensors move to an MSP430 that talks UART. GPIO9 only drives the MSP's
RST, where slow edges don't matter. The MSP's ROM bootloader (BSL) runs over the
same UART, so the ESP can check and reflash the MSP firmware with no programmer.

## MCU

**MSP430FR2100**, TSSOP-16 (PW), datasheet SLASE78E. The pinout is the same for the whole family.

| part         | FRAM    | code space (linker) | RAM   | note                          |
|--------------|---------|---------------------|-------|-------------------------------|
| MSP430FR2000 | 0.5 KB  | 384 B               | 512 B | **too small**: firmware is ~800 B in C |
| MSP430FR2100 | 1 KB    | 896 B               | 512 B | default build, ~90 B left     |
| MSP430FR2111 | 3.75 KB | 3.6 KB              | 1 KB  | room for more sensors         |

The FR2000 could only be used with the firmware rewritten in assembly, and even then it would be tight.

Toolchain: TI MSP430-GCC 9.3 (`/usr/lib/gcc/msp430/bin/msp430-elf-gcc`, includes
the device headers and `.ld` files). The system `msp430-gcc` 4.6 (mspgcc) is too old for the FR2xxx parts.

## Wiring to J1

```
                 J1 (ESP32-C6 board)                     MSP430FR2100 (TSSOP-16)

  J1-11  ESP_TXD  GPIO16 ──────────────────┬──────────── 12  P1.6 / UCA0RXD / BSLRX
                                           └──────────── 3   TEST / SBWTCK
  J1-12  ESP_RXD  GPIO17 ─────────────────────────────── 11  P1.7 / UCA0TXD / BSLTX
  J1-4   GPIO9 (100nF on board) ──[ 1k ]──┬──────────── 4   RST / NMI / SBWTDIO
                                          └─[ 47k ]─ 3V3
  J1-6   3V3 ─────────────────────────────┬──────────── 5   DVCC
                                     100nF + 1uF
  J1-5   GND ─────────────────────────────┴──────────── 6   DVSS
```

| J1 | ESP32-C6       | MSP pin | MSP function                      |
|----|----------------|---------|-----------------------------------|
| 11 | GPIO16 (TX)    | 12, 3   | P1.6 UCA0RXD / BSLRX, **and** TEST |
| 12 | GPIO17 (RX)    | 11      | P1.7 UCA0TXD / BSLTX              |
| 4  | GPIO9          | 4       | RST/NMI (through 1k, 47k pull-up) |
| 6  | 3V3            | 5       | DVCC                              |
| 5  | GND            | 6       | DVSS                              |

- The BSL uses P1.6 (RX) and P1.7 (TX) (datasheet Table 9-4). These are also
  UCA0 RXD/TXD (USCIARMP = 0), so the app and the BSL use the same pins.
- The 47k pull-up on RST also keeps GPIO9 high while the ESP boots (strap pin).
- The 1k series resistor is a safeguard (see below). Fit 0 Ω instead if the bench test shows it isn't needed.

### TEST on the RX line – why it works

TEST is tied to the ESP TX line, which idles high. It is therefore high most of
the time and toggles with the UART data.

- TEST high only turns **Spy-Bi-Wire (SBW)** on: TEST becomes SBWTCK and RST
  becomes SBWTDIO. **The CPU keeps running** the app or the BSL. Only a debugger
  stops it, by going through the JTAG TAP.
- RST (SBWTDIO) is held high by the ESP, so TMS is 1 in every SBW slot. The TAP
  stays in Test-Logic-Reset and no JTAG instruction runs. **TEST toggling with
  UART data is ignored** while the app or the BSL runs.
- BSL entry needs RST low, so UART traffic can't trigger it by accident.
- **To reset the MSP, take TEST low first.** While SBW is on, the RST pin is
  SBWTDIO and doesn't reset the chip. Drive TX low for **≥ 100 µs** (tSBW,Ret max)
  so SBW turns off, then pull RST low.
- Each SBW cycle has a TDO slot in which the MSP may drive SBWTDIO. The **1k** limits
  contention with the ESP's push-pull high. The ESP side is held by the 100 nF, so
  the line is back at 1 for the next TMS slot.
- At power-up with TEST high, the MSP may or may not start its app. It doesn't
  matter: the ESP always takes over at boot with its own reset or BSL sequence.
- When the ESP resets, its ROM log toggles GPIO16. The MSP's RX and TEST see
  garbage; this is harmless because the app ignores RX.
- SBW debugging (TI debugger) is possible only with the J1 link disconnected.

## Sensors on the MSP

| sensor  | signal | MSP pin    | note                                         |
|---------|--------|------------|----------------------------------------------|
| HX711   | SCK    | P1.1 (1)   | push-pull output                              |
| HX711   | DOUT   | P1.0 (2)   | HX711 on 5 V → 10k series + 20k to GND divider |
| DS18B20 | DQ     | P1.2 (16)  | 4k7 pull-up to 3V3, VDD from 3V3 (non-parasitic) |
| spare   |        | P1.3, P2.0, P2.1, P2.6, P2.7 | future sensors          |

- Avoid P1.4/P1.5: they are 4-wire JTAG pins (TCK/TMS).
- The 3.3 V SCK into a 5 V HX711 is still marginal (HX711 VIH = 3.5 V at 5 V).
  If readings glitch, add a 74AHCT1G125 buffer, or power the HX711 from 3.3 V.
- The other HX711 and DS18B20 notes in `fw20/README.md` still apply: 10 SPS, gain 128,
  SCK high < 50 µs, 12-bit conversion takes 750 ms.

## Protocol (MSP → ESP)

UART **9600 8E1**, the same format as the BSL, so the ESP never reconfigures its
UART. The MSP sends ASCII lines ending in `\n`, so a plain terminal is enough for testing.

Every frame is `<type><hex payload>*<crc>\n`. The CRC is Dallas/Maxim CRC-8
(reflected polynomial 0x8C, init 0) over every byte before `*`, sent as 2
uppercase hex digits. The same CRC function checks the DS18B20 scratchpad on the MSP.

| frame         | when                          | content                                        |
|---------------|-------------------------------|------------------------------------------------|
| `V<2 hex>*cc` | once after reset              | firmware version                               |
| `H<6 hex>*cc` | each HX711 conversion (~10/s) | channel A raw value, 24-bit two's complement, MSB first |
| `h<2 hex>*cc` | instead of `H`                | HX711 error: `01` not ready for 500 ms (missing), `02` DOUT still low after the 25th pulse (unpowered: the divider holds DOUT low) |
| `T<4 hex>*cc` | ~1/s                          | DS18B20 raw temperature, int16, 1/16 °C (scratchpad CRC already checked) |
| `t<2 hex>*cc` | instead of `T`                | DS18B20 error: `01` bus stuck low, `02` no presence pulse, `03` scratchpad CRC error |

Examples: `V01*67`, `H00A1B2*4D`, `T0550*45` (85 °C). `sensor/fw/tools/decode.py`
decodes and checks frames.

- The ESP drops any frame that is malformed, too long or has a bad CRC (for example garbage after a reset).
- The MSP app has **no RX handling**. BSL entry is done in hardware.
- ~100 B/s in total, well within 9600 baud (~870 B/s with 8E1).

## ESP boot flow (planned, not coded yet)

The MSP image is embedded in the fw20 image, so one `idf.py flash` updates both chips.

1. **BSL entry** (GPIO9 and GPIO16 as plain GPIO):
   TX low and RST low (TX low ≥ 100 µs first) → TX high, low, high (two
   rising edges on TEST) → RST high while TX is still high → TX low.
   Hand GPIO16 back to the UART (9600 8E1).
2. **Firmware check:** the BSL password is the 32-byte interrupt vector table
   (0xFFE0–0xFFFF). The ESP sends the vectors of its embedded image.
   - Password accepted → BSL *CRC check* (0x16) over the code area. If it
     matches the embedded image, the firmware is OK.
   - Password wrong → the FRAM BSL mass-erases. Then send the password of
     the erased chip (32 × 0xFF), write the image with *RX data block* (0x10;
     ~800 B, about 1 s at 9600) and check the CRC.
3. **Start the app:** TX low ≥ 100 µs, RST low → high (TEST low), wait for the
   `V<xx>` line and start reading `H`/`T` lines.
4. It can't brick the MSP: hardware BSL entry doesn't depend on the app, so an
   interrupted update just retries on the next boot.

BSL frame format: `0x80 NL NH CMD data… CKL CKH`, with CRC-CCITT over CMD + data.
The BSL answers with an ACK byte (0x00), then a `0x80 …` response frame.

## MSP firmware (`sensor/fw/`)

```bash
cd sensor/fw
make                        # MSP430FR2100 (default): build/sensor.elf .hex .bin .lst, prints the size
make MCU=msp430fr2111       # other part of the family
python3 tools/decode.py --selftest
```

- `main.c`: a single superloop with no interrupts. The watchdog runs at 1 s (ACLK = REFO).
  The DCO runs at 8 MHz with the FLL locked to REFO. UCA0 runs 9600 8E1 on P1.6/P1.7.
- Main loop tick is about 10 ms:
  - HX711: when DOUT goes low it reads 24 + 1 pulses (channel A, gain 128; SCK high
    ~1 µs). After the 25th pulse DOUT must be high, else error `h02`. No conversion
    for 50 ticks gives error `h01`.
  - DS18B20: a 100-tick cycle. Tick 0 sends `CC 44` (convert). Tick 80 (after 800 ms)
    sends `CC BE` and reads 9 bytes, then checks the CRC.
  - 1-wire is bit-banged open drain using P1DIR, with timings from Maxim AN126:
    reset 480/70/410 µs, slot ~60 µs, read sample at ~10 µs.
- Size tricks:
  - native 16-bit types
  - all frames go through one `frame()` function
  - globals in `.noinit`, so no bss init code is linked
  - a single asm delay loop instead of many inline `__delay_cycles`
- `compile_flags.txt` is for clangd (editor diagnostics) only.

## Standalone test

Use a 3.3 V USB-UART adapter (FTDI/CP2102) instead of the ESP:

| adapter | MSP                    |
|---------|------------------------|
| TXD     | P1.6 + TEST (pin 12, 3) |
| RXD     | P1.7 (pin 11)           |
| RTS     | RST through 1k (pin 4)  |
| 3V3/GND | DVCC/DVSS               |

- **Monitor:** `python3 sensor/fw/tools/decode.py /dev/ttyUSB0` (or
  `picocom -b 9600 -y e /dev/ttyUSB0`). Expect `V01`, then `H…` frames at ~10/s
  and `T…` at ~1/s. Load the cell and warm the sensor;
  the values should react.
- **Flash:** `python3 sensor/fw/tools/bsl.py flash /dev/ttyUSB0 sensor/fw/build/sensor.hex`
  (pyserial).
  - It drives RST from RTS and makes the TEST pulses on TX with a UART break
    (break = TX low).
  - It runs the same algorithm as the ESP boot flow: the image's vector table is
    the password, a matching CRC means "up to date" and the write is skipped,
    otherwise it erases, writes and verifies with the CRC check.
  - It then resets the MSP into the app.
  - Other commands: `bsl.py erase PORT`, `bsl.py reset PORT`.
  - Options: `--force` (flash even if up to date), `--no-reset`, `-v` (packet
    dump), `--test dtr` (TEST wired to DTR instead of TX),
    `--invert-reset`/`--invert-test`.
  - `bsl.py --selftest` runs the flash flow against a simulated BSL.
  - The adapter must support break (FTDI and CP210x do).

## Open points (verify on the bench / in SLAU610)

- [ ] Exact FR2xx BSL entry timing (pulse widths, TEST high → RST high delay)
- [ ] Wrong BSL password → mass erase on FR2100?
- [ ] BSL CRC check command and maximum RX data block size on FR2100
- [ ] App keeps running with TEST toggling (scope: no resets, no stalls), and the
      BSL keeps working with TEST high
- [ ] Is the 1k on RST needed (look for contention in the TDO slot)?
- [ ] REFO / DCO accuracy for 9600 baud over the keg temperature range
- [ ] Check timing on the scope: HX711 SCK high < 50 µs, 1-wire slots and the read sample point
- [ ] Unplug each sensor and check the error frames (`h01`/`h02`, `t01`/`t02`)
