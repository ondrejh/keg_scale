#!/usr/bin/env python3
"""
Kegator web UI dev server (Python 3 stdlib only).

Serves the web20 folder and provides the device API:
  GET  /data.json    scale data
  GET  /conf.json    configuration (no passwords)
  GET  /keglist.txt  known keg names
  POST /do.php       actions (form fields as in sketch/keg_scale/http_handlers.ino)

Modes:
  mock (default)      API emulated in memory, seeded from data.json / conf.json /
                      keglist.txt. Nothing is written back; restart = reset.
  --device HOST       API forwarded to a real scale, static files still local.

Usage:
  python3 dev_server.py [--port 8000] [--bind 0.0.0.0] [--keg] [--device 192.168.42.1]
"""

import argparse
import copy
import json
import random
import urllib.error
import urllib.request
from email.parser import BytesParser
from email.policy import default as email_policy
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qsl

ROOT = Path(__file__).resolve().parent
API_GET = ('/data.json', '/conf.json', '/keglist.txt')
API_POST = ('/do.php',)
DEV_ID = 'kegDEV000'
CALIB_MIN_ROWS = 2   # firmware refuses to delete below this


# ---------------------------------------------------------------- mock state

def load_json(name):
    try:
        return json.loads((ROOT / name).read_text(encoding='utf-8'))
    except (OSError, ValueError):
        return {}


class MockDevice:

    def __init__(self, with_keg):
        self.data = load_json('data_keg.json' if with_keg else 'data.json')
        self.data.setdefault('sw', 'dev_server (mock)')
        self.conf = load_json('conf.json')
        self.conf.setdefault('disp_timeout', 15)    # fw20 settings.h defaults
        self.conf.setdefault('disp_dim', 25)
        self.secrets = {}   # passwords: stored, never sent back
        try:
            text = (ROOT / 'keglist.txt').read_text(encoding='utf-8')
            self.keglist = [k for k in text.splitlines() if k.strip()]
        except OSError:
            self.keglist = []

    # -- calibration helpers (units <-> raw, linear interpolation)

    def calib_rows(self):
        """Sorted list of (units, raw)."""
        rows = [(float(u), int(r)) for u, r in self.data.get('calib', {}).items()]
        return sorted(rows)

    def set_calib_rows(self, rows):
        self.data['calib'] = {'%.2f' % u: r for u, r in sorted(rows)}

    def units_to_raw(self, units):
        rows = self.calib_rows()
        if not rows:
            return 0
        if len(rows) == 1:
            return rows[0][1]
        i = 1
        while i < len(rows) - 1 and units >= rows[i][0]:
            i += 1
        (u0, r0), (u1, r1) = rows[i - 1], rows[i]
        return int(r0 + (r1 - r0) * (units - u0) / (u1 - u0))

    # -- GET

    def get(self, path):
        if path == '/data.json':
            d = copy.deepcopy(self.data)
            if 'raw' in d:
                d['raw'] += random.randint(-300, 300)   # look alive
            return 'application/json', json.dumps(d)
        if path == '/conf.json':
            return 'application/json', json.dumps(self.conf)
        return 'text/plain; charset=utf-8', '\n'.join(self.keglist) + '\n'

    # -- POST /do.php

    def do(self, f):
        if 'keg' in f and 'vol' in f:
            try:
                vol = float(f['vol'])
            except ValueError:
                return False
            if vol <= 0:
                return False
            units = vol * 0.54 + 5.5           # simulated weight of a full keg
            raw = self.units_to_raw(units)
            self.data['units'] = '%.1f' % units
            self.data['raw'] = raw
            self.data['keg'] = {'name': f['keg'], 'fullraw': raw,
                                'volume': f['vol'], 'left': f['vol']}
            if f['keg'] and f['keg'] not in self.keglist:
                self.keglist.append(f['keg'])
            return True

        if f.get('del') == 'yes':
            self.data.pop('keg', None)
            return True

        if 'addc' in f:
            try:
                units = float(f['addc'])
                raw = int(f['rawc']) if 'rawc' in f else int(self.data.get('raw', 0))
            except ValueError:
                return False
            rows = [r for r in self.calib_rows() if r[0] != units]
            rows.append((units, raw))
            self.set_calib_rows(rows)
            return True

        if 'delc' in f:
            try:
                idx = int(f['delc'])
            except ValueError:
                return False
            rows = self.calib_rows()
            if not (0 < idx <= len(rows)) or len(rows) <= CALIB_MIN_ROWS:
                return False
            del rows[idx - 1]
            self.set_calib_rows(rows)
            return True

        if 'wssid' in f:
            if f['wssid']:
                self.conf['ssid'] = f['wssid']
            else:
                self.conf.pop('ssid', None)
            self.secrets['wpwd'] = f.get('wpwd', '')
            return True

        if 'disp_timeout' in f or 'disp_dim' in f:
            limits = {'disp_timeout': 3600, 'disp_dim': 100}
            new = {}
            for key, top in limits.items():
                if key in f:
                    try:
                        v = int(f[key])
                    except ValueError:
                        return False
                    if not 0 <= v <= top:
                        return False
                    new[key] = v
            self.conf.update(new)
            return True

        if any(k in f for k in ('mqtt_url', 'mqtt_topic', 'mqtt_user')):
            for key in ('mqtt_topic', 'mqtt_url', 'mqtt_user'):
                if key in f:
                    if f[key]:
                        self.conf[key] = f[key]
                    else:
                        self.conf.pop(key, None)
            if f.get('mqtt_url') and not self.conf.get('mqtt_topic'):
                self.conf['mqtt_topic'] = 'home/keg/' + DEV_ID
            if 'mqtt_user' in f:
                self.secrets['mqtt_pass'] = f.get('mqtt_pass', '')
            return True

        return False


