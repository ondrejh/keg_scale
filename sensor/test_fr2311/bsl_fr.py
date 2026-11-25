#!/usr/bin/env python3
"""MSP430FR2311 ROM BSL over USB-UART. Entry, reset and packets come from ../fw/tools/bsl.py.

    bsl_fr.py PORT probe [N]            BSL entry, N tries per variant, changes nothing
    bsl_fr.py PORT info IMAGE           BSL version, which commands the BSL knows
    bsl_fr.py PORT flash IMAGE [--old OLD.hex] [--force]
    bsl_fr.py PORT verify IMAGE
    bsl_fr.py PORT reset
    bsl_fr.py PORT loop [N] IMAGE       entry + password + read + reset into the app, N times
    bsl_fr.py --selftest

IMAGE is the program in the chip (its vectors are the BSL password); for flash the new one,
with --old for the one it replaces. A WRONG PASSWORD ERASES THE WHOLE PROGRAM MEMORY, so
commands that send one need the test app to answer, or --first.

Flashing uses only what the FR2111 BSL has too: password, write, read back. Not Mass Erase
(0x15) and CRC Check (0x16): the FR2311 has them, the FR2111 doesn't (SLAU550AB table 4-9).
"""
import argparse
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'fw', 'tools'))
from bsl import (BLOCK, Bsl, BslError, CMD_RX_DATA, CMD_RX_PASSWORD, CMD_TX_DATA, ERASED,  # noqa: E402
                 MESSAGES, OneWire, RSP_DATA, RSP_MESSAGE, addr3, crc_ccitt, load_image,
                 open_port, password_of, segments)
from bsl import check_image as check_common, difference as differences      # noqa: E402
from host_fr import Link, describe, wait_boot           # noqa: E402

CMD_CRC_CHECK = 0x16
CMD_TX_VERSION = 0x19
START, END = 0xF100, 0x10000        # FRAM, same on the FR2111


class FrBsl(Bsl):
    stray = b''             # bytes nobody asked for

    def command(self, core, response=True):
        time.sleep(0.003)
        if self.ser.in_waiting:
            s = self.ser.read(self.ser.in_waiting)
            self.stray += s
            if self.verbose:
                print('  ? stray', s.hex(' '))
        return super().command(core, response)

    def unlock_erased(self):
        """After a wrong password: FF password, with a new entry if the session is gone."""
        try:
            if self.password(ERASED):
                print('erased: same BSL session goes on')
                return
        except BslError as e:
            print('erased: BSL stopped answering (%s), new entry' % e)
        time.sleep(0.5)
        self.start()
        if not self.password(ERASED):
            raise BslError('FF password rejected: chip not erased (BSL signature?)')


def check_image(mem):
    if any(a < START or a >= END for a in mem):
        raise BslError('image has data outside %04X-%04X' % (START, END - 1))
    check_common(mem)


def do_verify(bsl, mem):
    diff = differences(bsl, mem)
    if diff:
        raise BslError('verify: ' + diff)
    print('verify ok (%d bytes)' % sum(len(d) for _, d in segments(mem)))


def do_flash(bsl, mem, old=None, force=False):
    """The chip is in the BSL and locked. False: was up to date."""
    check_image(mem)
    pw = password_of(old or mem)
    if bsl.password(pw):
        print('password accepted')
        if not force and differences(bsl, mem) is None:
            print('firmware up to date')
            return False
        print('erasing with a wrong password')
        if bsl.password(bytes(b ^ 0xFF for b in pw)):
            raise BslError('a wrong password was accepted')
    else:
        print('password rejected')
    bsl.unlock_erased()
    blank = bsl.read_mem(START, END - START)
    if blank != b'\xFF' * len(blank):
        # not fatal: FRAM can be overwritten. But it is not what SLAU550 says
        n = sum(b != 0xFF for b in blank)
        print('NOTE: %d of %d bytes are not FF after the erase' % (n, len(blank)))
    else:
        print('%04X-%04X blank' % (START, END - 1))
    segs = segments(mem)                # ascending: the reset vector goes last
    total, done = sum(len(d) for _, d in segs), 0
    for a, d in segs:
        for off in range(0, len(d), BLOCK):
            bsl.write(a + off, d[off:off + BLOCK])
            done += len(d[off:off + BLOCK])
            print('\rwriting %d/%d' % (done, total), end='', flush=True)
    print()
    do_verify(bsl, mem)
    return True


