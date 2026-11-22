#!/usr/bin/env python3
"""Decode keg20 sensor bridge frames.

    decode.py /dev/ttyUSB0      read the serial port (9600 8E1, needs pyserial)
    decode.py -                 read lines from stdin
    decode.py --selftest        check the CRC implementation

Frame: <type><hex payload>*<crc8 hex>\\n, Dallas/Maxim CRC-8 over the bytes
before '*'.
"""
import re
import sys

FRAME = re.compile(r'^([VHhTt])([0-9A-F]*)\*([0-9A-F]{2})$')

HX_ERR = {1: 'not ready (missing)', 2: 'DOUT low after 25th pulse (unpowered)'}
DS_ERR = {1: 'bus stuck low', 2: 'no presence', 3: 'scratchpad CRC'}
PAYLOAD_LEN = {'V': 2, 'H': 6, 'h': 2, 'T': 4, 't': 2}


def crc8(data, crc=0):
    for b in data:
        for _ in range(8):
            mix = (crc ^ b) & 1
            crc >>= 1
            if mix:
                crc ^= 0x8C
            b >>= 1
    return crc


def make_frame(typ, payload):
    body = typ + payload
    return '%s*%02X' % (body, crc8(body.encode()))


def decode(line):
    """Return a description string, or raise ValueError."""
    m = FRAME.match(line.strip())
    if not m:
        raise ValueError('bad format')
    typ, payload, crc = m.groups()
    if crc8((typ + payload).encode()) != int(crc, 16):
        raise ValueError('CRC mismatch')
    if len(payload) != PAYLOAD_LEN[typ]:
        raise ValueError('bad length')
    v = int(payload, 16)
    if typ == 'V':
        return 'version %d' % v
    if typ == 'H':
        if v & 0x800000:
            v -= 1 << 24
        return 'hx711 %d' % v
    if typ == 'h':
        return 'hx711 error %02X %s' % (v, HX_ERR.get(v, '?'))
    if typ == 'T':
        if v & 0x8000:
            v -= 1 << 16
        return 'temp %.4f C (raw %d)' % (v / 16, v)
    return 'temp error %02X %s' % (v, DS_ERR.get(v, '?'))


def selftest():
    # DS18B20 datasheet ROM example: CRC over all 8 bytes incl. CRC byte = 0
    assert crc8(bytes([0x02, 0x1C, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xA2])) == 0
    # DS18B20 power-on scratchpad (85 C): 50 05 4B 46 7F FF 0C 10 1C
    assert crc8(bytes([0x50, 0x05, 0x4B, 0x46, 0x7F, 0xFF, 0x0C, 0x10, 0x1C])) == 0
    f = make_frame('T', '0550')
    assert decode(f) == 'temp 85.0000 C (raw 1360)', decode(f)
    assert decode(make_frame('H', 'FFFFFE')) == 'hx711 -2'
    assert decode(make_frame('t', '02')).startswith('temp error 02')
    bad = f[:-1] + ('0' if f[-1] != '0' else '1')
    try:
        decode(bad)
        raise AssertionError('corrupted CRC accepted')
    except ValueError:
        pass
    print('selftest ok, e.g.', f, make_frame('H', '00A1B2'), make_frame('V', '01'))


def lines(src):
    if src == '-':
        for line in sys.stdin:
            yield line
        return
    import serial
    with serial.Serial(src, 9600, parity=serial.PARITY_EVEN, timeout=None) as s:
        while True:
            yield s.readline().decode('ascii', 'replace')


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    if sys.argv[1] == '--selftest':
        selftest()
        return
    for line in lines(sys.argv[1]):
        line = line.strip()
        if not line:
            continue
        try:
            print('%-14s %s' % (line, decode(line)))
        except ValueError as e:
            print('%-14s !! %s' % (line, e))
        sys.stdout.flush()


if __name__ == '__main__':
    main()
