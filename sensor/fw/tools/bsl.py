#!/usr/bin/env python3
"""Flash the keg20 sensor bridge (MSP430FR2xxx) through its ROM BSL over a USB-UART.

    bsl.py flash /dev/ttyUSB0 build/sensor.hex [--force] [--no-reset]
    bsl.py erase /dev/ttyUSB0
    bsl.py reset /dev/ttyUSB0
    bsl.py --selftest

Wiring (3.3 V USB-UART, see ../../README.md):
    TXD -> P1.6 (BSLRX) and TEST      RXD <- P1.7 (BSLTX)
    RTS -> RST (through 1k)           3V3/GND -> DVCC/DVSS

TEST is tied to TX, so the entry pulses on TEST are made with a UART break
(break = TX low). With --test dtr, TEST is driven from DTR instead. The RTS/DTR
pins are active low: rts=True drives the pin low. --invert-reset/--invert-test
flip that if the adapter or wiring inverts.

Flash flow:
  1. hardware BSL entry
  2. password = vector table of the image (0xFFE0-0xFFFF). If it is accepted and
     the CRC of every image segment matches, the chip is up to date (unless --force).
  3. otherwise mass erase (a wrong password erases too), password 32 x 0xFF,
     write the image, verify with the BSL CRC check
  4. reset into the application (TEST low, RST pulse)

Input: Intel HEX (build/sensor.hex) or TI-TXT.
"""
import argparse
import sys
import time

# ---- BSL protocol (SLAU550: MSP430 FRAM device bootloader, UART) ----

BAUD = 9600
CMD_RX_DATA = 0x10
CMD_RX_PASSWORD = 0x11
CMD_MASS_ERASE = 0x15
CMD_CRC_CHECK = 0x16
CMD_LOAD_PC = 0x17
RSP_DATA = 0x3A
RSP_MESSAGE = 0x3B

BLOCK = 64          # data bytes per RX data block (well below the BSL buffer)

ACK_ERRORS = {
    0x51: 'header incorrect', 0x52: 'checksum incorrect', 0x53: 'packet size zero',
    0x54: 'packet size exceeds buffer', 0x55: 'unknown error', 0x56: 'unknown baud rate',
}
MESSAGES = {
    0x00: 'ok', 0x01: 'memory write check failed', 0x02: 'FRAM fail bit set',
    0x03: 'voltage change during program', 0x04: 'BSL locked', 0x05: 'password error',
    0x06: 'byte write forbidden', 0x07: 'unknown command', 0x08: 'packet length exceeds buffer',
}

VECTORS = 0xFFE0    # 32-byte interrupt vector table = BSL password


class BslError(Exception):
    pass


def crc_ccitt(data, crc=0xFFFF):
    """CRC-16-CCITT (poly 0x1021, init 0xFFFF): packet checksum and CRC check command."""
    for b in data:
        x = ((crc >> 8) ^ b) & 0xFF
        x ^= x >> 4
        crc = ((crc << 8) ^ (x << 12) ^ (x << 5) ^ x) & 0xFFFF
    return crc


def addr3(a):
    return bytes([a & 0xFF, (a >> 8) & 0xFF, (a >> 16) & 0xFF])


