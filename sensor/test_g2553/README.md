# Bench test: BSL entry and UART with TEST tied to RX (MSP430G2553)

Proves the principle of the keg20 sensor link before the PCB exists: TEST is tied to the
UART RX line, BSL entry = break pulses on that line + RST. The chip is a bare DIP-20 G2553
on a breadboard, the host is a 3.3 V USB-UART adapter (CH340) with one control line.
That line is **RTS**, although it was taken for DTR; `--rst dtr` selects the other one.

The G2553 BSL protocol differs from the FR2111 (SLAU319 vs SLAU550), so `bsl_g2.py` has its
own packets. `enter()` and `reset()` come from `../fw/tools/bsl.py`: that is the code under
test. A pass doesn't prove the FR2111 packets, nor fully its P1.7 pin (TDO).

## Wiring

```
adapter TX  --[1k]--+-- 7   P1.5 (BSL RX, app RX, TMS)
                    +-- 17  TEST
                    +-- 9   P2.1 (probe input)
                    +--[47k]-- VCC
adapter RX  --[1k]----- 3   P1.1 (BSL TX, app TX)
adapter RTS --[1k]--+-- 16  RST      direct, not through a capacitor
                    +--[47k]-- VCC   no capacitor, max 1 nF
adapter 3V3 ----------- 1   VCC      100 nF to GND at the chip
adapter GND ----------- 20  GND
                        6   P1.4 --[47k]-- VCC    (TCK must not float)
                        15  P1.7 -- 8  P2.0       (TDO probe wire)
                        2   P1.0 --[470R]--|>|-- GND   LED
```

## Firmware

`make` builds `build/v1.hex` (slow blink) and `build/v2.hex` (fast blink). Software UART
9600 8E1 on the BSL pins, frames as in `../fw` (list at the top of `main.c`). BSLSKEY = 0.
Without factory calibration the app measures the baud rate from `0x80` bytes; the host
tools send them.

## Danger

The G2553 BSL unlocks info segment A. A mass erase, or a **wrong password** while word FFDE
is not 0, erases everything **including the DCO calibration**. `bsl_g2.py` has no mass erase
and needs `--first` before it sends a password to a chip with unknown content. `flash` and
`info` save the info memory to `backup/` when they can.

## Steps

| # | command | pass |
|---|---------|------|
| 1 | `bsl_g2.py PORT probe` (changes nothing) | BSL entered and synchronised |
| 2 | `bsl_g2.py PORT info --first` | chip ID 2553, BSL 0203 |
| 3 | `bsl_g2.py PORT flash build/v1.hex --first` | verify ok, app version 1, LED blinks |
| 4 | `host.py PORT stress 1000`, then `cmd J` | no errors, no resets |
| 5 | `bsl_g2.py PORT flash build/v2.hex --old build/v1.hex` | version 2, fast blink |
| 6 | `bsl_g2.py PORT loop 50` | 0 failures |
| 7 | `host.py PORT rst-test`, `host.py PORT break` | reset only with TEST low; break harmless |
| 8 | power cycle with TEST high, `host.py PORT monitor` | does the app start by itself? |

If step 1 fails: try `--rst dtr`, then `--rst manual` (RST by hand), then `--test dtr --rst manual`.

## Results (2026-11-24, G2553, CH340)

| # | result |
|---|--------|
| 1 | pass, after two fixes: RST is on RTS (not DTR), and `enter()` must not end with TEST low |
| 2 | pass: chip ID 2553, BSL 0203. The old program's password was unknown, so the chip was erased, including the DCO calibration (no backup) |
| 3 | pass: verify ok, version 1, autobaud |
| 4 | pass: 1000 pings, 0 lost/wrong, 0 resets. Probe: P1.5 and P1.7 0 wrong of 65535, TEST high and low |
| 5 | pass: v2 runs, 200 pings ok; then back to v1 |
| 6 | pass: entry + sync 100/100, reset 50/50, loop 100/100. 9 boot frames had junk in front (BSL NAK / TX glitch) |
| 7 | pass: RST pulse ignored with TEST high, resets with TEST low; break 200 ms and 2 s don't reset |
| 8 | not done (power cycle by hand) |

Not covered: wrong-password handling with BSLSKEY = 0 (v1 and v2 have the same vectors, so
the password always matched), one-wire mode, the FR2111 BSL packets and its P1.7 = TDO.
The P1.7 result counts only if the P1.7-P2.0 wire was fitted.

Cause of the first failure: the final 5 ms TEST low after RST rose was received by the BSL as
a byte. Sync got 90h in every try without it, at 1200-19200 baud, and none with it (1, 5, 100 ms).
