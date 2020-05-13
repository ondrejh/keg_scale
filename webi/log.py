#!/usr/bin/env python3

import requests
import json
import datetime
import time

while True:

    data = json.loads(requests.get('http://192.168.42.1/data.json').content)
    log = datetime.datetime.now().isoformat() + '; {}; {}; {}; {}'.format(data['raw'], data['units'], data['temp'],
                                                                          data['traw'])
    print(log)
    with open('log.csv', 'a') as file:
        file.write(log + '\n')

    time.sleep(10)