def do_info(bsl, mem):
    """Needs the unlocked BSL."""
    for name, core in (('TX BSL version', bytes([CMD_TX_VERSION])),
                       ('CRC check', bytes([CMD_CRC_CHECK]) + addr3(0xFFE0) + b'\x20\x00')):
        try:
            cmd, data = bsl.command(core)
        except BslError as e:
            print('%s (0x%02X): %s' % (name, core[0], e))
            continue
        if cmd == RSP_MESSAGE:
            print('%s (0x%02X): %s' % (name, core[0], MESSAGES.get(data[0], data.hex())))
        elif core[0] == CMD_CRC_CHECK:
            ok = data == crc_ccitt(password_of(mem)).to_bytes(2, 'little')
            print('%s (0x%02X): supported, value %s' % (name, core[0], 'ok' if ok else
                                                       'WRONG ' + data.hex()))
        else:
            print('%s (0x%02X): %s' % (name, core[0], data.hex('.')))


def probe(bsl, n):
    """Which entry variants work? Sends no password."""
    for low, name in ((None, 'TEST stays high'), (0.001, 'TEST low 1 ms after RST'),
                      (0.005, 'TEST low 5 ms after RST'), (0.05, 'TEST low 50 ms after RST')):
        bsl.final_low = low
        good, errs = 0, []
        for _ in range(n):
            bsl.enter()
            try:
                good += bsl.state() == 'locked'
            except BslError as e:
                errs.append(str(e))
                bsl.ser.reset_input_buffer()
            bsl.reset()                 # every try starts from the app
            time.sleep(0.1)
        print('%-26s %d/%d%s' % (name, good, n, '  (%s)' % errs[0] if errs else ''), flush=True)
    bsl.final_low = FrBsl.final_low


def loop(bsl, link, mem, n):
    bad = 0
    for i in range(n):
        # A blank chip enters the BSL by itself: a running app is what makes this a test
        if link.alive() is None:
            print('%d: app not running, stopped' % (i + 1))
            bad += 1
            break
        try:
            bsl.ser.timeout = 1.0
            bsl.start(1)
            if not bsl.password(password_of(mem)):
                raise BslError('password rejected: chip is ERASED now')
            if bsl.read_mem(0xFFE0, 32) != password_of(mem):
                raise BslError('vectors read back wrong')
        except BslError as e:
            bad += 1
            print('%d: %s' % (i + 1, e))
        link.boots.clear()
        link.buf = b''
        bsl.reset()
        if wait_boot(link, 2) is None:
            bad += 1
            print('%d: app did not start after reset' % (i + 1))
        else:
            print('\r%d' % (i + 1), end='', flush=True)
    print('\n%d loops, %d failures (%d boot frames with junk in front, stray BSL bytes %s): %s'
          % (n, bad, link.junk, bsl.stray.hex(' ') or 'none', 'FAIL' if bad else 'PASS'))
    return not bad


# ---- self test ----