class Bsl:
    def __init__(self, ser, test='tx', invert_test=False, invert_reset=False, verbose=False):
        self.ser = ser
        self.test_pin = test
        self.invert_test = invert_test
        self.invert_reset = invert_reset
        self.verbose = verbose

    # -- control lines --

    def rst(self, high):
        self.ser.rts = (not high) ^ self.invert_reset

    def test(self, high):
        if self.test_pin == 'tx':
            self.ser.break_condition = (not high) ^ self.invert_test
        else:
            self.ser.dtr = (not high) ^ self.invert_test

    def release_test(self):
        """Back to normal UART use (TX idle high again; TEST follows, harmless)."""
        if self.test_pin == 'tx':
            self.ser.break_condition = False
        else:
            self.test(False)

    def enter(self):
        """Hardware BSL entry: two rising edges on TEST while RST is low,
        RST rises while TEST is high, then TEST low."""
        self.test(False)
        time.sleep(0.01)    # TEST low >= 100 us: Spy-Bi-Wire off, RST is reset again
        self.rst(False)
        time.sleep(0.01)
        self.test(True)
        time.sleep(0.005)
        self.test(False)
        time.sleep(0.005)
        self.test(True)
        time.sleep(0.005)
        self.rst(True)
        time.sleep(0.005)
        self.test(False)
        time.sleep(0.005)
        self.release_test()
        time.sleep(0.05)    # BSL start-up
        self.ser.reset_input_buffer()   # drop application frames sent before the reset

    def reset(self):
        """Reset into the application: TEST low (SBW off), RST pulse."""
        self.test(False)
        time.sleep(0.01)
        self.rst(False)
        time.sleep(0.01)
        self.rst(True)
        time.sleep(0.01)
        self.release_test()

    # -- packets --

    def _read(self, n):
        data = self.ser.read(n)
        if len(data) != n:
            raise BslError('timeout (got %d of %d bytes)' % (len(data), n))
        return data

    def command(self, core, response=True):
        """Send a core command, check the ACK, return (rsp_cmd, rsp_data) or None."""
        pkt = bytes([0x80, len(core) & 0xFF, len(core) >> 8]) + core
        crc = crc_ccitt(core)
        pkt += bytes([crc & 0xFF, crc >> 8])
        if self.verbose:
            print('  >', pkt.hex(' '))
        self.ser.write(pkt)
        ack = self._read(1)[0]
        if ack != 0x00:
            raise BslError('NAK 0x%02X %s' % (ack, ACK_ERRORS.get(ack, '?')))
        if not response:
            return None
        hdr = self._read(3)
        if hdr[0] != 0x80:
            raise BslError('bad response header %s' % hdr.hex(' '))
        n = hdr[1] | (hdr[2] << 8)
        if n == 0:
            raise BslError('empty response')
        body = self._read(n)
        ck = self._read(2)
        if self.verbose:
            print('  <', (bytes([ack]) + hdr + body + ck).hex(' '))
        if crc_ccitt(body) != ck[0] | (ck[1] << 8):
            raise BslError('response checksum mismatch')
        return body[0], body[1:]

    def message(self, core):
        cmd, data = self.command(core)
        if cmd != RSP_MESSAGE or len(data) != 1:
            raise BslError('unexpected response 0x%02X %s' % (cmd, data.hex(' ')))
        return data[0]

    def password(self, pw):
        """True when accepted. A wrong password mass-erases the chip."""
        assert len(pw) == 32
        msg = self.message(bytes([CMD_RX_PASSWORD]) + pw)
        if msg not in (0x00, 0x05):
            raise BslError('password: %s' % MESSAGES.get(msg, hex(msg)))
        return msg == 0x00

    def mass_erase(self):
        msg = self.message(bytes([CMD_MASS_ERASE]))
        if msg != 0x00:
            raise BslError('mass erase: %s' % MESSAGES.get(msg, hex(msg)))

    def write(self, addr, data):
        msg = self.message(bytes([CMD_RX_DATA]) + addr3(addr) + data)
        if msg != 0x00:
            raise BslError('write 0x%04X: %s' % (addr, MESSAGES.get(msg, hex(msg))))

    def crc(self, addr, length):
        cmd, data = self.command(bytes([CMD_CRC_CHECK]) + addr3(addr)
                                 + bytes([length & 0xFF, length >> 8]))
        if cmd == RSP_MESSAGE and data:
            raise BslError('CRC check: %s' % MESSAGES.get(data[0], hex(data[0])))
        if cmd != RSP_DATA or len(data) != 2:
            raise BslError('unexpected CRC response 0x%02X %s' % (cmd, data.hex(' ')))
        return data[0] | (data[1] << 8)


# ---- image files ----

def load_ihex(text):
    mem, base = {}, 0
    for n, line in enumerate(text.splitlines(), 1):
        line = line.strip()
        if not line:
            continue
        if not line.startswith(':'):
            raise ValueError('line %d: not Intel HEX' % n)
        rec = bytes.fromhex(line[1:])
        if sum(rec) & 0xFF:
            raise ValueError('line %d: checksum' % n)
        cnt, addr, typ, data = rec[0], (rec[1] << 8) | rec[2], rec[3], rec[4:-1]
        if len(data) != cnt:
            raise ValueError('line %d: length' % n)
        if typ == 0x00:
            for i, b in enumerate(data):
                mem[base + addr + i] = b
        elif typ == 0x01:
            break
        elif typ == 0x02:
            base = ((data[0] << 8) | data[1]) << 4
        elif typ == 0x04:
            base = ((data[0] << 8) | data[1]) << 16
        # 0x03 / 0x05: start address, ignored
    return mem


