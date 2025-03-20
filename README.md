# Profiling Float Project

This project reads sensor data using an MS5837 sensor and aggregates the values for one minute. Additionally, it controls a stepper motor with a sequence triggered via a web interface and utilizes an ultrasonic sensor for float detection.

## Hardware Connections

- **I2C Sensor (MS5837)**
  - **SDA (Sensor SDA):** Connect to D5, which is GPIO14 on the ESP32.
  - **SCL (Sensor SCL):** Connect to D6, which is GPIO12 on the ESP32.

- **Stepper Motor**
  - **Direction Pin:** Connect to GPIO5 (dirPin).
  - **Step Pin:** Connect to GPIO4 (stepPin).

- **Buttons**
  - **Button 1:** Connect to GPIO15 (BUTTON_PIN_1).
  - **Button 2:** Connect to GPIO16 (BUTTON_PIN_2).

- **Ultrasonic Sensor**
  - **Trigger:** Connect to GPIO2 (ULTRASONIC_TRIGGER_PIN).
  - **Echo:** Connect to GPIO0 (ULTRASONIC_ECHO_PIN) – use caution with this pin on the ESP32.

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