class FakeFr:
    """FR2xxx with entry sequence, BSL and (silent) app."""
    timeout = 0

    def __init__(self, mem=None, fr2111=False, dies=False):
        self.mem = dict(mem or {})
        self.fr2111, self.dies, self.armed = fr2111, dies, False
        self.out, self.bsl, self.unlocked, self.edges = bytearray(), False, False, 0
        self.t, self.r = True, True
        self.cmds = set()

    # TEST = not break, RST = not rts
    @property
    def break_condition(self):
        return not self.t

    @break_condition.setter
    def break_condition(self, v):
        if not self.t and not v and not self.r:
            self.edges += 1
        if v and self.r and self.armed:
            self.bsl, self.armed = True, False      # starts with TEST low
        self.t = not v

    @property
    def rts(self):
        return not self.r

    @rts.setter
    def rts(self, v):
        if v and self.r:
            self.edges = 0
        if not v and not self.r:                    # RST rises
            self.unlocked = False
            self.bsl, self.armed = False, self.t and self.edges >= 2
        self.r = not v

    dtr = False

    @property
    def in_waiting(self):
        return len(self.out)

    def reset_input_buffer(self):
        self.out.clear()

    def read(self, n):
        d, self.out[:n] = bytes(self.out[:n]), b''
        return d

    def reply(self, cmd, data):
        core = bytes([cmd]) + data
        self.out += bytes([0x80, len(core) & 255, len(core) >> 8]) + core
        self.out += crc_ccitt(core).to_bytes(2, 'little')

    def write(self, p):
        if not self.bsl:
            self.bsl, self.armed = self.armed, False    # the first packet's low bits
            return
        n = p[1] | p[2] << 8
        core = p[3:3 + n]
        assert p[0] == 0x80 and crc_ccitt(core).to_bytes(2, 'little') == p[3 + n:]
        cmd, data = core[0], core[1:]
        self.cmds.add(cmd)
        self.out.append(0)
        if cmd == CMD_RX_PASSWORD:
            self.unlocked = data == bytes(self.mem.get(0xFFE0 + i, 0xFF) for i in range(32))
            if not self.unlocked:
                self.mem.clear()
                if self.dies:
                    self.bsl = False
            self.reply(RSP_MESSAGE, b'\x00' if self.unlocked else b'\x05')
        elif self.fr2111 and cmd in (0x15, CMD_CRC_CHECK, CMD_TX_VERSION):
            self.reply(RSP_MESSAGE, b'\x07')
        elif not self.unlocked:
            self.reply(RSP_MESSAGE, b'\x04')
        elif cmd == CMD_RX_DATA:
            a = data[0] | data[1] << 8
            self.mem.update({a + i: b for i, b in enumerate(data[3:])})
            self.reply(RSP_MESSAGE, b'\x00')
        elif cmd == CMD_TX_DATA:
            a, k = data[0] | data[1] << 8, data[3] | data[4] << 8
            self.reply(RSP_DATA, bytes(self.mem.get(a + i, 0xFF) for i in range(k)))
        elif cmd == CMD_TX_VERSION:
            self.reply(RSP_DATA, bytes.fromhex('000835B3'))
        elif cmd == CMD_CRC_CHECK:
            a, k = data[0] | data[1] << 8, data[3] | data[4] << 8
            c = crc_ccitt(bytes(self.mem.get(a + i, 0xFF) for i in range(k)))
            self.reply(RSP_DATA, c.to_bytes(2, 'little'))
        else:
            raise AssertionError(cmd)


class FakeOneWire(FakeFr):
    """The BSL hears its own reply: whatever comes before the next break is lost."""
    flood = False

    @property
    def break_condition(self):
        return FakeFr.break_condition.fget(self)

    @break_condition.setter
    def break_condition(self, v):
        FakeFr.break_condition.fset(self, v)
        self.flood = False

    def write(self, p):
        self.out += p
        if self.flood:
            self.out += b'\x51'
            return
        was = len(self.out)
        super().write(p)
        self.flood = len(self.out) > was


