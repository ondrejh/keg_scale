#!/usr/bin/env python3
"""MSP430G2553 ROM BSL over USB-UART, entry code shared with ../fw/tools/bsl.py.

    bsl_g2.py PORT probe                BSL entry + sync only, changes nothing
    bsl_g2.py PORT info  [IMAGE..]      chip ID, BSL version, info memory backup
    bsl_g2.py PORT flash IMAGE [--old OLD.hex]
    bsl_g2.py PORT verify IMAGE
    bsl_g2.py PORT reset
    bsl_g2.py PORT loop [N]             entry + sync + reset into the app, N times
    bsl_g2.py --selftest

IMAGE arguments also serve as BSL passwords. There is no mass erase on purpose: the
BSL's mass erase wipes the DCO calibration (info segment A). So does a WRONG PASSWORD,
unless word 0xFFDE is 0 (it is in the test images). Commands that send a password
therefore need the test app to be running, or --first for a chip with unknown content.
"""
import argparse
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'fw', 'tools'))
from bsl import Bsl, BslError, load_image, open_port, password_of, segments  # noqa: E402
from host import Link, describe                                              # noqa: E402

SYNC, ACK, NAK = 0x80, 0x90, 0xA0
SEG = 512
MAIN, END = 0xC000, 0x10000
LAST = 0xFE00               # segment with BSLSKEY and the vectors: erase first, write last
BLOCK = 128                 # even, <= 250


def cksum(b):
    lo = hi = 0
    for i, x in enumerate(b):
        if i & 1:
            hi ^= x
        else:
            lo ^= x
    return bytes([~lo & 0xFF, ~hi & 0xFF])


class Nak(BslError):
    pass


class G2Bsl(Bsl):
    manual = False
    synced = False      # sync already acknowledged, command frame expected next

    def rst(self, high):
        if self.manual:
            input('>>> %s RST, then Enter ' % ('RELEASE' if high else 'connect to GND:'))
        else:
            super().rst(high)

    def sync(self):
        time.sleep(0.002)               # >= 1.2 ms after the last received character
        self.ser.reset_input_buffer()
        self.ser.write(bytes([SYNC]))
        r = self._read(1)[0]
        if r != ACK:
            raise BslError('sync: got 0x%02X instead of 0x90' % r)
        time.sleep(0.002)

    def g2(self, cmd, addr=0, ll=0, lh=0, data=b'', reply=False):
        n = 4 + len(data)
        assert n % 2 == 0 and n < 255
        pkt = bytes([0x80, cmd, n, n, addr & 0xFF, addr >> 8, ll, lh]) + data
        pkt += cksum(pkt)
        if self.verbose:
            print('  >', pkt.hex(' '))
        if not self.synced:
            self.sync()
        self.synced = False
        self.ser.write(pkt)
        r = self._read(1)[0]
        if r == NAK:
            raise Nak('NAK to command 0x%02X' % cmd)
        if not reply:
            if r != ACK:
                raise BslError('command 0x%02X: got 0x%02X' % (cmd, r))
            return b''
        if r != 0x80:
            raise BslError('command 0x%02X: reply header 0x%02X' % (cmd, r))
        hdr = bytes([r]) + self._read(3)
        body = self._read(hdr[2])
        if self._read(2) != cksum(hdr + body):
            raise BslError('reply checksum')
        if self.verbose:
            print('  <', (hdr + body).hex(' '))
        return body

    def start(self, tries=3):
        """Hardware entry + first sync. Returns the number of tries used."""
        for i in range(1, tries + 1):
            self.synced = False
            self.enter()
            try:
                self.sync()
                self.synced = True
                return i
            except BslError as e:
                err = e
        raise BslError('no BSL after %d entries (%s)' % (tries, err))

    def read_mem(self, addr, n):
        out = b''
        while n:
            k = min(n, BLOCK)
            out += self.g2(0x14, addr, k, 0, reply=True)
            addr, n = addr + k, n - k
        return out

    def unlocked(self):
        try:
            return self.read_mem(0x0FF0, 16)
        except Nak:
            return None

    def unlock(self, passwords):
        """Try FF (blank or half-flashed chip) first, then the images. Returns the ID block."""
        for name, pw in [('FF', b'\xFF' * 32)] + passwords:
            try:
                self.g2(0x10, data=pw)
            except Nak:
                pass
            except BslError:            # no answer: the BSL may be erasing
                time.sleep(0.5)
                self.start()
                continue
            chip = self.unlocked()
            if chip:
                print('unlocked with password: %s' % name)
                return chip
        # a wrong password on a chip with BSLSKEY != 0 has erased it: FF works after re-entry
        self.start()
        self.g2(0x10, data=b'\xFF' * 32)
        chip = self.unlocked()
        if not chip:
            raise BslError('no password accepted')
        print('unlocked with FF after re-entry: the chip was erased by a wrong password')
        return chip

    def erase_segment(self, addr):
        self.g2(0x16, addr, 0x02, 0xA5)

    def write_mem(self, addr, data):
        self.g2(0x12, addr, len(data), 0, data=data)


