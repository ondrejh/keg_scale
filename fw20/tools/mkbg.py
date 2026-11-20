#!/usr/bin/env python3
"""Cut display gauge textures out of web20/img/bg.jpg.

Writes raw little-endian LVGL pixel data (LVGL swaps bytes in flush):
  foam.rgb565    foam band, made seamless for vertical tiling (crossfaded wrap)
  beer.rgb565    beer band, made seamless for vertical tiling (crossfaded wrap)
  edge.rgb565a8  foam/beer transition strip with faded top/bottom (RGB565 plane
                 followed by A8 plane), drawn over the level boundary

Run with system python3 (needs Pillow + numpy) after bg.jpg changes:
  python3 tools/mkbg.py [--preview DIR]
"""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
SRC = HERE / '../../web20/img/bg.jpg'
DST = HERE / '../main/img'

SIZE = 466                  # display width/height
X0 = (650 - SIZE) // 2      # centre crop, native scale

# source row ranges [start, end) - see row colour profile of bg.jpg
FOAM = (40, 160)
EDGE = (130, 210)   # foam/beer line is around row 180
BEER = (188, 280)
XFADE = 30          # rows crossfaded to make foam/beer tiles seamless
EDGE_FADE = (30, 20)  # edge strip alpha ramp rows (top, bottom)


def band(im, rows):
    return im.crop((X0, rows[0], X0 + SIZE, rows[1]))


def flatten(b):
    """Remove vertical brightness/tint gradient: scale each row to the band mean."""
    a = np.asarray(b, dtype=np.float32)
    row = a.mean(axis=1, keepdims=True)                 # per row, per channel
    a = a * (a.mean(axis=(0, 1), keepdims=True) / row)
    return Image.fromarray(np.clip(a, 0, 255).astype(np.uint8), 'RGB')


def seamless(b, k=XFADE):
    """Period n-k tile: first k rows blend into the band's last k rows."""
    n = b.height
    t = b.crop((0, 0, SIZE, n - k))
    for i in range(k):
        a = (i + 0.5) / k
        row = Image.blend(b.crop((0, n - k + i, SIZE, n - k + i + 1)),
                          b.crop((0, i, SIZE, i + 1)), a)
        t.paste(row, (0, i))
    return t


def edge_alpha(h, top, bottom):
    m = Image.new('L', (SIZE, h), 255)
    d = ImageDraw.Draw(m)
    for i in range(top):
        d.line((0, i, SIZE, i), fill=int(255 * (i + 0.5) / top))
    for i in range(bottom):
        d.line((0, h - 1 - i, SIZE, h - 1 - i), fill=int(255 * (i + 0.5) / bottom))
    return m


def rgb565(im):
    out = bytearray()
    for r, g, b in im.getdata():
        v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
        out += v.to_bytes(2, 'little')
    return bytes(out)


def tile_v(tile, x, y, h, canvas):
    """Tile vertically into canvas region starting at y (tile origin at y)."""
    yy = y
    while yy < y + h:
        part = tile.crop((0, 0, SIZE, min(tile.height, y + h - yy)))
        canvas.paste(part, (x, yy))
        yy += tile.height


def gauge(foam, beer, edge, alpha, pct):
    """Same placement as ui_gauge_set_level()."""
    img = Image.new('RGB', (SIZE, SIZE))
    tile_v(foam, 0, 0, SIZE, img)
    e = edge.height
    y = e // 2 + SIZE * (100 - pct) // 100
    if y < SIZE:
        tile_v(beer, 0, y, SIZE - y, img)
    img.paste(edge, (0, y - e // 2), alpha)
    img = img.crop((0, 0, SIZE, SIZE))
    mask = Image.new('L', (SIZE, SIZE), 0)
    ImageDraw.Draw(mask).ellipse((0, 0, SIZE - 1, SIZE - 1), fill=255)
    out = Image.new('RGB', (SIZE, SIZE))
    out.paste(img, mask=mask)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', type=Path, default=SRC)
    ap.add_argument('--dst', type=Path, default=DST)
    ap.add_argument('--preview', type=Path, help='write gauge_XXX.png previews here')
    a = ap.parse_args()

    im = Image.open(a.src).convert('RGB')
    foam = seamless(flatten(band(im, FOAM)))
    beer = seamless(flatten(band(im, BEER)))
    edge = band(im, EDGE)
    alpha = edge_alpha(edge.height, *EDGE_FADE)

    a.dst.mkdir(parents=True, exist_ok=True)
    out = {
        'foam.rgb565': (foam, rgb565(foam)),
        'beer.rgb565': (beer, rgb565(beer)),
        'edge.rgb565a8': (edge, rgb565(edge) + alpha.tobytes()),
    }
    for name, (t, data) in out.items():
        p = a.dst / name
        p.write_bytes(data)
        print('%-14s %dx%d  %d bytes' % (name, t.width, t.height, len(data)))

    if a.preview:
        a.preview.mkdir(parents=True, exist_ok=True)
        for pct in (0, 25, 50, 75, 100):
            p = a.preview / ('gauge_%03d.png' % pct)
            gauge(foam, beer, edge, alpha, pct).save(p)
            print('preview', p)


if __name__ == '__main__':
    main()
