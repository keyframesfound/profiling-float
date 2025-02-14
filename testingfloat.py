#!/usr/bin/python
# -*- coding: UTF-8 -*-
# cython:language_level=2 

from __future__ import unicode_literals
import sys
import importlib
try:
    importlib.reload(sys) #importlib.reload for python3
except:
    reload(sys) #reload for python2
    sys.setdefaultencoding('utf-8')
import os
import inspect
import time
import datetime
import requests
from urllib.parse import unquote
import ast
import json
import serial
import string
import uuid
import subprocess
import multiprocessing

import RPi.GPIO as GPIO
from gpiozero import Button

# flask api server lib
from flask import Flask, request, jsonify
from flask_restful import Resource, Api
from flask_cors import CORS
from flaskext.mysql import MySQL

app = Flask(__name__)
api = Api(app)
# api = CORS(app)

# MySQL configurations - localDB
mysql_l = MySQL()
mysql_l.init_app(app)
app.config['MYSQL_DATABASE_USER'] = "root"
app.config['MYSQL_DATABASE_PASSWORD'] = "ssc"
app.config['MYSQL_DATABASE_DB'] = "rov"
app.config['MYSQL_DATABASE_HOST'] = "localhost"
myuuid = uuid.uuid4()
print("UUID: " + str(myuuid))


########################################################
# Set Current Time
########################################################
class SetCurrentDTime(Resource):
    def post(self):
        curr_dt = request.args.get('dt')
        decoded_string = unquote(curr_dt)
        print(str(decoded_string))
        settime = subprocess.call(["sudo", "date", "-s", curr_dt])
        time.sleep(1)
        return {'message': 'Request for SetCurrentDTime received!!'}

########################################################
# MySQL DB Function
########################################################
def DbInsert(table, field_lstr, value_lstr):
    db = mysql_l.connect()
    cur = db.cursor()
    sql_str = "INSERT IGNORE INTO `" + table + "` ( " + field_lstr + " ) VALUES ( " + value_lstr + ");"
    print(sql_str)
    try:
        cur.execute(sql_str)
        db.commit()
        r = cur.fetchall()
    except Exception:
        return 'Error: DbInsert()'
    finally:
        cur.close()
        db.close()

########################################################
# Buoyancy Stepper Control API Function
########################################################
step_no = "10000"
GdButton = Button(17)
ButtonCkCnt = 0
# up = "U"+"1000" # rotation clockwise for upward direction
# down = "D" + "1000" # rotation anti-clockwise for downward direction


def FloatMovement(direction, steps):
    ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=0.5)
#     ser.write("".encode()) #if USB Connected to Arduino, 1st cmd Ignored by Arduino. So send Empty Cmd to workaround
#     time.sleep(2)
    ser.write((direction + steps).encode())
    time.sleep(1)
    rnt = ser.readall()
    time.sleep(1)
    ser.close()

class FloatSpeed(Resource):
    def get(self):
        set_speed = request.args.get('speed')
#         print(str(set_speed))
        decoded_speed = unquote(set_speed)
#         print(str(decoded_speed))
        time.sleep(0.5)
        ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=0.5)
#         print(type(decoded_speed))
        
        ser.write(("d" + str(decoded_speed)).encode())
        time.sleep(1)
        rnt = ser.readall()
        print(str(rnt))
        time.sleep(1)
        ser.close()
        return {'message': 'Float Speed Set'}

class FloatUpDown(Resource):
    def get(self):
        myuuid = uuid.uuid4()
#         x = requests.get('/p/sp')
#         print(x.status_code)
        time.sleep(1)
        FloatMovement("U", step_no)
        time.sleep(10)
        FloatMovement("D", step_no)
        time.sleep(10)
#         x = requests.get('/p/ep')
#         print(x.status_code)
        time.sleep(1)
        return {'message': 'Float UpDown'}

class FloatUpDownButtonControl(Resource):
    def get(self):
        FloatMovement("U", step_no)
#         time.sleep(120)
        GdButton.wait_for_press(timeout=5.0)
        FloatMovement("D", step_no)
        time.sleep(10)
        return {'message': 'Float UpDown Button Control'}

class FloatDown(Resource):
    def get(self):
        FloatMovement("D", step_no)
        return {'message': 'Float Down'}

class FloatUp(Resource):
    def get(self):
        FloatMovement("U", step_no)
        return {'message': 'Float Up'}

########################################################
# Report Line Chart3js Function
########################################################
class ReportLineChart(Resource):
    def get(self):
        db = mysql_l.connect()
        cur = db.cursor()
        sql_str = "SELECT * FROM `rov_float`"
        try:
            cur.execute(sql_str)
            db.commit()
            r = cur.fetchall()
            data = []
            for row in r:
                data.append({
                    'label': row[0],
                    'value': row[1]
                })

            return jsonify(data)
        except Exception as e:
            return jsonify({'error': str(e)})
        finally:
            cur.close()
            db.close()
            
            
class LineChartHtml(Resource):
    def get(self):
        return '''
        <!DOCTYPE html>
        <html>
        <head>
            <title>Chart.js Example</title>
            <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
        </head>
        <body>
            <canvas id="chart"></canvas>
            <script>
                fetch('/chartdata')
                    .then(response => response.json())
                    .then(data => {
                        const labels = data.map(item => item.label);
                        const values = data.map(item => item.value);

                        const ctx = document.getElementById('chart').getContext('2d');
                        new Chart(ctx, {
                            type: 'bar',
                            data: {
                                labels: labels,
                                datasets: [{
                                    label: 'Data',
                                    data: values,
                                    backgroundColor: 'rgba(75, 192, 192, 0.2)',
                                    borderColor: 'rgba(75, 192, 192, 1)',
                                    borderWidth: 1
                                }]
                            },
                            options: {
                                scales: {
                                    y: {
                                        beginAtZero: true
                                    }
                                }
                            }
                        });
                    });
            </script>
        </body>
        </html>
        '''

#ROUTES
api.add_resource(StartUltraDisCollection, '/u/su') # Route
api.add_resource(StopUltraDisCollection, '/u/eu') # Route 
api.add_resource(ReportUltraDisCollection, '/u') # Route
api.add_resource(FloatDown, '/fd') # Route
api.add_resource(FloatUp, '/fu') # Route
api.add_resource(FloatUpDown, '/fud') # Route
api.add_resource(FloatUpDownButtonControl, '/fudbc') # Route
api.add_resource(ReportLineChart, '/chartdata') # Route
api.add_resource(LineChartHtml, '/chart') # Route
api.add_resource(SetCurrentDTime, '/sdt') # Route
api.add_resource(FloatSpeed, '/sspeed') # Route

if __name__ == '__main__':
    # Development Mode
    app.run(host='0.0.0.0', port=7123, debug=True)
