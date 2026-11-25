#!/usr/bin/env python3
"""Flash the keg20 sensor bridge (MSP430FR2xxx) through its ROM BSL over a USB-UART.

    bsl.py flash /dev/ttyUSB0 build/sensor.hex [--old OLD.hex] [--force] [--no-reset]
    bsl.py erase /dev/ttyUSB0
    bsl.py reset /dev/ttyUSB0
    bsl.py --selftest

Wiring (3.3 V USB-UART, see ../../README.md):
    TXD -> P1.6 (BSLRX) and TEST      RXD <- P1.7 (BSLTX)
    RTS -> RST (through 1k)           3V3/GND -> DVCC/DVSS
    (adapter without RTS: DTR -> RST, --rst dtr)

TEST is tied to TX, so the entry pulses on TEST are made with a UART break
(break = TX low). With --test dtr, TEST is driven from DTR instead. The RTS/DTR
pins are active low: rts=True drives the pin low. --invert-reset/--invert-test
flip that if the adapter or wiring inverts.

Flash flow:
  1. hardware BSL entry
  2. password = vector table of the image, or of --old (0xFFE0-0xFFFF). If it is accepted
     and the memory reads back equal, the chip is up to date (unless --force).
  3. otherwise erase with a wrong password (a rejected one in step 2 has erased already),
     password 32 x 0xFF, write the image, read it back
  4. reset into the application (TEST low, RST pulse)

The FR20xx/FR21xx BSL has no Mass Erase (0x15), CRC Check (0x16) and TX BSL Version (0x19)
(SLAU550AB table 4-9), so they aren't used.

--onewire: TX and RX on one line (TX open drain or behind a diode). The BSL hears its own
replies there; a break after each one keeps it from answering itself for ever.

Tested on an FR2311 (../../test_fr2311), not on an FR2111 yet.

Input: Intel HEX (build/sensor.hex) or TI-TXT.
"""
import argparse
import sys
import time

# ---- BSL protocol (SLAU550: MSP430 FRAM device bootloader, UART) ----

BAUD = 9600
CMD_RX_DATA = 0x10
CMD_RX_PASSWORD = 0x11
CMD_LOAD_PC = 0x17
CMD_TX_DATA = 0x18
RSP_DATA = 0x3A
RSP_MESSAGE = 0x3B

BLOCK = 64          # data bytes per RX/TX data block (max. 256)
GAP = 0.003         # s before each packet: >= 1.2 ms after the BSL's last byte

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
ERASED = b'\xFF' * 32
SIGNATURES = range(0xFF80, 0xFF88)      # JTAG/BSL signatures: can lock the chip for good


class BslError(Exception):
    pass


def crc_ccitt(data, crc=0xFFFF):
    """CRC-16-CCITT (poly 0x1021, init 0xFFFF): packet checksum."""
    for b in data:
        x = ((crc >> 8) ^ b) & 0xFF
        x ^= x >> 4
        crc = ((crc << 8) ^ (x << 12) ^ (x << 5) ^ x) & 0xFFFF
    return crc


def addr3(a):
    return bytes([a & 0xFF, (a >> 8) & 0xFF, (a >> 16) & 0xFF])


class OneWire:
    """Serial port whose own bytes come back: reads and checks them after each write."""

    def __init__(self, ser):
        object.__setattr__(self, 'ser', ser)

    def __getattr__(self, k):
        return getattr(self.ser, k)

    def __setattr__(self, k, v):
        setattr(self.ser, k, v)

    def write(self, data):
        self.ser.write(data)
        echo = self.ser.read(len(data))
        if echo != data:
            raise BslError('one wire: sent %s, line shows %s' % (data.hex(' '), echo.hex(' ')))