# ---------------------------------------------------------------- HTTP

def parse_form(content_type, body):
    """Parse multipart/form-data (fetch + FormData) or urlencoded body."""
    if content_type.startswith('multipart/form-data'):
        msg = BytesParser(policy=email_policy).parsebytes(
            b'Content-Type: ' + content_type.encode() + b'\r\n\r\n' + body)
        fields = {}
        for part in msg.iter_parts():
            name = part.get_param('name', header='content-disposition')
            if name:
                fields[name] = part.get_payload(decode=True).decode('utf-8')
        return fields
    return dict(parse_qsl(body.decode(), keep_blank_values=True))


class Handler(SimpleHTTPRequestHandler):

    mock = None      # MockDevice
    device = None    # proxy target host

    def end_headers(self):
        self.send_header('Cache-Control', 'no-store')
        super().end_headers()

    def reply(self, code, ctype, text):
        body = text.encode('utf-8') if isinstance(text, str) else text
        self.send_response(code)
        self.send_header('Content-Type', ctype)
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def proxy(self, body=None):
        url = 'http://%s%s' % (self.device, self.path)
        req = urllib.request.Request(url, data=body, method=self.command)
        if body is not None:
            req.add_header('Content-Type', self.headers.get('Content-Type', ''))
        try:
            with urllib.request.urlopen(req, timeout=3) as r:
                self.reply(r.status, r.headers.get('Content-Type', 'text/plain'), r.read())
        except urllib.error.HTTPError as e:
            self.reply(e.code, 'text/plain', e.read())
        except (urllib.error.URLError, OSError) as e:
            self.log_message('proxy %s failed: %s', url, e)
            self.reply(502, 'text/plain', 'device unreachable: %s\n' % e)

    def route(self):
        return self.path.split('?', 1)[0]

    def do_GET(self):
        path = self.route()
        if path == '/':
            self.send_response(302)
            self.send_header('Location', '/index.html')
            self.end_headers()
        elif path in API_GET:
            if self.device:
                self.proxy()
            else:
                self.reply(200, *self.mock.get(path))
        else:
            super().do_GET()

    def do_POST(self):
        path = self.route()
        if path not in API_POST:
            self.reply(404, 'text/plain', 'not found\n')
            return
        body = self.rfile.read(int(self.headers.get('Content-Length') or 0))
        if self.device:
            self.proxy(body)
            return
        fields = parse_form(self.headers.get('Content-Type', ''), body)
        ok = self.mock.do(fields)
        self.log_message('do.php %s -> %s', fields, 'OK' if ok else 'ERROR')
        self.reply(200, 'text/html', 'OK' if ok else 'ERROR')


def main():
    ap = argparse.ArgumentParser(description='Kegator web UI dev server')
    ap.add_argument('--port', type=int, default=8000)
    ap.add_argument('--bind', default='0.0.0.0')
    ap.add_argument('--keg', action='store_true', help='mock: start with a tapped keg (data_keg.json)')
    ap.add_argument('--device', metavar='HOST', help='proxy API to a real scale, e.g. 192.168.42.1')
    args = ap.parse_args()

    Handler.device = args.device
    if not args.device:
        Handler.mock = MockDevice(args.keg)

    srv = ThreadingHTTPServer((args.bind, args.port), partial(Handler, directory=str(ROOT)))
    mode = 'proxy -> http://%s' % args.device if args.device else 'mock' + (' (keg)' if args.keg else '')
    print('Kegator dev server [%s] on http://localhost:%d/' % (mode, args.port))
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        print()


if __name__ == '__main__':
    main()
