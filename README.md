# Profiling Float Project

An ESP32-based automated profiling float system for water measurements using MS5837 pressure/temperature sensor.

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