class Bsl:
    # s: TEST low after RST rose. The FR2311 starts its BSL only then (any length from 1 ms, up to
    # 500 ms later); without it the first packet is lost. None: TEST stays high, as the
    # G2553 needs it (its BSL takes the low for data)
    final_low = 0.005
    # s: TX low after each reply, for one wire; 0: none. >= 3 ms needed on the FR2311
    quench = 0

    def __init__(self, ser, test='tx', invert_test=False, invert_reset=False, verbose=False,
                 rst='rts'):
        self.ser = ser
        self.test_pin = test
        self.invert_test = invert_test
        self.invert_reset = invert_reset
        self.verbose = verbose
        self.rst_pin = rst

    # -- control lines --

    def rst(self, high):
        level = (not high) ^ self.invert_reset
        if self.rst_pin == 'dtr':
            self.ser.dtr = level
        else:
            self.ser.rts = level

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
        """Hardware BSL entry: two rising edges on TEST while RST is low, RST rises while
        TEST is high, then TEST low once (see final_low)."""
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
        if self.final_low is not None or self.test_pin != 'tx':
            self.test(False)
            time.sleep(self.final_low or 0.005)
        self.release_test()
        time.sleep(0.05)    # BSL start-up
        self.ser.reset_input_buffer()   # drop application frames sent before the reset

    def start(self, tries=3):
        """Entry, checked with a packet that changes nothing. Returns the number of tries."""
        for i in range(1, tries + 1):
            self.enter()
            try:
                s = self.state()
                if s != 'locked':
                    raise BslError('BSL is %s right after entry: no reset happened' % s)
                return i
            except BslError as e:
                err = e
        raise BslError('no BSL after %d entries (%s)' % (tries, err))

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
        time.sleep(GAP)
        try:
            return self._command(core, response)
        finally:
            if self.quench:
                self.ser.break_condition = True
                time.sleep(self.quench)
                self.ser.break_condition = False
                time.sleep(0.005)
                self.ser.reset_input_buffer()

    def _command(self, core, response):
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

    def write(self, addr, data):
        msg = self.message(bytes([CMD_RX_DATA]) + addr3(addr) + data)
        if msg != 0x00:
            raise BslError('write 0x%04X: %s' % (addr, MESSAGES.get(msg, hex(msg))))

    def state(self):
        """'locked' or 'unlocked' when the BSL answers, else BslError. Changes nothing."""
        cmd, data = self.command(bytes([CMD_TX_DATA]) + addr3(VECTORS) + b'\x02\x00')
        if cmd == RSP_DATA:
            return 'unlocked'
        if cmd == RSP_MESSAGE and data == b'\x04':
            return 'locked'
        raise BslError('unexpected answer 0x%02X %s' % (cmd, data.hex(' ')))

    def read_mem(self, addr, n):
        out = b''
        while n:
            k = min(n, BLOCK)
            cmd, data = self.command(bytes([CMD_TX_DATA]) + addr3(addr) + bytes([k, 0]))
            if cmd != RSP_DATA or len(data) != k:
                raise BslError('read 0x%04X: 0x%02X %s' % (addr, cmd, data.hex(' ')))
            out += data
            addr, n = addr + k, n - k
        return out

    def unlock_erased(self):
        """After a wrong password: the FF password, with a new entry if the BSL is gone."""
        try:
            if self.password(ERASED):
                return
        except BslError:
            pass
        time.sleep(0.5)
        self.start()
        if not self.password(ERASED):
            raise BslError('erased-chip password rejected')


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

def check_image(mem):
    if any(mem.get(a, 0xFF) != 0xFF for a in SIGNATURES):
        raise BslError('image writes the JTAG/BSL signatures (FF80-FF87), refusing')
    if (mem.get(0xFFFE, 0xFF), mem.get(0xFFFF, 0xFF)) == (0xFF, 0xFF):
        raise BslError('image has no reset vector')


def difference(bsl, mem):
    """First difference between image and chip, or None."""
    for a, d in segments(mem):
        got = bsl.read_mem(a, len(d))
        for i in range(len(d)):
            if got[i] != d[i]:
                return '0x%04X is %02X, expected %02X' % (a + i, got[i], d[i])
    return None


