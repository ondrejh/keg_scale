#!/usr/bin/env python3

import re
from subprocess import call


FILENAME = 'webi.h'
DESTINATION = '../sketch/keg_scale/'


def get_var_name(fname):

    return fname.replace('-', '_').replace('/', '_').split('.')[0]


def import_html(fname):

    varname = get_var_name(fname)
    sout = 'const char *{}_name PROGMEM = "/{}";\n'.format(varname, fname)
    sout += 'const char *{}_html PROGMEM = "\\\n'.format(varname)
    for line in open(fname, 'r'):
        line = line.replace('"', '\\"')
        line = line.rstrip()
        line += '\\n\\\n'
        sout += line
    sout += '";\n\n'
    return sout


def import_bin(fname, line_bytes=16, end=b'\x00'):

    varname = get_var_name(fname)
    fcont = open(fname, 'rb').read()
    if end is not None:
        fcont += end
    flen = len(fcont)
    sout = '#define {}_LEN {}\n'.format(varname.upper(), flen)
    sout += 'const char *{}_name PROGMEM = "/{}";\n'.format(varname, fname)
    sout += 'const char {}_bin[{}_LEN] PROGMEM = {}\n'.format(varname, varname.upper(), '{')
    first = True
    cnt = 0
    for b in fcont:
        if first:
            first = False
        else:
            sout += ',{}'.format('\n' if ((cnt % line_bytes) == 0) else ' ')
        sout += '0x{:02X}'.format(b)
        cnt += 1
    sout += '};\n\n'
    return sout


HEADER = FILENAME.upper().replace('.','_')


with open(FILENAME, 'w') as fout:

    print('Creating {} header file'.format(FILENAME))
    fout.write('#ifndef __{0}__\n#define __{0}__\n\n'.format(HEADER))
    print('  Embed html file index.h')
    fout.write(import_html('index.html'))
    print('  Embed html file calib.html')
    fout.write(import_html('calib.html'))
    #print('  Embed html file keg.html')
    #fout.write(import_html('keg.html'))
    print('  Embed html file config.html')
    fout.write(import_html('config.html'))
    print('  Embed binary css file style.css')
    fout.write(import_bin('sass/style.css'))#, end=None))
    print('  Embed binary favicon.ico')
    fout.write(import_bin('favicon.ico', end=None))
    print('  Embed binary img/barell.svg')
    fout.write(import_bin('img/barell.svg', end=None))
    print('  Embed binary img/bg.jpg')
    fout.write(import_bin('img/bg.jpg', end=None))

    print('  Embed html file as binary keg.html')
    fout.write(import_bin('keg.html', end=None))

    print('  Embed binary img/favicon-16x16.png')
    fout.write(import_bin('img/favicon-16x16.png', end=None))
    print('  Embed binary img/favicon-32x32.png')
    fout.write(import_bin('img/favicon-32x32.png', end=None))
    print('  Embed binary img/apple-touch-icon.png')
    fout.write(import_bin('img/apple-touch-icon.png', end=None))
    print('  Embed binary img/site.webmanifest')
    fout.write(import_bin('img/site.webmanifest'))#, end=None))
    print('  Embed binary img/android-chrome-512x512.png')
    fout.write(import_bin('img/android-chrome-512x512.png', end=None))
    print('  Embed binary img/android-chrome-192x192.png')
    fout.write(import_bin('img/android-chrome-192x192.png', end=None))
    
    #print('  Embed binary file jquery-3.5.1.min.js')
    #fout.write(import_bin('jquery-3.5.1.min.js', end=None))
    #print('  Embed binary file jquery-ui-1.12.1.min.js')
    #fout.write(import_bin('jquery-ui-1.12.1.min.js', end=None))
    fout.write('#endif __{}__\n'.format(HEADER))
    print('Done')

print()
print('Copy "webi.h" to "{}" .. '.format(DESTINATION), end='')
print('OK' if call(['cp', 'webi.h', DESTINATION]) == 0 else 'ERROR')

