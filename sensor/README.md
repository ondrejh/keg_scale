# ToDo

- [x] wire MSP430FR2311 and test do BSL tests (2/3 wire)
  - test_fr2311 folder
- [ ] FR2311: power-up with TEST high, 3 wires and one wire
  - `test_fr2311/host_fr.py /dev/ttyUSB0 power-test` (one wire: `--onewire`), asks to remove the chip's 3V3
- [ ] FR2311: one wire with J9 removed (TCK open); passed with 3 wires
- [ ] FR2311: RST glitch during a frame with one wire, does it reset the MSP? (one unexplained reset)
- [ ] FR2111: entry, flash, 3 wires and one wire on the real chip (other BSL version than the FR2311)
- [ ] on the first board: the rest of [Open points](#open-points-verify-on-the-bench--in-slau610)

# keg20 sensor bridge (MSP430FR2111 + ADS1231 + DS18B20)

Small MCU between the keg20 display board (ESP32-C6, header J1) and the sensors.
It reads the sensors (load cell through an ADS1231, DS18B20, more later) and sends
**raw data** over the UART. The ESP does the rest: CRC checks, filtering,
calibration and conversion to units.

Schematic: `schematic/keg20/sensor/` (KiCad 10). Reference designators below are from it.

Status:

- Schematic drawn, ERC clean. No PCB yet, the FR2111 itself not tested.
- The link (TEST on RX, BSL entry, flashing, reset) is tested with 3 wires and with one wire on an
  FR2311, which has the same BSL pins ([test_fr2311](test_fr2311/README.md)), before that on a
  G2553 ([test_g2553](test_g2553/README.md)).
- Firmware (`sensor/fw/`) builds, but it is still written for the **HX711**. With the
  ADS1231 it **must drive PDWN (P1.3) high**, see [ADS1231](#ads1231).

## Why

Only GPIO9, GPIO16 and GPIO17 are free on J1 (see `fw20/README.md`):

- GPIO9 is the BOOT strap and has **100 nF to GND** on the board. 1-wire can't
  work on it, and HX711 SCK needs workarounds (slow edges, read-back).
- The UART pins (GPIO16 TX, GPIO17 RX) work well.

So the sensors move to an MSP430 that talks UART. GPIO9 only drives the MSP's
RST, where slow edges don't matter. The MSP's ROM bootloader (BSL) runs over the
same UART, so the ESP can check and reflash the MSP firmware with no programmer.

## MCU

**MSP430FR2111** (U1), TSSOP-16 (PW), datasheet SLASE78E. The pinout is the same for the
whole family, so an FR2100 also fits the board.

| part         | FRAM    | code space (linker) | RAM   | note                          |
|--------------|---------|---------------------|-------|-------------------------------|
| MSP430FR2000 | 0.5 KB  | 384 B               | 512 B | **too small**: firmware is ~800 B in C |
| MSP430FR2100 | 1 KB    | 896 B               | 512 B | default `make`, ~90 B left    |
| MSP430FR2111 | 3.75 KB | 3.6 KB              | 1 KB  | **on the schematic**, room for more sensors |

The FR2000 could only be used with the firmware rewritten in assembly, and even then it would be tight.

Toolchain: TI MSP430-GCC 9.3 (`/usr/lib/gcc/msp430/bin/msp430-elf-gcc`, includes
the device headers and `.ld` files). The system `msp430-gcc` 4.6 (mspgcc) is too old for the FR2xxx parts.

## Connectors and jumpers

J1 connects to the keg20 board header J1 (`fw20/README.md`). The pin numbers are **not** the same:

| J1 | net     | keg20 J1 | ESP32-C6    | MSP pin | function                                          |
|----|---------|----------|-------------|---------|---------------------------------------------------|
| 1  | ESP_3V3 | 6        | 3V3         | –       | board 3V3, through JP1 1-2                        |
| 2  | ESP_RXD | 12       | GPIO17 (RX) | 11      | P1.7 UCA0TXD / BSLTX, through R6 330R             |
| 3  | ESP_TXD | 11       | GPIO16 (TX) | 12, 3   | P1.6 UCA0RXD / BSLRX **and** TEST, through R5 330R, TEST through JP3 |
| 4  | ESP_RST | 4        | GPIO9       | 4       | RST/NMI, through R1 330R, pull-up R2 4k7          |
| 5  | GND     | 5        | GND         | 6       | DVSS                                              |
| 6  | +5V     | 2 (VSYS) | –           | –       | LDO U4 (ADC and bridge supply)                    |

| connector | use                                                              |
|-----------|------------------------------------------------------------------|
| J2        | 4-wire load cell: 1 E+, 2 S+, 3 S−, 4 E−, 5 GND (cable shield)   |
| J3–J6     | four 3-wire half-bridge cells, alternative to J2                 |
| J7        | external 5 V: 1 = +5 V, 2 = GND, through Schottky D1             |
| J8        | Spy-Bi-Wire pogo pads, 2.54 mm: 1 RST, 2 TEST, 3 GND             |

| jumper | position                | meaning                                              |
|--------|-------------------------|------------------------------------------------------|
| JP1    | 1-2 (default, bridged)  | 3V3 from keg20 (J1-1)                                |
|        | 2-3                     | 3V3 from U4, board runs from 5 V only. **Cut 1-2 first**, never bridge all three pads |
| JP2    | open (default)          | two-wire UART                                        |
|        | closed                  | one-wire UART on J1-3                                |
| JP3    | bridged (default)       | TEST tied to MSP RX, needed for BSL entry over UART  |
|        | cut                     | TEST separate, fit R8 (see [Programming pads](#programming-pads-fallback)) |

```
                J1                                           MSP430FR2111 (TSSOP-16)

  3  ESP_TXD ──┬──[ R5 330R ]──┬──────┬──────┬────────────── 12  P1.6 / UCA0RXD / BSLRX
               D4              │      │      └─ JP3 ─┬────── 3   TEST / SBWTCK
                               │      │              ├─[ R8 10k, not fitted ]─ GND
                               │      │              └─ J8-2
                               │      └─[ R7 4k7 ]─ 3V3
                             JP2 (open)
                               └──►|── D2 ──┐  (anode at JP2, cathode at TXD)
  2  ESP_RXD ─────[ R6 330R ]───────────────┴─────────────── 11  P1.7 / UCA0TXD / BSLTX
  4  ESP_RST ──┬──[ R1 330R ]──┬──────────────────────────── 4   RST / NMI / SBWTDIO
               D4              ├─[ R2 4k7 ]─ 3V3
                               └─ J8-1
  1  ESP_3V3 ── JP1 1-2 ── 3V3 ──┬────────────────────────── 5   DVCC
                          C1 4u7 + C2 100n
  5  GND ────────────────────────┴────────────────────────── 6   DVSS
```

- The BSL uses P1.6 (RX) and P1.7 (TX) (datasheet Table 9-4). These are also
  UCA0 RXD/TXD (USCIARMP = 0), so the app and the BSL use the same pins.
- R2 also keeps GPIO9 high while the ESP boots (strap pin). If the module's 3V3 is off while the
  ESP boots, R1/R2 and the MSP's input diode pull GPIO9 low instead (R2 alone: ~0.3 V) and the
  ESP may enter download mode. Can't happen with JP1 1-2; with JP1 2-3 the module needs its 5 V
  first or together with the ESP.
- R1 is a safeguard (see below): RST low level 0.22 V.
- The low pull-ups (R2, R7 4k7) and series resistors (330R) are for long wires.
- No capacitor on the MSP RST pin: max 1.1 nF with Spy-Bi-Wire. The 100 nF on the
  keg20 side of R1 is fine.

### Power

```
  J7-1 ──►|── D1 SS14 ──┬── +5V ──┬──[ R9 10R ]── U4 LP5907-3.3 ── +3.3VA ──┬── ADS1231 AVDD, VREFP, bridge E+
  J1-6 (VSYS) ──────────┘         D3 TVS                                    └── JP1 2-3 ─┐
  J1-1 (keg20 3V3) ─────────────────────────────────────────────────────────── JP1 1-2 ─┴── +3V3: MSP, ADS1231 DVDD, DS18B20
```

- U4 feeds the analog side only, so the ESP's 3V3 noise stays out of the measurement.
  With JP1 2-3 the MCU shares it; readings may be slightly noisier.
- U4 input: 5.5 V max (6 V absolute max). D1 does not protect against overvoltage.
- U4 output capacitance must stay **0.7–10 µF**: 2.2 µF now (C4 C5 C9 C11), 8.1 µF
  with JP1 2-3 (adds C1 C2 C3 C7). C9: X7R, 10 V or higher.
- R9 with C8 damps the ringing when a powered cable is plugged in.
- J7 also powers the keg20 board through J1-6 (intended). With USB plugged in at the same time it
  may feed back into it, so leave J1-6 open or add a second Schottky there.
- Without 5 V (JP1 1-2) the MCU runs but the ADC has no analog supply: the load cell data is invalid.
- With only one of the two boards powered, current leaks through the UART and RST lines: up to ~8 mA
  per push-pull line (330R), the MSP allows ±2 mA diode current. Power both together.

### ESD

D3 (SMF5V0A, on +5V) and D4 (SP0502BAHT, on ESP_TXD and ESP_RST) go right at J1 with a
short path to GND. D3 clamps above U4's 6 V absolute maximum: it is for ESD and spikes,
not for sustained overvoltage. ESP_RXD and the load cell inputs have no ESD parts.
D1 covers reversed polarity on J7 only. Reversed 5 V on J1 shorts through D3, and a J1
plug put in backwards puts 5 V on the 3V3 pin: use a keyed connector (decided at layout).

### One-wire UART and 4-wire standalone module

JP2 closed: MSP TXD pulls J1-3 low through D2, R7 pulls it high. Both directions
share J1-3 and J1-2 can stay unconnected.

As a standalone module only J1 3–6 are needed: **TXD, RST, GND, +5V**, with JP1 on 2-3
(1-2 cut) and JP2 closed.

- The host pin on J1-3 must be **open-drain, never push-pull**: a push-pull high fights
  MSP TXD through D2 (~8 mA through R5 330R). A USB-UART adapter may only listen
  (its RX on J1-3, its TX not connected).
- Keep TXD and RST at 3.3 V levels.
- R7 holds TEST high at power-up and the app doesn't start by itself (seen on a G2553): the host must
  reset it (TXD low ≥ 100 µs, pulse RST low). Or cut JP3 and fit R8 if UART flashing isn't needed.
- The app only sends, so listening works with the firmware as it is.
- The MSP also receives what it sends. The app ignores RX, but **the BSL hears its own replies**
  (hardware UART, measured on an FR2311): a reply arrives intact, then the BSL answers its echo with
  51h, hears that too, and floods the line for ever. **The host must send a break (TX low ≥ 3 ms, use
  10 ms) after each BSL reply.** With it flash, read back and 100 entry loops passed; without, about
  half of the replies were lost. `bsl.py --onewire` does it.
- With one wire the MSP's own data also pulls TEST low, so a glitch on RST during a frame may reset the MSP
  (one unexplained RST reset seen on the bench, right after rewiring). Keep RST quiet.
- A host that also sends must not talk over the MSP's frames: send only between them.
- TEST toggles with the MSP's own data as well (harmless while RST is high, see below).
- R7 must be 4k7: TEST has an internal pull-down (20–50k) and with an open-drain host only R7
  holds the line high. Calculated idle level: 2.67–2.91 V with 4k7 (a guaranteed high needs
  ~2.5 V), only 1.0–1.4 V with 47k. Rise time ~3 µs with 300 pF, ~9 µs with 1 nF.
- R5 330R keeps the low level at ~0.3 V (open-drain host) to 0.5 V (diode in the host's TX), limit 0.82 V.
  1k would leave no margin.

### TEST on the RX line – why it works

TEST is tied to the ESP TX line (through JP3 and R5), which idles high. It is therefore high most of
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
- Each SBW cycle has a TDO slot in which the MSP may drive SBWTDIO. **R1** limits
  contention with the ESP's push-pull high (10 mA peak with 330R, 3 mA with 1k). The ESP side is held by the 100 nF, so
  the line is back at 1 for the next TMS slot.
- R7 keeps TEST high when nothing is connected to J1-3.
- At power-up with TEST high the MSP doesn't start its app without help (seen on a G2553, LED
  only; not tested on an FR2xx). In normal use it doesn't matter: the ESP always takes over at boot with
  its own reset or BSL sequence (both need TEST low or the BSL entry pulses).
- A **watchdog reset** with TEST high does restart the app (FR2311, 5/5).
- When the ESP resets, its ROM log toggles GPIO16. The MSP's RX and TEST see
  garbage; this is harmless because the app ignores RX.
- SBW debugging (TI debugger) is possible only with the J1 link disconnected.

### Programming pads (fallback)

If the UART bootloader method doesn't work, program the MSP over Spy-Bi-Wire with pogo pins
on **J8** (footprint `POGO_PADS_3` in `schematic/lib/keg01.pretty`).

- Unplug J1: the host and the 100 nF on the keg20 RST line load the signals.
- J8 has no supply pin. Power the board from J7 with JP1 on 2-3 and keep the programmer at 3.3 V.
- JP2 must be open while JP3 is bridged, otherwise MSP TXD can pull TEST low through D2.
- If TEST on the UART line is the problem, cut JP3 and fit R8 (10k to GND). R8 does nothing
  useful while JP3 is bridged (R7 and the host hold the line high). R7 stays on MSP RX.
  UART BSL entry doesn't work with JP3 cut, J8 is then the way to program the chip.

## Sensors on the MSP

| sensor  | signal    | MSP pin    | note                                             |
|---------|-----------|------------|--------------------------------------------------|
| ADS1231 | SCLK      | P1.1 (1)   | push-pull output                                 |
| ADS1231 | DRDY/DOUT | P1.0 (2)   | DVDD = 3V3, no level shifting                    |
| ADS1231 | PDWN      | P1.3 (15)  | push-pull output, R3 100k pull-down              |
| DS18B20 | DQ        | P1.2 (16)  | R4 4k7 pull-up to 3V3, VDD from 3V3 (non-parasitic) |
| spare   |           | P2.0, P2.1, P2.6, P2.7 | future sensors, not routed to a connector |

- Avoid P1.4/P1.5: they are 4-wire JTAG pins (TCK/TMS). P1.6/P1.7 (BSL/UART) are TDI/TDO.
- DS18B20: 12-bit conversion takes 750 ms. The other notes in `fw20/README.md` still apply.

### ADS1231

Datasheet SBAS414D.

- SPEED is tied low (10 SPS), CLKIN is tied low (internal oscillator), gain is fixed at 128.
- **PDWN:** hold low for ≥ 10 ms after power-up, then high. R3 keeps the ADC off while the
  MSP is in reset or in the BSL. The current firmware drives P1.3 low as an unused pin, so
  the ADC **never starts**. This is the one change it needs.
- The readout is compatible with the HX711 code: 24 bits MSB first, two's complement, and the
  25th pulse forces DOUT high. The HX711 50/60 µs SCK limits don't apply: holding SCLK high
  (at 10 SPS for ~100 ms) only enters standby.
- First valid data ~400 ms after wake-up.
- The reference is ratiometric: VREF = AVDD = E+. Full scale is ±0.5 × 3.3 V / 128 = ±12.9 mV.
- The input common mode must stay within 1.5 V to AVDD − 1.5 V = **1.5–1.8 V**. A balanced
  bridge sits at 1.65 V; check it with the real cells.
- E− returns through the ADS1231 switch (SW, max 30 mA). It is open in power-down and
  standby, so E− is not GND. VREFN is sensed at E−.
- C6 100 nF C0G goes close to the CAP pins.
- Input filter for J2 and J3–J6: R10/R11 100R in series with S+/S−, C10 100 nF C0G between
  AINP and AINN (~8 kHz). Place it at U2 pins 7 and 8.
- C11 100 nF between VREFP and VREFN, at U2 pins 10 and 9 (E− is not GND).

### Load cells

**J2** takes a 4-wire full bridge. Pin 5 (GND) is for the cable shield: connect it at this end
only. If the shield is joined to a cell body that is grounded elsewhere, leave it open.
Cells with 6 wires have two sense wires: join SEN+ to E+ and SEN− to E−, never to GND.

**J3–J6** take four 3-wire half-bridge cells (same wiring and pads as `schematic/splitter`, P1–P4):

| connector | splitter | red wire | pin 1 / 2 / 3         |
|-----------|----------|----------|-----------------------|
| J3        | P1       | E+       | black / red / white   |
| J4        | P2       | S−       | white / red / black   |
| J5        | P3       | E−       | black / red / white   |
| J6        | P4       | S+       | white / red / black   |

White wires join J3–J4 and J5–J6, black wires join J4–J5 and J6–J3. Use J2 **or** J3–J6,
never both. If the reading falls with load, swap S+ and S− (or invert it in software).

## Protocol (MSP → ESP)

UART **9600 8E1**, the same format as the BSL, so the ESP never reconfigures its
UART. The MSP sends ASCII lines ending in `\n`, so a plain terminal is enough for testing.

Every frame is `<type><hex payload>*<crc>\n`. The CRC is Dallas/Maxim CRC-8
(reflected polynomial 0x8C, init 0) over every byte before `*`, sent as 2
uppercase hex digits. The same CRC function checks the DS18B20 scratchpad on the MSP.

| frame         | when                          | content                                        |
|---------------|-------------------------------|------------------------------------------------|
| `V<2 hex>*cc` | once after reset              | firmware version                               |
| `H<6 hex>*cc` | each ADC conversion (~10/s)   | load cell raw value, 24-bit two's complement, MSB first |
| `h<2 hex>*cc` | instead of `H`                | ADC error: `01` not ready for 500 ms (missing or powered down), `02` DOUT still low after the 25th pulse (unpowered) |
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
   rising edges on TEST) → RST high while TX is still high → **TX low once** (1–200 ms tried, use
   5 ms) → TX high. The FR2311 starts its BSL only at that low; without it the first packet is
   lost. (A G2553 needs the opposite, TX must stay high.)
   Hand GPIO16 back to the UART (9600 8E1, idle high, no glitch), wait 50 ms.
   Check with a packet that changes nothing: *TX data block* (0x18) without password → `3B 04`
   ("locked"). No answer → repeat the entry.
2. **Firmware check:** the BSL password is the 32-byte interrupt vector table
   (0xFFE0–0xFFFF). The ESP sends the vectors of its embedded image.
   - Password accepted → read the code area back with *TX data block* (0x18) and compare. If it
     matches the embedded image, the firmware is OK. If not, erase: send a wrong password.
   - Password wrong → **the BSL erases everything** and answers 05.
   - After an erase send the password of the erased chip (32 × 0xFF; the FR2311 goes on in the
     same session, otherwise enter again), write the image with *RX data block* (0x10, max. 256
     data bytes; ~800 B, about 1 s at 9600), reset vector last, and read it back.
   - The FR21xx BSL has **no Mass Erase (0x15), CRC Check (0x16) and TX BSL Version (0x19)**
     (SLAU550AB table 4-9). Never write FF80–FF87 (JTAG/BSL signatures).
   - Wait ≥ 1.2 ms after the BSL's last byte before the next packet (3 ms used). One wire (JP2
     closed): a break after each reply, see above.
3. **Start the app:** TX low ≥ 100 µs, RST low → high (TEST low), wait for the
   `V<xx>` line and start reading `H`/`T` lines. Ignore junk in front of the first line:
   the reset's TX low is data for the BSL (it sends a NAK) and TX glitches during reset.
4. It can't brick the MSP: hardware BSL entry doesn't depend on the app, so an
   interrupted update just retries on the next boot.

BSL frame format: `0x80 NL NH CMD data… CKL CKH`, with CRC-CCITT over CMD + data.
The BSL answers with an ACK byte (0x00), then a `0x80 …` response frame.

## MSP firmware (`sensor/fw/`)

```bash
cd sensor/fw
make MCU=msp430fr2111       # MSP430FR2111 (schematic): build/sensor.elf .hex .bin .lst, prints the size
make                        # MSP430FR2100 (Makefile default); run make clean when switching
python3 tools/decode.py --selftest
```

- Written for the HX711; the ADS1231 needs PDWN (P1.3), see above.
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

Use a 3.3 V USB-UART adapter (FTDI/CP2102) instead of the ESP, with two wires (**JP2 open**).
The ADC also needs 5 V on J1-6 or J7:

| adapter | J1                                        |
|---------|-------------------------------------------|
| TXD     | 3 (MSP P1.6 + TEST)                       |
| RXD     | 2 (MSP P1.7)                              |
| RTS     | 4 (RST, R1 330R is on the board); adapter without RTS: DTR and `--rst dtr` |
| 3V3/GND | 1 / 5 (JP1 1-2; with JP1 2-3 leave 1 open) |

- **Monitor:** `python3 sensor/fw/tools/decode.py /dev/ttyUSB0` (or
  `picocom -b 9600 -y e /dev/ttyUSB0`). Expect `V01`, then `H…` frames at ~10/s
  and `T…` at ~1/s. Load the cell and warm the sensor;
  the values should react. Until the firmware drives PDWN, expect `h…` errors or no valid
  `H…` data.
- **Flash:** `python3 sensor/fw/tools/bsl.py flash /dev/ttyUSB0 sensor/fw/build/sensor.hex`
  (pyserial).
  - It drives RST from RTS and makes the TEST pulses on TX with a UART break
    (break = TX low).
  - It runs the same algorithm as the ESP boot flow: the image's vector table is
    the password, an equal read-back means "up to date" and the write is skipped,
    otherwise it erases (wrong password), writes and reads back. With `--old OLD.hex` the
    vectors of the old image are the password. Without it other vectors just mean an erase.
  - It then resets the MSP into the app.
  - Other commands: `bsl.py erase PORT`, `bsl.py reset PORT`.
  - Options: `--onewire` (JP2 closed, adapter TX behind a diode as in
    [test_fr2311](test_fr2311/README.md)), `--force` (flash even if up to date), `--no-reset`,
    `-v` (packet dump), `--test dtr` (TEST wired to DTR instead of TX),
    `--invert-reset`/`--invert-test`.
  - `bsl.py --selftest` runs the flash flow against a simulated BSL. On hardware it is tested with
    an FR2311, 3 wires and one wire (up to date, forced, other code, wrong password, erase, blank chip).
  - The adapter must support break (FTDI, CP210x and CH340 do).

## Open points (verify on the bench / in SLAU610)

Answered by the FR2311 test (repeat on the first FR2111, its BSL ROM is another version):

- [x] TEST high "enables the JTAG pins" (datasheet 9.7): P1.6 (TDI) and P1.7 (TDO) still work as UART,
      in the app and in the BSL
- [x] BSL entry: TEST low once after RST rises. Pulses of 5 ms, RST high 5 ms after TEST high
- [x] Wrong BSL password erases (SLAU550AB, and seen on the FR2311)
- [x] BSL commands: no CRC check, RX data block max. 256 bytes (SLAU550AB)
- [x] App keeps running with TEST toggling (1000 pings with 3 wires and with one wire, no resets), the BSL works with TEST high
- [x] One-wire mode: app and BSL work, with a break after each BSL reply
- [x] TCK/TMS (P1.4/P1.5) floating during the BSL can enter JTAG mode (SLAU550 3.3.2.1 recommends 47k
      pull-up and 1 nF each): 260 of 260 entries and 20 flashes ok with both open (3 wires,
      short wiring, no nearby noise source). No pull-ups on the board for now

Still open:

- [ ] **FR2111**: repeat entry, flash and one wire on the real chip
- [ ] Contention on RST in the TDO slot with R1 330R: no problem seen on the FR2311, not measured
- [ ] REFO / DCO accuracy for 9600 baud over the keg temperature range
- [ ] Firmware: drive PDWN (P1.3), check the ADS1231 readout
- [ ] Bridge common mode within 1.5–1.8 V, ADC noise with JP1 1-2 and 2-3
- [ ] One-wire line levels (R7 4k7, R5 330R) not measured
- [ ] U4 stable with JP1 2-3 (8.1 µF in total), 5 V overshoot at plug-in below 6 V at U4
- [ ] Does an FR2xx also need a reset after power-up with TEST high (as the G2553)? Not tested on the FR2311 in
      a controlled way; once it came up by itself after a power cycle (cause brownout)
- [ ] Spy-Bi-Wire over J8 with R2 4k7 (TI's reference circuit: 47k)
- [ ] Check the D3/D4 ratings in their datasheets before ordering
- [ ] Check timing on the scope: 1-wire slots and the read sample point
- [ ] Unplug each sensor and check the error frames (`h01`/`h02`, `t01`/`t02`)