def flash(bsl, mem, force=False, old=None):
    """False: was up to date."""
    check_image(mem)
    segs = segments(mem)                # ascending: the reset vector goes last
    total = sum(len(d) for _, d in segs)
    for a, d in segs:
        print('segment 0x%04X-0x%04X (%d bytes)' % (a, a + len(d) - 1, len(d)))

    bsl.start()
    pw = password_of(old or mem)
    if bsl.password(pw):
        if not force and difference(bsl, mem) is None:
            print('firmware up to date')
            return False
        print('password accepted, %s: erasing with a wrong password'
              % ('forced' if force else 'firmware differs'))
        if bsl.password(bytes(b ^ 0xFF for b in pw)):
            raise BslError('a wrong password was accepted')
    else:
        print('password rejected: chip erased by the BSL')
    bsl.unlock_erased()

    done = 0
    for a, d in segs:
        for off in range(0, len(d), BLOCK):
            bsl.write(a + off, d[off:off + BLOCK])
            done += len(d[off:off + BLOCK])
            print('\rwriting %d/%d' % (done, total), end='', flush=True)
    print()
    diff = difference(bsl, mem)
    if diff:
        raise BslError('verify: ' + diff)
    print('verify ok')
    return True


def erase(bsl):
    bsl.start()
    # a password that no program has: all vectors 0
    if bsl.password(bytes(32)):
        raise BslError('the all-zero password was accepted: not erased')
    bsl.unlock_erased()
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
    """FR21xx model: entry sequence, password lock, erase on a bad password, commands it has.
    onewire: own bytes come back, and after a reply it is busy with itself until a break.
    dies: an erase ends the BSL session. needs_low: starts only with a TEST low after RST rose."""

    dtr = False

    def __init__(self, mem=None, onewire=False, dies=False, needs_low=True):
        self.mem = dict(mem or {})
        self.onewire, self.dies, self.needs_low = onewire, dies, needs_low
        self.out = bytearray()
        self.t = self.r = True          # TEST, RST
        self.edges = 0
        self.bsl = self.armed = self.unlocked = self.flood = False

    @property
    def break_condition(self):
        return not self.t

    @break_condition.setter
    def break_condition(self, v):
        self.flood = False
        if not self.t and not v and not self.r:
            self.edges += 1
        if v and self.r and self.armed:
            self.bsl, self.armed = True, False
        self.t = not v

    @property
    def rts(self):
        return not self.r

    @rts.setter
    def rts(self, v):
        if v and self.r:
            self.edges = 0
        if not v and not self.r:        # RST rises
            entry = self.t and self.edges >= 2
            self.unlocked = False
            self.armed, self.bsl = entry and self.needs_low, entry and not self.needs_low
        self.r = not v

    @property
    def in_waiting(self):
        return len(self.out)

    def reset_input_buffer(self):
        self.out.clear()

    def read(self, n):
        data, self.out[:n] = bytes(self.out[:n]), b''
        return data

    def _reply(self, cmd, data):
        core = bytes([cmd]) + data
        self.out += bytes([0x80, len(core), 0]) + core + crc_ccitt(core).to_bytes(2, 'little')

    def write(self, pkt):
        if self.onewire:
            self.out += pkt
        if not self.bsl:                # running the app, or lost: its low bits start the BSL
            self.bsl, self.armed = self.armed, False
            return
        if self.flood:
            self.out += b'\x51' * 20
            return
        self.flood = self.onewire
        assert pkt[0] == 0x80
        n = pkt[1] | (pkt[2] << 8)
        core, ck = pkt[3:3 + n], pkt[3 + n:5 + n]
        if crc_ccitt(core) != ck[0] | (ck[1] << 8):
            self.out.append(0x52)
            return
        self.out.append(0x00)
        cmd, data = core[0], core[1:]
        if cmd == CMD_RX_PASSWORD:
            self.unlocked = data == password_of(self.mem)
            if not self.unlocked:
                self.mem.clear()
                self.bsl = not self.dies
            self._reply(RSP_MESSAGE, b'\x00' if self.unlocked else b'\x05')
        elif cmd not in (CMD_RX_DATA, CMD_TX_DATA, CMD_LOAD_PC):
            self._reply(RSP_MESSAGE, b'\x07')
        elif not self.unlocked:
            self._reply(RSP_MESSAGE, b'\x04')
        elif cmd == CMD_RX_DATA:
            a = data[0] | (data[1] << 8) | (data[2] << 16)
            self.mem.update({a + i: b for i, b in enumerate(data[3:])})
            self._reply(RSP_MESSAGE, b'\x00')
        else:
            a, n = data[0] | (data[1] << 8), data[3] | (data[4] << 8)
            self._reply(RSP_DATA, bytes(self.mem.get(a + i, 0xFF) for i in range(n)))


