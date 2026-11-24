#!/usr/bin/env python3
"""UART side of the BSL entry bench test on the FR2311 LaunchPad (see README.md).
Most of it is ../test_g2553/host.py.

    host_fr.py PORT monitor [SECONDS]
    host_fr.py PORT cmd V | S | P 0055AAFF | L 02 0A
    host_fr.py PORT stress [N]      pings (one wire: between the heartbeats; "stress N any": any time)
    host_fr.py PORT break [MS]      TX (= TEST, RX) low for MS, MSP must survive
    host_fr.py PORT rst-test        RST pulse with TEST high, then with TEST low
    host_fr.py PORT wdt-test [N]    watchdog reset with TEST high: does the app come back?
    host_fr.py PORT power-test      power-up with TEST high (asks for the 3V3 jumper)
    host_fr.py --selftest

RST is on RTS (active low) unless --rst dtr. --onewire: TX and RX on one line.
"""
import argparse
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'test_g2553'))
import host as base                                     # noqa: E402  (also adds ../fw/tools)
from bsl import Bsl, open_port                          # noqa: E402
from decode import make_frame                           # noqa: E402

CAUSES = {0x00: 'none', 0x02: 'brownout', 0x04: 'RST pin', 0x06: 'software BOR',
          0x08: 'LPMx.5 wakeup', 0x0A: 'security violation', 0x0E: 'SVSH (supply)',
          0x14: 'software POR', 0x16: 'WATCHDOG', 0x18: 'watchdog password',
          0x1A: 'FRCTL password', 0x1C: 'FRAM bit error', 0x1E: 'peripheral fetch',
          0x20: 'PMM password', 0x24: 'FLL unlock'}
WATCHDOG = 0x16


def cause(c):
    return CAUSES.get(c, '%02Xh' % c)


def describe(f):
    t, d = f
    if t == 'B' and len(d) == 3:
        return 'boot: version %d, cause %s%s' % (d[0], cause(d[1]),
                                                 ' + ' + cause(d[2]) if d[2] else '')
    if t == 's' and len(d) == 14:
        w = [int.from_bytes(d[i:i + 2], 'big') for i in (0, 3, 5, 7, 9, 12)]
        return ('uptime %d s, cause %s, rx ok %d, parity %d, framing/break %d, bad frames %d, '
                'overrun %d, own echo wrong %d'
                % (w[0], cause(d[2]), w[1], w[2], w[3], w[4], d[11], w[5]))
    return base_describe(f)


base_describe = base.describe
base.describe = describe            # used by base.stress(), base.monitor(), base.brk()


class Link(base.Link):
    def sync(self):
        pass                        # no autobaud here


def wait_boot(link, seconds):
    """Only listens. Returns the first boot frame or None."""
    end = time.time() + seconds
    while time.time() < end and not link.boots:
        link.poll(0.1)
    return link.boots[0] if link.boots else None


def wdt_test(link, bsl, n):
    back = 0
    for i in range(n):
        link.boots.clear()
        f = link.request('W')
        if f != ('w', b''):
            print('%d: no answer to W: %s' % (i + 1, f))
            return False
        b = wait_boot(link, 4)
        if b and b[1] == WATCHDOG:
            back += 1
            print('%d: %s' % (i + 1, describe(('B', b))))
        else:
            print('%d: %s -> reset with TEST low' % (
                i + 1, describe(('B', b)) if b else 'STOPPED: no boot frame for 4 s'))
            link.boots.clear()
            bsl.reset()
            if wait_boot(link, 2) is None:
                print('   no boot after the reset either')
                return False
    print('came back from a watchdog reset with TEST high: %d of %d' % (back, n))
    return back == n


def power_test(link, bsl, ask=input):
    """Which is the least help the chip needs after power-up with TEST high?"""
    bsl.test(False)                 # lines low: the chip isn't fed through them
    bsl.rst(False)
    ask('>>> REMOVE the 3V3 jumper on J101 (chip off), wait 3 s, then Enter ')
    bsl.release_test()              # line and RST high, as on the sensor board
    bsl.rst(True)
    time.sleep(0.2)
    link.ser.reset_input_buffer()
    link.buf = b''
    link.boots.clear()
    print('>>> PUT the 3V3 jumper back within 20 s (only listening) ...', flush=True)
    b = wait_boot(link, 20)
    if b:
        print('app started by itself: %s' % describe(('B', b)))
        return 'alone'
    if link.alive():
        print('app answers but sent no boot frame')
        return 'answers'

    def low_pulse():
        bsl.test(False)
        time.sleep(0.2)
        bsl.release_test()

    def rst_pulse():
        bsl.rst(False)
        time.sleep(0.02)
        bsl.rst(True)
    for name, act in (('line low 200 ms', low_pulse), ('RST pulse, TEST high', rst_pulse),
                      ('RST pulse, TEST low', bsl.reset)):
        act()
        b = wait_boot(link, 2)
        print('%s: %s' % (name, describe(('B', b)) if b else 'no start'))
        if b:
            return name
    return None