def load_titxt(text):
    mem, addr = {}, None
    for line in text.split('\n'):
        line = line.strip()
        if not line:
            continue
        if line.startswith('@'):
            addr = int(line[1:], 16)
        elif line.lower() == 'q':
            break
        else:
            if addr is None:
                raise ValueError('TI-TXT: data before @address')
            for tok in line.split():
                mem[addr] = int(tok, 16)
                addr += 1
    return mem


def load_image(path):
    with open(path) as f:
        text = f.read()
    mem = load_ihex(text) if text.lstrip().startswith(':') else load_titxt(text)
    if not mem:
        raise ValueError('%s: no data' % path)
    return mem


def segments(mem):
    """Contiguous (addr, bytes) runs, padded with 0xFF to even address and length."""
    addrs = sorted(mem)
    segs, start, prev = [], addrs[0], addrs[0]
    for a in addrs[1:] + [None]:
        if a is not None and a <= prev + 2:     # bridge 1-byte holes (pad below)
            prev = a
            continue
        s, e = start & ~1, (prev + 2) & ~1      # [s, e) even aligned
        segs.append((s, bytes(mem.get(x, 0xFF) for x in range(s, e))))
        if a is not None:
            start = prev = a
    return segs


def password_of(mem):
    return bytes(mem.get(VECTORS + i, 0xFF) for i in range(32))


# ---- actions ----

def flash(bsl, mem, force=False):
    segs = segments(mem)
    total = sum(len(d) for _, d in segs)
    for a, d in segs:
        print('segment 0x%04X-0x%04X (%d bytes)' % (a, a + len(d) - 1, len(d)))

    bsl.enter()
    if bsl.password(password_of(mem)):
        if not force and all(bsl.crc(a, len(d)) == crc_ccitt(d) for a, d in segs):
            print('firmware up to date')
            return False
        print('firmware differs: mass erase')
        bsl.mass_erase()
    else:
        print('password rejected: chip mass-erased by the BSL')
    if not bsl.password(b'\xFF' * 32):
        raise BslError('erased-chip password rejected')

    done = 0
    for a, d in segs:
        for off in range(0, len(d), BLOCK):
            bsl.write(a + off, d[off:off + BLOCK])
            done += len(d[off:off + BLOCK])
            print('\rwriting %d/%d' % (done, total), end='', flush=True)
    print()
    for a, d in segs:
        got, want = bsl.crc(a, len(d)), crc_ccitt(d)
        if got != want:
            raise BslError('verify 0x%04X: CRC 0x%04X, expected 0x%04X' % (a, got, want))
    print('verify ok')
    return True


def erase(bsl):
    bsl.enter()
    # Mass erase needs an unlocked BSL; a wrong password erases anyway.
    if not bsl.password(b'\xFF' * 32):
        print('password rejected: chip mass-erased by the BSL')
        if not bsl.password(b'\xFF' * 32):
            raise BslError('erased-chip password rejected')
    else:
        bsl.mass_erase()
    print('erased')


def open_port(port):
    import serial
    ser = serial.Serial()
    ser.port = port
    ser.baudrate = BAUD
    ser.bytesize = serial.EIGHTBITS
    ser.parity = serial.PARITY_EVEN
    ser.stopbits = serial.STOPBITS_ONE
    ser.timeout = 1.0
    ser.rts = False     # set before open: no reset glitch when the port opens
    ser.dtr = False
    ser.open()
    return ser


# ---- self test against a simulated BSL ----

