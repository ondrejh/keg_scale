#!/usr/bin/python3

'''
script to display, save and restore calibration

USAGE:
    calib.py --ip 192.168.42.1 .. display calibration
    calib.py --ip 192.168.42.1 --file AUTO .. save calibration
    calib.py --ip 192.168.42.1 --file <EXISTING_FILE> .. restore calibration
'''

import requests
import click
import json
import os
from time import sleep
import datetime


KEG_IP = '192.168.42.1'
#KEG_IP = '127.0.0.1'


def get_config(url):
    data_str = None

    try:
        data_str = requests.get(url, allow_redirects=True)
    except requests.exceptions.ConnectionError:
        return None

    data = json.loads(data_str.content)
    config = {}
    if 'devid' in data.keys():
        config['devid'] = data['devid']
    if 'calib' in data.keys():
        config['calib'] = data['calib']
        return config
    
    return None


def print_config(config):
    if config is not None:
        if 'devid' in config.keys():
            print('Device: {}'.format(config['devid']))
        print('Calibration table:')
        for key in config['calib'].keys():
            print("  {:5s}: {:10d}".format(key, config['calib'][key]))


def set_one_calibration_row(raw, units, ip=KEG_IP):
    url = 'http://{}/do.php'.format(ip)
    r = requests.post(url, data = {'addc': str(units), 'rawc': raw})
    if r.content == b'OK':
        return True
    return False


def delete_one_calibration_row(row, ip=KEG_IP):
    url = 'http://{}/do.php'.format(ip)
    r = requests.post(url, data = {'delc': row})
    if r.content == b'OK':
        return True
    print(row, r.content)
    return False


def key_in_keys(key, keys):
    k = float(key)
    for kk in keys:
        kkk = float(kk)
        if kkk == k:
            return True
    return False


@click.command()
@click.option('--ip', default=KEG_IP, help='Keg scale ip address')
@click.option('--file', default=None, help='File where the configuration is stored')
def config(ip, file):

    url = 'http://{}/data.json'.format(ip)
    config = get_config(url)

    if file is not None:
        if os.path.isfile(file):
            f = open(file)
            fconf = json.loads(f.read())
            f.close()
            print('File "{}"'.format(file))
            print_config(fconf)
            print()
            print('Device before setup:')
            print_config(config)
            print()
            for key in fconf['calib'].keys():
                ret = set_one_calibration_row(fconf['calib'][key], key, ip=ip)
                print('Set {} {} .. {}'.format(key, fconf['calib'][key], 'OK' if ret else 'ERROR'))
            while (True):
                config = get_config(url)
                cont = False
                for cnt in range(len(config['calib'].keys())):
                    key = list(config['calib'].keys())[cnt]
                    if not key_in_keys(key, list(fconf['calib'].keys())):
                        ret = delete_one_calibration_row(cnt + 1, ip=ip)
                        print('Del {} {} .. {}'.format(cnt+1, key, 'OK' if ret else 'ERROR'))
                        cont = True
                        break
                if not cont:
                    break

            print()
            print('Device after setup:')
            config = get_config(url)
            print_config(config)
        else:
            if file == 'AUTO':
                fname = datetime.datetime.now().strftime('{}_%Y%m%d.json'.format(config['devid']))
                file = fname
            f = open(file, 'w')
            json.dump(config, f)
            f.close()
            print('Configuration saved to file "{}"'.format(file))
    else:
        if config is not None:
            print_config(config)
        else:
            print("Can't get device configuration!")


if __name__ == "__main__":

    config()