def selftest():
    assert crc_ccitt(b'123456789') == 0x29B1     # CRC-16/CCITT-FALSE check value
    assert crc_ccitt(b'\x11' + ERASED) == 0xE69E     # SLAU550AB 4.1.5.2, blank device password
    image = {0xFC00 + i: (i * 7) & 0xFF for i in range(301)}    # odd length -> padding
    image.update({0xFFFE: 0x00, 0xFFFF: 0xFC})
    segs = segments(image)
    assert [(a, len(d)) for a, d in segs] == [(0xFC00, 302), (0xFFFE, 2)], segs
    assert load_ihex(':02FFFE0000FC05\n:00000001FF\n') == {0xFFFE: 0x00, 0xFFFF: 0xFC}
    assert load_titxt('@FFFE\n00 FC\nq\n') == {0xFFFE: 0x00, 0xFFFF: 0xFC}

    def fails(f, *args):
        try:
            f(*args)
        except BslError:
            return True
        return False
    assert fails(check_image, {0xFC00: 1})
    assert fails(check_image, {**image, 0xFF84: 0x55})

    old_sleep = time.sleep
    time.sleep = lambda s: None
    try:
        for onewire, dies, needs_low in ((o, d, n) for o in (False, True)
                                         for d in (False, True) for n in (False, True)):
            chip = FakeBsl(onewire=onewire, dies=dies, needs_low=needs_low)

            def run(mem, **kw):
                b = Bsl(OneWire(chip) if onewire else chip)
                b.quench = 0.01 if onewire else 0
                done = flash(b, mem, **kw)
                assert all(chip.mem.get(a) == v for a, v in mem.items())
                b.reset()
                assert not chip.bsl
                return done
            assert run(image) is True                       # blank chip
            assert run(image) is False                      # same image: skipped
            assert run(image, force=True) is True
            image2 = dict(image)
            image2[0xFC10] ^= 0xFF                          # same vectors, other code
            assert run(image2) is True
            image3 = dict(image)
            image3[0xFFE0] = 0x12                           # other vectors
            assert run(image3, old=image2) is True          # ... known
            assert run(image) is True                       # ... unknown: wrong password
            b = Bsl(OneWire(chip) if onewire else chip)
            b.quench = 0.01 if onewire else 0
            erase(b)
            assert not chip.mem
            erase(b)                                        # blank already
            if onewire:
                b.quench = 0
                assert fails(flash, b, image)               # floods without the break
        b = Bsl(FakeBsl(image))
        b.final_low = None
        assert fails(b.start)                               # every first packet lost
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
    ap.add_argument('--old', help='flash: image now in the chip (password); without it a chip with '
                                  'other vectors is erased')
    ap.add_argument('--onewire', action='store_true', help='TX and RX on one line')
    ap.add_argument('--force', action='store_true', help='flash even if up to date')
    ap.add_argument('--no-reset', action='store_true', help='stay in the BSL after flashing')
    ap.add_argument('--test', choices=['tx', 'dtr'], default='tx',
                    help='TEST driven by TX via break (default) or by DTR')
    ap.add_argument('--rst', choices=['rts', 'dtr'], default='rts',
                    help='RST driven by RTS (default) or by DTR')
    ap.add_argument('--invert-test', action='store_true')
    ap.add_argument('--invert-reset', action='store_true')
    ap.add_argument('-v', '--verbose', action='store_true', help='dump BSL packets')
    args = ap.parse_args()
    if args.action == 'flash' and not args.image:
        ap.error('flash needs an image file')
    if args.rst == 'dtr' and args.test == 'dtr':
        ap.error('DTR cannot drive both RST and TEST')

    mem = load_image(args.image) if args.action == 'flash' else None
    old = load_image(args.old) if args.old else None
    ser = open_port(args.port)
    bsl = Bsl(OneWire(ser) if args.onewire else ser, args.test, args.invert_test, args.invert_reset, args.verbose,
              args.rst)
    bsl.quench = 0.01 if args.onewire else 0
    try:
        if args.action == 'flash':
            flash(bsl, mem, args.force, old)
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