def selftest():
    img = {START + i: (i * 7) & 0xFF for i in range(1301)}
    img.update({0xFFEC: 0x10, 0xFFED: 0xF1, 0xFFFE: 0x14, 0xFFFF: 0xF1})
    img2 = dict(img)
    img2[0xF110] ^= 0xFF
    img3 = dict(img)
    img3[0xFFEC] ^= 0x22
    old_sleep, time.sleep = time.sleep, lambda s: None
    try:
        for kind in (FakeFr, FakeOneWire):
            for fr2111 in (False, True):
                for dies in (False, True):
                    chip = kind(fr2111=fr2111, dies=dies)
                    bsl = FrBsl(OneWire(chip) if kind is FakeOneWire else chip)

                    bsl.quench = 0.01 if kind is FakeOneWire else 0

                    def run(mem, old=None, force=False):
                        bsl.start(1)
                        r = do_flash(bsl, mem, old, force)
                        assert all(chip.mem[a] == v for a, v in mem.items())
                        bsl.reset()
                        assert not chip.bsl
                        return r
                    assert run(img)                     # blank chip, FF password differs
                    assert not run(img)                 # up to date
                    assert run(img, force=True)
                    assert run(img2, img)               # other code, same vectors
                    assert run(img3, img2)              # other vectors
                    assert run(img)                     # wrong password
                    chip.mem[0xF100] ^= 1
                    assert run(img)
                    assert not chip.cmds & {0x15, CMD_CRC_CHECK, CMD_TX_VERSION}
                    bsl.start(1)
                    assert bsl.password(password_of(img))
                    do_info(bsl, img)
        chip = FakeFr(img)
        probe(FrBsl(chip), 3)
        assert chip.mem == img
        b = FrBsl(FakeFr(img))
        b.final_low = None
        try:
            b.start()
            raise AssertionError
        except BslError:
            pass
        for bad in ({0xC000: 1}, {0xFF84: 0x55, 0xFFFE: 0, 0xFFFF: 0xF1}, {0xF100: 1}):
            try:
                check_image(bad)
                raise AssertionError
            except BslError:
                pass
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
    ap.add_argument('args', nargs='*', help='image file, and/or N for probe and loop')
    ap.add_argument('--old', help='flash: image now in the chip (password)')
    ap.add_argument('--first', action='store_true',
                    help='chip content unknown: accept that it gets erased')
    ap.add_argument('--force', action='store_true', help='flash even if up to date')
    ap.add_argument('--rst', choices=['rts', 'dtr'], default='rts')
    ap.add_argument('--test', choices=['tx', 'dtr'], default='tx')
    ap.add_argument('--no-reset', action='store_true')
    ap.add_argument('--onewire', action='store_true', help='TX and RX on one line')
    ap.add_argument('--quench', type=float, default=10, metavar='MS',
                    help='one wire: break after each BSL reply (0: none)')
    ap.add_argument('-v', '--verbose', action='store_true')
    a = ap.parse_args()
    if a.test == a.rst == 'dtr':
        ap.error('DTR cannot drive both RST and TEST')
    nums = [int(x) for x in a.args if x.isdigit()]
    files = [x for x in a.args if not x.isdigit()]
    if a.action in ('info', 'flash', 'verify', 'loop') and len(files) != 1:
        ap.error('needs one image file')
    mem = load_image(files[0]) if files else None
    old = load_image(a.old) if a.old else None

    ser = open_port(a.port)
    bsl = FrBsl(OneWire(ser) if a.onewire else ser, a.test, verbose=a.verbose, rst=a.rst)
    bsl.quench = a.quench / 1000 if a.onewire else 0
    link = Link(ser, onewire=a.onewire)
    ok = True
    try:
        if mem:
            check_image(mem)
        if a.action == 'reset':
            bsl.reset()
            return
        v = link.alive()
        ser.timeout = 1.0
        if a.action == 'probe':
            print('app: %s' % ('version %d' % v if v else
                               'no answer (a blank FR2311 enters the BSL by itself: no proof)'))
            probe(bsl, nums[0] if nums else 10)
        elif v is None and not a.first:
            raise BslError('test app not answering, chip content unknown: see --first')
        elif a.action == 'loop':
            ok = loop(bsl, link, mem, nums[0] if nums else 50)
        else:
            print('BSL entered (try %d)' % bsl.start())
            if a.action == 'flash':
                do_flash(bsl, mem, old, a.force)
            elif not bsl.password(password_of(mem)):
                raise BslError('password rejected: the chip is ERASED now')
            elif a.action == 'verify':
                do_verify(bsl, mem)
            else:
                do_info(bsl, mem)
        if a.action != 'loop' and not a.no_reset:
            link.boots.clear()
            link.buf = b''
            bsl.reset()
            b = wait_boot(link, 2)
            print('app: %s' % (describe(('B', b)) if b else 'no boot frame after reset'))
            if bsl.stray:
                print('stray BSL bytes: %s' % bsl.stray.hex(' '))
        sys.exit(0 if ok else 1)
    except BslError as e:
        if bsl.stray:
            print('stray BSL bytes: %s' % bsl.stray.hex(' '))
        sys.exit('error: %s' % e)
    finally:
        ser.close()


if __name__ == '__main__':
    main()
