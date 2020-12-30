#!/usr/bin/env python3

import re


def get_var_name(fname):

    return fname.replace('-', '.').replace('/', '_').split('.')[0]


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


with open('../webi.h', 'w') as fout:

    print('Creating webi.h header file')
    fout.write('#ifndef __WEBI_H__\n#define __WEBI_H__\n\n')
    print('  Embed html file index.h')
    fout.write(import_html('index.html'))
    print('  Embed html file calib.html')
    fout.write(import_html('calib.html'))
    print('  Embed html file keg.html')
    fout.write(import_html('keg.html'))
    print('  Embed html file config.html')
    fout.write(import_html('config.html'))
    print('  Embed binary css file style.css')
    fout.write(import_bin('sass/style.css'))#, end=None))
    print('  Embed binary favicon.ico')
    fout.write(import_bin('favicon.ico'))#, end=None))
    print('  Embed binary img/barell.svg')
    fout.write(import_bin('img/barell.svg', end=None))
    print('  Embed binary img/bg.jpg')
    fout.write(import_bin('img/bg.jpg'))#, end=None))
    print('  Embed binary file jquery-3.5.1.min.js')
    fout.write(import_bin('jquery-3.5.1.min.js', end=None))
    fout.write('#endif __WEBI_H__\n')
    print('Done')
