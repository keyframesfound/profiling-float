# Profiling Float Project

This project reads sensor data using an MS5837 sensor and aggregates the values for one minute. Additionally, it controls a stepper motor with a sequence triggered via a web interface and utilizes an ultrasonic sensor for float detection.

## Hardware Connections

- **I2C Sensor (MS5837)**
  - **SDA (Sensor SDA):** Connect to GPIO21 (D21) on the ESP32 WROOM 32UE.
  - **SCL (Sensor SCL):** Connect to GPIO22 (D22) on the ESP32 WROOM 32UE.

- **Stepper Motor**
  - **Direction Pin:** Connect to GPIO5 (D5).
  - **Step Pin:** Connect to GPIO4 (D4).

- **Buttons**
  - **Button 1:** Connect to GPIO15 (D15).
  - **Button 2:** Connect to GPIO16 (D16).

- **Ultrasonic Sensor**
  - **Trigger:** Connect to GPIO2 (D2).
  - **Echo:** Connect to GPIO17 (D17).

## WiFi Settings

- The device sets up a WiFi Access Point with:
  - **SSID:** SSCFloat
  - **Password:** DT1234dt

## Code Functionality

- **Sensor Reading:** The MS5837 sensor provides pressure and temperature values.
- **Data Aggregation:** Readings are accumulated over one minute and made available via the `/data` endpoint.
- **Stepper Sequence:** A web endpoint (`/control`) triggers the stepper motor sequence. The sequence:
  - Spins clockwise until Button 1 is pressed.
  - Waits until the ultrasonic sensor detects the float (distance ≤ 10 cm).
  - Waits an additional 45 seconds.
  - Spins anticlockwise until Button 2 is pressed.
- **Over-the-Air Updates:** Supports OTA firmware updates via Arduino OTA protocol  (`/update`)
## Additional Features

- **Over-the-Air Updates:** Supports OTA firmware updates via Arduino OTA protocol
- **Task Management:** Uses FreeRTOS for concurrent operation:
  - Sensor readings run on Core 0
  - Web server runs on Core 0
  - Motor control runs on Core 1 (highest priority)
- **Thread Safety:** Uses mutexes and queues to protect shared resources