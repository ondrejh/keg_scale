#!/usr/bin/env python3

""" This will generate from csv file generated with log.py html charts and open it in browser"""

import datetime
import subprocess
import os

file_in = 'log.csv'
file_out = 'log.html'

browser_exec = "C:\Program Files (x86)\Google\Chrome\Application\chrome.exe"
if not os.path.isfile(browser_exec):
    browser_exec = 'firefox'

data = {'time': [],
        'scale_raw': [],
        'temp_raw': [],
        'scale': [],
        'temp': []}

for line in open(file_in):
    splt = line.strip().split(';')
    tim = datetime.datetime.strptime(splt[0], '%Y-%m-%dT%H:%M:%S.%f')
    s_raw = int(splt[1].strip())
    s = float(splt[2].strip())
    t = float(splt[3].strip())
    t_raw = int(splt[4].strip())
    data['time'].append(tim)
    data['scale_raw'].append(s_raw)
    data['temp_raw'].append(t_raw)
    data['scale'].append(s)
    data['temp'].append(t)


def data_to_js(data, variable):

    js = '{}: ['.format(variable)
    first = True

    for item in data:
        if first:
            first = False
        else:
            js += ', '

        if type(item) == datetime.datetime:
            js += "'" + datetime.datetime.strftime(item, "%Y-%m-%d %H:%M:%S.%f") + "'"
        else:
            js += str(item)
    js += ']'
    return js


tim = data_to_js(data['time'], 'x')
scal = data_to_js(data['scale'], 'y')
temp = data_to_js(data['temp'], 'y')
sraw = data_to_js(data['scale_raw'], 'y')
traw = data_to_js(data['temp_raw'], 'y')

weight_trace = 'var weight_kg = {\n' + tim + ',\n' + scal + ",\ntype: 'scatter',\nname: 'weight [kg]'};\n"
tempetature_trace = 'var temp_c = {\n' + tim + ',\n' + temp +\
                    ",\ntype: 'scatter',\nname: 'temperature [C]',\nyaxis: 'y2'};\n"
wraw_trace = 'var weight_raw = {\n' + tim + ',\n' + sraw + ",\ntype: 'scatter',\nname: 'scale [raw]'};\n"
traw_trace = 'var temp_raw = {\n' + tim + ',\n' + traw + ",\ntype: 'scatter',\nname: 'temp [raw]',\nyaxis: 'y2'};\n"

with open(file_out, 'w') as file:
    file.write('<!DOCTYPE html><html lang="en"><head><meta charset="utf-8"><title>Log chart</title>')
    file.write('<script src="https://cdn.plot.ly/plotly-latest.min.js"></script></head>\n')
    file.write('<body>\n<div id="chart"></div>\n')
    file.write('<body>\n<div id="chart2"></div>\n')
    file.write('<script>\n')
    file.write(weight_trace)
    file.write(tempetature_trace)
    file.write('var data = [weight_kg, temp_c];\n')
    file.write("var layout = {\nyaxis: {title: 'weight [kg]'},\nyaxis2: {\ntitle: 'temperature C',\n")
    file.write("overlaying: 'y',\nside: 'right'},\nlegend: {orientation: 'h', x: 0.5}};\n")
    file.write("Plotly.newPlot('chart', data, layout);\n")
    file.write(wraw_trace)
    file.write(traw_trace)
    file.write('var data_raw = [weight_raw, temp_raw];\n')
    file.write("var layout_raw = {\nyaxis: {title: 'scale [raw]'},\nyaxis2: {\ntitle: 'temp [raw]',\n")
    file.write("overlaying: 'y',\nside: 'right'},\nshowlegent: true, legend: {x: 0.5, orientation: 'h'}};\n")
    file.write("Plotly.newPlot('chart2', data_raw, layout_raw);\n")
    file.write('</script></body></html>')

subprocess.call([browser_exec, file_out])