def check_image(mem):
    if any(a < MAIN or a >= END for a in mem):
        raise BslError('image has data outside C000-FFFF')
    if (mem.get(0xFFDE), mem.get(0xFFDF)) != (0, 0):
        raise BslError('image: word FFDE (BSLSKEY) is not 0, refusing')


def blocks(mem):
    """(addr, data), vector segment last, never across a segment boundary."""
    out = []
    for a, d in segments(mem):
        while d:
            k = min(len(d), BLOCK, SEG - a % SEG)
            out.append((a, d[:k]))
            a, d = a + k, d[k:]
    return sorted(out, key=lambda b: (b[0] >= LAST, b[0]))


def show_info(bsl, chip):
    print('chip ID %s, BSL version %s' % (chip[0:2].hex(), chip[10:12].hex()))
    info = bsl.read_mem(0x1000, 256)
    a = info[0xC0:]
    x = 0
    for i in range(2, 64, 2):
        x ^= a[i] | (a[i + 1] << 8)
    good = ((a[0] | (a[1] << 8)) + x) & 0xFFFF == 0
    blank = all(b == 0xFF for b in a)
    print('info segment A: %s' % ('ERASED (no DCO calibration)' if blank else
                                  'checksum ok, calibration present' if good else 'checksum BAD'))
    if not blank:
        d = os.path.join(HERE, 'backup')
        os.makedirs(d, exist_ok=True)
        p = os.path.join(d, time.strftime('info_%Y%m%d_%H%M%S.bin'))
        with open(p, 'wb') as f:
            f.write(info)
        print('info memory saved to', p)


def do_verify(bsl, mem):
    for a, d in blocks(mem):
        got = bsl.read_mem(a, len(d))
        if got != d:
            i = next(i for i in range(len(d)) if got[i] != d[i])
            raise BslError('verify: %04X is %02X, expected %02X' % (a + i, got[i], d[i]))
    print('verify ok (%d bytes)' % sum(len(d) for _, d in blocks(mem)))


def do_flash(bsl, mem):
    check_image(mem)
    for a in [LAST] + list(range(MAIN, LAST, SEG)):
        bsl.erase_segment(a)
        print('\rerased %04X' % a, end='', flush=True)
    print()
    total, done = sum(len(d) for _, d in blocks(mem)), 0
    for a, d in blocks(mem):
        bsl.write_mem(a, d)
        done += len(d)
        print('\rwriting %d/%d' % (done, total), end='', flush=True)
    print()
    do_verify(bsl, mem)


def loop(bsl, link, n):
    bad = 0
    for i in range(n):
        link.boots.clear()
        link.buf = b''
        try:
            tries = bsl.start(1)
        except BslError as e:
            tries, bad = 0, bad + 1
            print('%d: %s' % (i + 1, e))
        bsl.reset()
        if not link.alive() or not link.boots:
            bad += 1
            print('%d: app did not start after reset' % (i + 1))
        elif tries:
            print('\r%d ok' % (i + 1), end='', flush=True)
    print('\n%d loops, %d failures (%d boot frames with junk in front): %s'
          % (n, bad, link.junk, 'FAIL' if bad else 'PASS'))
    return not bad


# ---- self test ----

class FakeG2:
    rts = break_condition = False

    @property
    def dtr(self):
        return False

    @dtr.setter
    def dtr(self, low):
        if low:                     # RST
            self.synced = self.unlocked = False

    def __init__(self, main=None):
        self.mem = {a: 0xFF for a in range(0x1000, 0x10000)}
        self.mem.update({0x10C0 + i: 0x5A for i in range(64)})
        self.mem.update(main or {})
        for i, b in enumerate(bytes.fromhex('2553') + bytes(8) + bytes.fromhex('0203') + bytes(4)):
            self.mem[0x0FF0 + i] = b
        self.out, self.unlocked, self.synced = bytearray(), False, False
        self.die_after = None       # stop after N write commands (interrupted flash)

    def reset_input_buffer(self):
        self.out.clear()

    def read(self, n):
        d, self.out[:n] = bytes(self.out[:n]), b''
        return d

    def write(self, p):
        if not self.synced:
            assert p == b'\x80'
            self.synced = True
            self.out.append(ACK)
            return
        self.synced = False
        assert p[2] == p[3] == len(p) - 6 and p[-2:] == cksum(p[:-2])
        cmd, a, ll, d = p[1], p[4] | p[5] << 8, p[6], p[8:-2]
        ok = True
        if cmd == 0x10:
            self.unlocked = d == bytes(self.mem[0xFFE0 + i] for i in range(32))
            if not self.unlocked and (self.mem[0xFFDE], self.mem[0xFFDF]) != (0, 0):
                for x in range(0x1000, 0x10000):
                    self.mem[x] = 0xFF
        elif not self.unlocked:
            ok = False
        elif cmd == 0x14:
            h = bytes([0x80, 0, ll, ll]) + bytes(self.mem[a + i] for i in range(ll))
            self.out += h + cksum(h)
            return
        elif cmd == 0x16:
            assert p[6:8] == b'\x02\xA5'
            for x in range(a & ~511, (a & ~511) + 512):
                self.mem[x] = 0xFF
        elif cmd == 0x12:
            if self.die_after is not None:
                self.die_after -= 1
                if self.die_after < 0:
                    return
            for i, b in enumerate(d):
                assert self.mem[a + i] == 0xFF
                self.mem[a + i] = b
        else:
            ok = False
        self.out.append(ACK if ok else NAK)


