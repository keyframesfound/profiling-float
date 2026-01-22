# Profiling Float Project

An ESP32-based automated profiling float system for water measurements using MS5837 pressure/temperature sensor.

#Step By Step 
1. Download the ESP32.ino
2. Use the arduino IDE, download the ESP board library
<img width="196" height="25" alt="image" src="https://github.com/user-attachments/assets/461de9f4-e31e-4bcb-94f3-0e19d5db4e64" />

3. Change to QIQ
<img width="580" height="53" alt="image (1)" src="https://github.com/user-attachments/assets/8703dccc-c23d-4ab1-8618-4a01f6d4fe4a" />

4. Upload speed can be adjusted faster
<img width="603" height="100" alt="image (2)" src="https://github.com/user-attachments/assets/938a8ade-f2ee-4045-8e7a-6422df6e0f56" />

*Remarks: 
- You can flash the code onto the ESP32 through wifi (connect to the SSCFloat Wifi) so no need to disassemble the float each time you guys want to change the code, however make sure OTA is at the end of the code
- Pressure sensor is 3v not 5v so if data that comes out is weird then check power source
- Float wifi and web server will **NOT** turn on if **NO** pressure sensor is detected
## Hardware Requirements

- ESP32 Development Board
- MS5837 Pressure/Temperature Sensor
- Stepper Motor
- 2x Limit Switches
- HC-SR04 Ultrasonic Sensor
- Power Supply
- Waterproof Housing

## Pin Connections

### Sensor Connections
- MS5837 SDA → GPIO21 (D21) [WHITE cable]
- MS5837 SCL → GPIO22 (D22) [GREEN cable]

### Stepper Motor
- Direction Pin → GPIO5 (D5) (BLUE)
- Step Pin → GPIO4 (D4) (GREEN)

### Buttons/Switches
- Bottom Limit Switch → GPIO13 (D13) (Button TOP)
- Top Limit Switch → GPIO14 (D14) (Button Bottom)


## WiFi Setup

The device creates an Access Point with:
- SSID: SSCFloat
- Password: DT1234dt
- IP Address: 192.168.4.1

## Web Interface

- `/data` - View pressure and temperature readings
- `/control` - Control stepper motor sequence

## Operation

1. The float performs continuous pressure/temperature measurements
2. Motor sequence can be triggered via web interface
3. Sequence includes:
   - Descent until bottom limit switch
   - Wait for float detection via ultrasonic sensor
   - 45-second measurement pause
   - Ascent until top limit switch

## Development

Built using Arduino IDE or PlatformIO. Required libraries:
- MS5837
- WiFi
- WebServer
- Wire
- ArduinoOTA
- Update

## OTA Updates

Supports both ArduinoOTA and web-based OTA updates for easy firmware deployment.
Can use WIFI to flash onto the ESP32 through connecting to it's WIFI