class FakeBsl:
    """Minimal FR2xxx BSL model: memory, password lock, mass erase on bad password."""

    def __init__(self, mem=None):
        self.mem = dict(mem or {})
        self.unlocked = False
        self.out = bytearray()

    rts = False
    dtr = False
    break_condition = False

    def reset_input_buffer(self):
        self.out.clear()

    def read(self, n):
        data, self.out[:n] = bytes(self.out[:n]), b''
        return data

    def _reply(self, cmd, data):
        core = bytes([cmd]) + data
        crc = crc_ccitt(core)
        self.out += bytes([0x80, len(core), 0]) + core + bytes([crc & 0xFF, crc >> 8])

    def write(self, pkt):
        assert pkt[0] == 0x80
        n = pkt[1] | (pkt[2] << 8)
        core, ck = pkt[3:3 + n], pkt[3 + n:5 + n]
        if crc_ccitt(core) != ck[0] | (ck[1] << 8):
            self.out.append(0x52)
            return
        self.out.append(0x00)
        cmd, data = core[0], core[1:]
        if cmd == CMD_RX_PASSWORD:
            ok = data == bytes(self.mem.get(VECTORS + i, 0xFF) for i in range(32))
            if not ok:
                self.mem.clear()
            self.unlocked = ok
            self._reply(RSP_MESSAGE, bytes([0x00 if ok else 0x05]))
            return
        if not self.unlocked:
            self._reply(RSP_MESSAGE, b'\x04')
            return
        a = data[0] | (data[1] << 8) | (data[2] << 16) if len(data) >= 3 else 0
        if cmd == CMD_MASS_ERASE:
            self.mem.clear()
            self._reply(RSP_MESSAGE, b'\x00')
        elif cmd == CMD_RX_DATA:
            for i, b in enumerate(data[3:]):
                self.mem[a + i] = b
            self._reply(RSP_MESSAGE, b'\x00')
        elif cmd == CMD_CRC_CHECK:
            ln = data[3] | (data[4] << 8)
            c = crc_ccitt(bytes(self.mem.get(a + i, 0xFF) for i in range(ln)))
            self._reply(RSP_DATA, bytes([c & 0xFF, c >> 8]))
        else:
            self._reply(RSP_MESSAGE, b'\x07')


def selftest():
    assert crc_ccitt(b'123456789') == 0x29B1     # CRC-16/CCITT-FALSE check value
    image = {0xFC00 + i: (i * 7) & 0xFF for i in range(301)}    # odd length -> padding
    image.update({0xFFFE: 0x00, 0xFFFF: 0xFC})
    segs = segments(image)
    assert [(a, len(d)) for a, d in segs] == [(0xFC00, 302), (0xFFFE, 2)], segs
    assert load_ihex(':02FFFE0000FC05\n:00000001FF\n') == {0xFFFE: 0x00, 0xFFFF: 0xFC}
    assert load_titxt('@FFFE\n00 FC\nq\n') == {0xFFFE: 0x00, 0xFFFF: 0xFC}

    old_sleep = time.sleep
    time.sleep = lambda s: None
    try:
        chip = FakeBsl()                                    # blank chip
        assert flash(Bsl(chip), image) is True
        assert all(chip.mem.get(a) == b for a, b in image.items())
        assert flash(Bsl(chip), image) is False             # same image: skipped
        image2 = dict(image)
        image2[0xFC10] ^= 0xFF                              # same vectors, other code
        assert flash(Bsl(chip), image2) is True and chip.mem[0xFC10] == image2[0xFC10]
        chip.mem[0xFFE0] = 0x12                             # other vectors: wrong password
        assert flash(Bsl(chip), image) is True and chip.mem[0xFC10] == image[0xFC10]
    finally:
        time.sleep = old_sleep
    print('selftest ok')


def main():
    if '--selftest' in sys.argv[1:]:
        selftest()
        return
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('action', choices=['flash', 'erase', 'reset'])
    ap.add_argument('port', help='serial port, e.g. /dev/ttyUSB0')
    ap.add_argument('image', nargs='?', help='Intel HEX or TI-TXT (flash)')
    ap.add_argument('--force', action='store_true', help='flash even if up to date')
    ap.add_argument('--no-reset', action='store_true', help='stay in the BSL after flashing')
    ap.add_argument('--test', choices=['tx', 'dtr'], default='tx',
                    help='TEST driven by TX via break (default) or by DTR')
    ap.add_argument('--invert-test', action='store_true')
    ap.add_argument('--invert-reset', action='store_true')
    ap.add_argument('-v', '--verbose', action='store_true', help='dump BSL packets')
    args = ap.parse_args()
    if args.action == 'flash' and not args.image:
        ap.error('flash needs an image file')

    mem = load_image(args.image) if args.action == 'flash' else None
    ser = open_port(args.port)
    bsl = Bsl(ser, args.test, args.invert_test, args.invert_reset, args.verbose)
    try:
        if args.action == 'flash':
            flash(bsl, mem, args.force)
        elif args.action == 'erase':
            erase(bsl)
        if not (args.action == 'flash' and args.no_reset):
            bsl.reset()
            print('reset')
    except BslError as e:
        sys.exit('error: %s' % e)
    finally:
        ser.close()


if __name__ == '__main__':
    main()