def selftest():
    # frame example from SLAU319, 2.4.3
    p = bytes.fromhex('80140404000F0E00')
    assert cksum(p) == bytes.fromhex('75E0')
    img = {0xC000 + i: (i * 7) & 0xFF for i in range(1301)}
    img.update({0xFFDE: 0, 0xFFDF: 0, 0xFFFE: 0, 0xFFFF: 0xC0})
    img2 = dict(img)
    img2[0xFFE4] = 0x34
    img2[0xFFE5] = 0xC1
    assert blocks(img)[-1][0] >= LAST
    old_sleep, time.sleep = time.sleep, lambda s: None
    try:
        def run(chip, mem, pws):
            b = G2Bsl(chip, rst='dtr')
            b.start()
            b.unlock([(n, password_of(m)) for n, m in pws])
            do_flash(b, mem)
            assert all(chip.mem[a] == v for a, v in mem.items())

        def calibrated(chip):
            return chip.mem[0x10C0] == 0x5A
        chip = FakeG2()
        run(chip, img, [])                          # blank
        run(chip, img2, [('old', img)])             # reflash, password from old image
        assert calibrated(chip)
        chip.die_after = 3                          # interrupted flash ...
        try:
            run(chip, img, [('old', img2)])
            raise AssertionError
        except BslError:
            pass
        chip.die_after = None
        run(chip, img, [('old', img)])              # ... recovers with FF
        assert calibrated(chip)
        for bad in ({0xC000: 1}, {0x10C0: 1, 0xFFDE: 0, 0xFFDF: 0}):
            try:
                check_image(bad)
                raise AssertionError
            except BslError:
                pass
        foreign = FakeG2({0xFFFE: 0x12})            # unknown app, BSLSKEY = FFFF
        run(foreign, img, [])
        assert not calibrated(foreign)              # erased: why --first exists
    finally:
        time.sleep = old_sleep
    print('selftest ok')


def main():
    if '--selftest' in sys.argv[1:]:
        selftest()
        return
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('port')
    ap.add_argument('action', choices=['probe', 'info', 'flash', 'verify', 'reset', 'loop'])
    ap.add_argument('args', nargs='*', help='image file(s), or N for loop')
    ap.add_argument('--old', help='image now in the chip (password)')
    ap.add_argument('--first', action='store_true',
                    help='chip content unknown: accept that a wrong password may erase '
                         'everything including the DCO calibration')
    ap.add_argument('--rst', choices=['rts', 'dtr', 'manual'], default='rts')
    ap.add_argument('--test', choices=['tx', 'dtr'], default='tx')
    ap.add_argument('--no-reset', action='store_true')
    ap.add_argument('-v', '--verbose', action='store_true')
    a = ap.parse_args()
    if a.test == 'dtr' and a.rst == 'dtr':
        ap.error('--test dtr needs --rst manual or rts')
    if a.action in ('flash', 'verify') and len(a.args) != 1:
        ap.error('needs one image file')
    files = ([] if a.action == 'loop' else list(a.args)) + ([a.old] if a.old else [])
    images = [(f, load_image(f)) for f in files]
    if a.action == 'flash':
        check_image(images[0][1])

    ser = open_port(a.port)
    bsl = G2Bsl(ser, a.test, verbose=a.verbose, rst='rts' if a.rst == 'rts' else 'dtr')
    bsl.manual = a.rst == 'manual'
    link = Link(ser)
    try:
        if a.action == 'reset':
            bsl.reset()
        elif a.action == 'loop':
            sys.exit(0 if loop(bsl, link, int(a.args[0]) if a.args else 50) else 1)
        else:
            if a.action != 'probe' and not a.first and link.alive() is None:
                raise BslError('test app not answering, chip content unknown: see --first')
            ser.timeout = 1.0
            print('BSL entered and synchronised (try %d)' % bsl.start())
            if a.action != 'probe':
                chip = bsl.unlock([(os.path.basename(f), password_of(m)) for f, m in images])
                show_info(bsl, chip)
                if a.action == 'flash':
                    do_flash(bsl, images[0][1])
                elif a.action == 'verify':
                    do_verify(bsl, images[0][1])
        if not a.no_reset and a.action != 'reset':
            bsl.reset()
        if not a.no_reset and not bsl.manual:
            v = link.alive()
            print('app: %s' % ('version %d' % v if v else 'no answer'))
            for b in link.boots:
                print('  ', describe(('B', b)))
    except BslError as e:
        sys.exit('error: %s' % e)
    finally:
        ser.close()


if __name__ == '__main__':
    main()
