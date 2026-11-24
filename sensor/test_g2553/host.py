#!/usr/bin/env python3
"""UART side of the BSL entry bench test (see README.md).

    host.py PORT monitor [SECONDS]
    host.py PORT cmd V | S | J | P 0055AAFF | L 02 0A
    host.py PORT stress [N]         pings with random and worst-case data
    host.py PORT break [MS]         TX (= TEST, RX) low for MS, MSP must survive
    host.py PORT rst-test           RST pulse with TEST high, then with TEST low
    host.py --selftest

RST is on RTS (active low) unless --rst dtr.
"""
import argparse
import os
import random
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'fw', 'tools'))
from bsl import Bsl, open_port               # noqa: E402
from decode import crc8, make_frame          # noqa: E402

ASYNC = 'BT'


def parse(line):
    """(type, payload bytes), or None."""
    line = line.strip()
    if len(line) < 4 or line[-3] != '*':
        return None
    body = line[:-3]
    try:
        if crc8(body.encode()) != int(line[-2:], 16):
            return None
        return body[0], bytes.fromhex(body[1:])
    except (ValueError, UnicodeError):
        return None


class Link:
    def __init__(self, ser, verbose=False):
        self.ser = ser
        self.verbose = verbose
        self.buf = b''
        self.bad = []           # undecodable lines
        self.boots = []         # B frames
        self.junk = 0           # boot frames with junk in front
        self.beats = []         # (time, uptime) of T frames

    def poll(self, timeout):
        """Next frame (type, payload) or None. Records B and T frames."""
        end = time.time() + timeout
        while True:
            while b'\n' in self.buf:
                raw, self.buf = self.buf.split(b'\n', 1)
                f = parse(raw.decode('latin1'))
                if f is None:
                    # junk at reset (TX glitch, BSL NAK) glued to the boot frame, only there
                    g = parse(raw.lstrip(bytes(range(0x80, 0x100))).decode('latin1'))
                    if g and g[0] == 'B':
                        f, self.junk = g, self.junk + 1
                if self.verbose:
                    print('   <', raw)
                if f is None:
                    self.bad.append(raw)
                    continue
                if f[0] == 'B':
                    self.boots.append(f[1])
                elif f[0] == 'T':
                    self.beats.append((time.time(), int.from_bytes(f[1], 'big')))
                return f
            left = end - time.time()
            if left <= 0:
                return None
            self.ser.timeout = min(left, 0.05)
            self.buf += self.ser.read(max(1, self.ser.in_waiting))

    def send(self, typ, payload=b''):
        self.ser.write((make_frame(typ, payload.hex().upper()) + '\n').encode())

    def request(self, typ, payload=b'', timeout=0.5):
        """Send, return the answer (B/T frames are recorded and skipped), or None."""
        self.send(typ, payload)
        end = time.time() + timeout
        while True:
            f = self.poll(max(0, end - time.time()))
            if f is None or f[0] not in ASYNC:
                return f

    def sync(self):
        """Autobaud pulses for a chip without calibration; ignored otherwise."""
        for _ in range(8):
            self.ser.write(b'\x80')
            self.ser.flush()
            time.sleep(0.01)

    def alive(self):
        """Version of the running test app, or None."""
        self.sync()
        self.ser.write(b'\n')
        for _ in range(3):
            f = self.request('V')
            if f and f[0] == 'V' and len(f[1]) == 1:
                return f[1][0]
        return None


def describe(f):
    t, d = f
    w = [int.from_bytes(d[i:i + 2], 'big') for i in range(0, len(d) - 1, 2)]
    if t == 'B' and len(d) == 3:
        return 'boot: version %d, cause IFG1=%02X%s, %s' % (
            d[0], d[1], cause(d[1]), 'factory DCO' if d[2] else 'NO CALIBRATION, autobaud')
    if t == 'T':
        return 'uptime %d s' % w[0]
    if t == 'V':
        return 'version %d' % d[0]
    if t == 's' and len(d) == 12:
        return ('uptime %d s, cause %02X%s, rx ok %d, parity %d, framing %d, bad frames %d, '
                'overrun %d' % (w[0], d[2], cause(d[2]),
                                *[int.from_bytes(d[i:i + 2], 'big') for i in (3, 5, 7, 9)], d[11]))
    if t == 'j' and len(d) == 16:
        return ('P1.5 vs TEST line: TEST high %d/%d wrong, TEST low %d/%d wrong; '
                'P1.7 (TDO) readback: TEST high %d/%d wrong, TEST low %d/%d wrong' % tuple(w))
    return d.hex(' ')


def cause(c):
    names = [n for b, n in ((1, 'WDT'), (4, 'POR'), (8, 'RST pin'), (16, 'NMI')) if c & b]
    return ' (%s)' % ', '.join(names) if names else ''


def monitor(link, seconds):
    end = time.time() + seconds if seconds else float('inf')
    nbad = 0
    while time.time() < end:
        f = link.poll(0.2)
        if f:
            print('%s  %s' % (f[0], describe(f)), flush=True)
        for raw in link.bad[nbad:]:
            print('!! ', raw, flush=True)
        nbad = len(link.bad)


