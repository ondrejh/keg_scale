#!/usr/bin/python3

""" fetch_data.py 

Program cyklicky čte data z váhy na dané IP adrese a ukládá je do souboru.

Vstupní argumenty:

	--ip <ip_adresa_vahy> [default "192.168.42.1"]
	--file <soubor> [default "YYYYMMDD_HHMM_name.csv"]

"""


import click
import urllib.request
import time
import json
from datetime import datetime


def write_line(fid, tim, raw, kg, temp):
    fid.write('{}; {}; {}; {}\n'.format(tim, raw, kg, temp))


@click.command()
@click.option('--ip', default='192.168.42.1', help='The scale IP adress.')
@click.option('--filename', default=None, help='Data file name.')
@click.option('--interval', default=5.0, help='Data fetch interval [s].')
@click.option('--timeout', default=30.0, help='The timeout for disconnection and program exit when the scale is not available [s].')
def fetch_data(ip, filename, interval, timeout):
    """The program cyclicaly fetches the data from Kegator(R) keg scale and saves it to the SQLite database file for further data processing (e.g. the stability test)."""

    #print(ip)
    #print(database)
    #print(interval)
    
    pause = 5.0
    try:
        pause = float(interval)
    except:
        print("Can't convert interval argument '{}' to float. Set default '{}'".format(interval, pause))
    tout = 30.0
    try:
        tout = float(timeout)
    except:
        print("Can't convert timeout argument '{}' to float. Set default '{}'".format(timeout, tout))

    data_url = 'http://{}/data.json'.format(ip)
    tlast = time.time()

    first = True

    while True:
        if True:
        #try:
            content = urllib.request.urlopen(data_url).read()
            data = json.loads(content)
            devid = data['devid']
            kg = data['units']
            raw = data['raw']
            temp = data['temp']
            if first:
                first = False
                if filename is None:
                    filename = '{}_{}.csv'.format(devid, datetime.now().strftime('%Y%m%d_%H%M'))
                    print(filename)
                with open(filename, 'w') as fid:
                    fid.write('tstamp; raw; kg; temp\n')
                    write_line(fid, datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'), raw, kg, temp)
            else:
                with open(filename, 'a') as fid:
                    write_line(fid, datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'), raw, kg, temp)

            print('{}: weigth {}kg, raw {}, temp {}°C'.format(devid, kg, raw, temp))
            tlast = time.time()
        #except:
            tdif = time.time() - tlast
            if tdif > timeout:
                print('The connection timed out. Bye bye!')
                break
        time.sleep(pause)


if __name__ == '__main__':
    fetch_data()
