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

import ms5837
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
# Pressure Sensor API Function
########################################################
sensor = ms5837.MS5837_30BA() # Default I2C bus is 1 (Raspberry Pi 3)
# We must initialize the sensor before reading it
if not sensor.init():
    print("Sensor could not be initialized")
    exit(1)
# We have to read values from sensor to update pressure and temperature
if not sensor.read():
    print("Sensor read failed!")
    exit(1)

pdata_list = []  # An empty list
def AddCompareList(new_e, reach_botlv_val): # new record, Reach Bot Lv Value
    cnt = 0
    max_rec = 5
    if len(pdata_list) < MaxRec: # Limit to keep 5 records for comparison
        pdata_list.append(new_e)
    else:
        for exist_e in pdata_list[-MaxRec:]:
            if abs(int(new_e) - int(exist_e)) < int(reach_botlv_val):
                cnt += 1
                
        if cnt == max_rec:
            print("MATE Float reach the bottom and ready to float up")
            FloatMovement(float_up)
        else:
            pdata_list.pop(0)
            pdata_list.append(new_e)

def PressCollection(p_event):
    while not p_event.is_set():
        print("Pressure-Depth Data Collection Activated!")
        if sensor.read():
        # print(("P: %0.1f mbar\tT: %0.2f C\tD: %0.2f m\tAltitude: %0.2f m") % (
            # sensor.pressure(), # Default is mbar (no arguments)
            # sensor.temperature(), # Default is degrees C (no arguments)
            # sensor.depth(),
            # sensor.altitude()))
#             curr_field = "uuid, p, t, d, a"
            curr_field = "uuid, p, t, d, a"
            print(myuuid)
            testuuid="hello"
#             curr_value = (("%s, %0.1f, %0.2f, %0.2f, %0.2f") % (
            curr_value = (("%s, %0.1f, %0.2f, %0.2f, %0.2f") % (
#                 str(myuuid),
                '\"' + str(myuuid) + '\"',
                sensor.pressure(ms5837.UNITS_kPa), # Get pressure in kilopascal(no arguments)
                sensor.temperature(), # Default is degrees C (no arguments)
                sensor.depth(),
                sensor.altitude()))
#             AddCompareList(sensor.altitude(), 5)
            DbInsert('rov_float', curr_field, curr_value)
            time.sleep(5)
        else:
            print("Sensor read retry!")

class StartPressCollection(Resource):
    def get(self):
        if 'p_event' not in app.config or app.config['p_event'].is_set():
            import threading
            p_event = threading.Event()
            app.config['p_event'] = p_event

            task_thread = threading.Thread(target=PressCollection, args=(p_event,))
            task_thread.start()

            return {'message': 'PressCollection started'}
        else:
            return {'message': 'PressCollection already running'}

class StopPressCollection(Resource):
    def get(self):
        if 'p_event' in app.config and not app.config['p_event'].is_set():
            app.config['p_event'].set()
            app.config.pop('p_event', None)
            return {'message': 'PressCollection stopped'}
        else:
            return {'message': 'PressCollection is not running'}

class ReportPressCollection(Resource):
    def get(self):
        return {'message': 'ReportPressCollection API'}
    
########################################################
# Ultrasound Sensor API Function
########################################################
#GPIO Mode (BOARD / BCM)
GPIO.setmode(GPIO.BCM)

#set GPIO Pins
GPIO_TRIGGER = 23
GPIO_ECHO = 24

#set GPIO direction (IN / OUT)
GPIO.setup(GPIO_TRIGGER, GPIO.OUT)
GPIO.setup(GPIO_ECHO, GPIO.IN)

def Ultra_distance():
    # set Trigger to HIGH
    GPIO.output(GPIO_TRIGGER, True)
    # set Trigger after 0.01ms to LOW
    time.sleep(0.00001)
    GPIO.output(GPIO_TRIGGER, False)
    StartTime = time.time()
    StopTime = time.time()
    # save StartTime
    while GPIO.input(GPIO_ECHO) == 0:
        StartTime = time.time()
    # save time of arrival
    while GPIO.input(GPIO_ECHO) == 1:
        StopTime = time.time()
    # time difference between start and arrival
    TimeElapsed = StopTime - StartTime
    # multiply with the sonic speed (34300 cm/s)
    # and divide by 2, because there and back
    distance = (TimeElapsed * 34300) / 2
    return distance

def UltraDisCollection(u_event):
    while not u_event.is_set():
        print("Ultrasound Distance Data Collection Activated")
        dist = ultra_distance()
        if dist:
            print ("Ultrasound Measured Distance = %.1f cm" % dist)
            curr_field = "ultrasound"
            curr_value = dist
            DbInsert('rov_ultrasound', curr_field, curr_value)
            time.sleep(5)
        else:
            print("Sensor read retry!")

class StartUltraDisCollection(Resource):
    def get(self):
        if 'u_event' not in app.config or app.config['u_event'].is_set():
            import threading
            u_event = threading.Event()
            app.config['u_event'] = u_event

            task_thread = threading.Thread(target=UltraDisCollection, args=(u_event,))
            task_thread.start()

            return {'message': 'ultraDisCollection started'}
        else:
            return {'message': 'ultraDisCollection already running'}

class StopUltraDisCollection(Resource):
    def get(self):
        if 'u_event' in app.config and not app.config['u_event'].is_set():
            app.config['u_event'].set()
            app.config.pop('u_event', None)
            return {'message': 'UltraDisCollection stopped'}
        else:
            return {'message': 'UltraDisCollection is not running'}

class ReportUltraDisCollection(Resource):
    def get(self):
        return {'message': 'ReportUltraDisCollection API'}


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
api.add_resource(StartPressCollection, '/p/sp') # Route
api.add_resource(StopPressCollection, '/p/ep') # Route 
api.add_resource(ReportPressCollection, '/p') # Route
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