def stress(link, n):
    s0 = link.request('S')
    if not s0 or s0[0] != 's':
        sys.exit('no status from the MSP')
    print('before:', describe(s0))
    link.boots.clear()
    link.bad.clear()
    lost = wrong = 0
    fixed = [b'\x00' * 8, b'\xFF' * 8, b'\x55' * 8, b'\xAA' * 8, b'\x80' * 8, b'\x01']
    t0 = time.time()
    for i in range(n):
        data = fixed[i // 2 % len(fixed)] if i % 2 else \
            bytes(random.randrange(256) for _ in range(random.randint(1, 8)))
        f = link.request('P', data)
        if f is None:
            lost += 1
            link.ser.write(b'\n')       # resynchronise the line parser
        elif f != ('p', bytes(b ^ 0xFF for b in data)):
            wrong += 1
    dt = time.time() - t0
    s1 = link.request('S')
    print('after: ', describe(s1) if s1 else 'no status')
    beats = [u for _, u in link.beats]
    gaps = sum(1 for a, b in zip(beats, beats[1:]) if b != a + 1)
    ok = s1 is not None and not (lost or wrong or link.boots or link.bad or gaps)
    if s1:
        ok = ok and s1[1][3:] is not None and s1[1][5:] == s0[1][5:]
    print('%d pings in %.0f s: lost %d, wrong %d, bad lines %d, heartbeat gaps %d, resets %d'
          % (n, dt, lost, wrong, len(link.bad), gaps, len(link.boots)))
    print('PASS' if ok else 'FAIL')
    return ok


def brk(link, ms):
    s0 = link.request('S')
    link.boots.clear()
    link.ser.break_condition = True
    time.sleep(ms / 1000.0)
    link.ser.break_condition = False
    time.sleep(0.05)
    link.ser.write(b'\n')
    s1 = link.request('S')
    print('before:', describe(s0) if s0 else None)
    print('after: ', describe(s1) if s1 else None)
    print('resets %d -> %s' % (len(link.boots),
                               'PASS' if s1 and not link.boots else 'FAIL'))


def rst_test(link, bsl):
    for test_high in (True, False):
        link.boots.clear()
        if not test_high:
            bsl.test(False)
        time.sleep(0.02)
        bsl.rst(False)
        time.sleep(0.02)
        bsl.rst(True)
        time.sleep(0.02)
        if not test_high:
            bsl.release_test()
        end = time.time() + 1.5
        while time.time() < end and not link.boots:
            link.poll(0.1)
        if not link.boots:
            link.alive()        # a chip without calibration boots only after a sync
        print('RST pulse, TEST %s: %s' % ('high' if test_high else 'low ',
                                          'reset' if link.boots else 'no reset'))


def selftest():
    assert parse(make_frame('V', '01')) == ('V', b'\x01')
    for raw, ok in ((b'\xa0\xff', True), (b'x', False)):
        for typ in 'BT':
            class One:
                timeout, in_waiting = 0, 0
                data = raw + make_frame(typ, '020A00').encode() + b'\n'

                def read(self, n):
                    d, self.data = self.data, b''
                    return d
            lk = Link(One())
            lk.poll(0.05)
            assert bool(lk.boots) == (ok and typ == 'B') and bool(lk.bad) != bool(lk.boots)
    assert parse('V01*00') is None and parse('') is None and parse('x*zz') is None
    for t, n in (('B', 3), ('T', 2), ('V', 1), ('s', 12), ('j', 16), ('p', 4)):
        describe((t, bytes(n)))

    class Echo:                 # MSP model
        def __init__(self):
            self.out, self.line, self.n = b'', b'', 0
            self.timeout = 0

        @property
        def in_waiting(self):
            return len(self.out)

        def read(self, n):
            d, self.out = self.out[:n], self.out[n:]
            return d

        def flush(self):
            pass

        def write(self, data):
            for c in data:
                if c == 10:
                    f, self.line = parse(self.line.decode('latin1')), b''
                    if f and f[0] == 'P':
                        self.out += (make_frame('p', bytes(b ^ 255 for b in f[1]).hex().upper())
                                     + '\n').encode()
                    elif f and f[0] == 'S':
                        self.out += (make_frame('s', '00' * 12) + '\n').encode()
                    elif f and f[0] == 'V':
                        self.out += (make_frame('V', '02') + '\n').encode()
                elif c >= 0x7F:
                    self.line = b''
                else:
                    self.line += bytes([c])
    assert Link(Echo()).alive() == 2
    assert stress(Link(Echo()), 50)
    e = Echo()
    e.write = lambda d: None
    assert Link(e).request('V', timeout=0.05) is None
    print('selftest ok')


def main():
    if '--selftest' in sys.argv[1:]:
        selftest()
        return
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('port')
    ap.add_argument('action', choices=['monitor', 'cmd', 'stress', 'break', 'rst-test'])
    ap.add_argument('args', nargs='*')
    ap.add_argument('--rst', choices=['rts', 'dtr'], default='rts')
    ap.add_argument('-v', '--verbose', action='store_true')
    a = ap.parse_args()
    ser = open_port(a.port)
    link = Link(ser, a.verbose)
    try:
        if a.action == 'monitor':
            link.sync()
            monitor(link, float(a.args[0]) if a.args else 0)
            return
        v = link.alive()
        for b in link.boots:
            print('note: MSP reset when the port was opened:', describe(('B', b)))
        if v is None:
            sys.exit('no answer from the test app')
        print('app version', v)
        if a.action == 'cmd':
            f = link.request(a.args[0], bytes.fromhex(''.join(a.args[1:])))
            print('%s  %s' % (f[0], describe(f)) if f else 'no answer')
        elif a.action == 'stress':
            sys.exit(0 if stress(link, int(a.args[0]) if a.args else 1000) else 1)
        elif a.action == 'break':
            brk(link, int(a.args[0]) if a.args else 200)
        else:
            rst_test(link, Bsl(ser, rst=a.rst))
    except KeyboardInterrupt:
        pass
    finally:
        ser.close()


if __name__ == '__main__':
    main()