def selftest():
    for t, n in (('B', 3), ('T', 2), ('V', 1), ('s', 14), ('p', 4), ('w', 0)):
        describe((t, bytes(n)))
    assert 'WATCHDOG' in describe(('B', b'\x01\x16\x00'))
    old, time.sleep = time.sleep, lambda s: None
    clock, real = [0.0], time.time

    class Chip:
        """Starts after ('wdt' | 'reset'): by its watchdog, or only by RST with TEST low."""
        timeout = in_waiting = 0
        rts = dtr = break_condition = False

        def __init__(self, wdt):
            self.wdt, self.out, self.hung = wdt, b'', False

        def __setattr__(self, k, v):
            object.__setattr__(self, k, v)
            if k == 'rts' and not v and self.break_condition:       # RST rises, TEST low
                self.boot(0x04)

        def boot(self, c):
            self.hung = False
            self.out += (make_frame('B', '01%02X00' % c) + '\n').encode()

        def reset_input_buffer(self):
            self.out = b''

        def read(self, n):
            clock[0] += 0.05
            d, self.out = self.out, b''
            if not d and self.hung and self.wdt:
                self.boot(WATCHDOG)
            return d

        def write(self, data):
            f = base.parse(data.decode('latin1'))
            if f and f[0] == 'W' and not self.hung:
                self.out += (make_frame('w', '') + '\n').encode()
                self.hung = True
    time.time = lambda: clock[0]
    try:
        c = Chip(True)
        assert wdt_test(Link(c), Bsl(c), 3)
        c = Chip(False)
        assert not wdt_test(Link(c), Bsl(c), 2)
        c = Chip(False)
        object.__setattr__(c, 'hung', True)
        assert power_test(Link(c), Bsl(c), lambda p: None) == 'RST pulse, TEST low'
    finally:
        time.sleep, time.time = old, real
    print('selftest ok')


def main():
    if '--selftest' in sys.argv[1:]:
        selftest()
        return
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('port')
    ap.add_argument('action', choices=['monitor', 'cmd', 'stress', 'break', 'rst-test',
                                       'wdt-test', 'power-test'])
    ap.add_argument('args', nargs='*')
    ap.add_argument('--rst', choices=['rts', 'dtr'], default='rts')
    ap.add_argument('--onewire', action='store_true')
    ap.add_argument('-v', '--verbose', action='store_true')
    a = ap.parse_args()
    ser = open_port(a.port)
    link = Link(ser, a.verbose, a.onewire)
    bsl = Bsl(ser, rst=a.rst)
    try:
        if a.action == 'monitor':
            base.monitor(link, float(a.args[0]) if a.args else 0)
            return
        if a.action == 'power-test':
            power_test(link, bsl)
            return
        v = link.alive()
        for b in link.boots:
            print('note: MSP reset when the port was opened:', describe(('B', b)))
        if v is None:
            sys.exit('no answer from the test app')
        print('app version', v)
        if a.action == 'cmd':
            if not a.args:
                ap.error('cmd needs a command')
            f = link.request(a.args[0], bytes.fromhex(''.join(a.args[1:])))
            print('%s  %s' % (f[0], describe(f)) if f else 'no answer')
        elif a.action == 'stress':
            sys.exit(0 if base.stress(link, int(a.args[0]) if a.args else 1000,
                                      'any' not in a.args) else 1)
        elif a.action == 'break':
            base.brk(link, int(a.args[0]) if a.args else 200)
        elif a.action == 'rst-test':
            base.rst_test(link, bsl)
        else:
            sys.exit(0 if wdt_test(link, bsl, int(a.args[0]) if a.args else 5) else 1)
    except KeyboardInterrupt:
        pass
    finally:
        ser.close()


if __name__ == '__main__':
    main()
