#include <Wire.h>
#include <MS5837.h>
#include <WiFi.h>              // Replaced ESP8266WiFi.h for ESP32
#include <WebServer.h>         // Replaced ESP8266WebServer.h for ESP32

// Define sensor I2C pins for ESP32 (cables connect as labeled)
#ifndef D5
#define D5 14 // Connect sensor SDA to GPIO14 on ESP32
#endif
#ifndef D6
#define D6 12 // Connect sensor SCL to GPIO12 on ESP32
#endif
#define CUSTOM_SDA_PIN D5 // Sensor SDA (GPIO14)
#define CUSTOM_SCL_PIN D6 // Sensor SCL (GPIO12)

// Create sensor instance
MS5837 sensor;

// WiFi credentials
const char* ssid     = "SSCFloat";
const char* password = "DT1234dt";

// Create server instance on port 80
WebServer server(80);  // Using ESP32 WebServer

// Sensor iteration variables
String iterationData = "";
float pressures[60];      // changed from 10 to 60 for 1 minute aggregation
float temperatures[60];   // changed from 10 to 60 for 1 minute aggregation
int sensorIdx = 0;

// Stepper motor pins and settings with connection labels for ESP32
const int dirPin  = 5;  // Connect Stepper Direction to GPIO5
const int stepPin = 4;  // Connect Stepper Step to GPIO4
int motorSpeed = 600;  // Delay in microseconds between steps

// New definitions for buttons and ultrasonic sensor with connection labels
#define BUTTON_PIN_1 15    // Connect Button 1 to GPIO15
#define BUTTON_PIN_2 16    // Connect Button 2 to GPIO16
#define ULTRASONIC_TRIGGER_PIN 2  // Connect Ultrasonic Trigger to GPIO2
#define ULTRASONIC_ECHO_PIN 0     // Connect Ultrasonic Echo to GPIO0 (use caution on ESP32)
const float ULTRASONIC_DIST_THRESHOLD = 10.0; // Threshold in cm

// Web endpoint handler for sensor data (/data)
void handleData() {
  server.send(200, "text/plain", iterationData);
}

// New function to measure distance with the ultrasonic sensor
float getUltrasoundDistance() {
    long duration;
    digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
    duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
    return duration * 0.034 / 2; // Convert to cm
}

// Modified runStepperSequence to add 45-second delay after bottom detection
void runStepperSequence() {
    // Spin clockwise until BUTTON_PIN_1 is pressed
    digitalWrite(dirPin, LOW);  // clockwise
    while(digitalRead(BUTTON_PIN_1) != LOW) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorSpeed);
    }
    // Wait until the ultrasound sensor detects the float (distance <= threshold)
    while(getUltrasoundDistance() > ULTRASONIC_DIST_THRESHOLD) {
        delay(10);
    }
    // 45-second delay after hitting the bottom
    delay(45000);
    // Spin anticlockwise until BUTTON_PIN_2 is pressed
    digitalWrite(dirPin, HIGH); // anticlockwise
    while(digitalRead(BUTTON_PIN_2) != LOW) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorSpeed);
    }
}

// Update the web endpoint handler to trigger the new sequence
void handleControl() {
  if(server.hasArg("action") && server.arg("action") == "start") {
    runStepperSequence();
    server.send(200, "text/html", "<html><body><p>Stepper sequence executed.</p><a href='/control'>Back</a></body></html>");
  } else {
    server.send(200, "text/html", "<html><body><button onclick=\"location.href='/control?action=start'\">Start Stepper Sequence</button></body></html>");
  }
}

// Stepper motor routine
void runStepper(int steps, bool clockwise) {
  digitalWrite(dirPin, (clockwise ? LOW : HIGH));
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);
    yield(); // Feed watchdog
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize I2C with custom pins (Sensor: SDA on GPIO14, SCL on GPIO12)
  Wire.begin(CUSTOM_SDA_PIN, CUSTOM_SCL_PIN);

  // Initialize the sensor
  if (!sensor.init()) {
      Serial.println("Could not initialize the MS5837 sensor!");
      while (1); // halt if sensor not detected
  }
  sensor.setFluidDensity(997); // 997 kg/m^3 for freshwater
  Serial.println("MS5837 sensor initialized!");
  
  // Initialize stepper motor pins with connection labels
  pinMode(dirPin, OUTPUT);  // Connect to Stepper Direction pin (GPIO5)
  pinMode(stepPin, OUTPUT); // Connect to Stepper Step pin (GPIO4)
  
  // Initialize new pins for buttons and ultrasonic sensor with labels
  pinMode(BUTTON_PIN_1, INPUT_PULLUP); // Connect Button 1 (GPIO15)
  pinMode(BUTTON_PIN_2, INPUT_PULLUP); // Connect Button 2 (GPIO16)
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT); // Connect Ultrasonic Trigger (GPIO2)
  pinMode(ULTRASONIC_ECHO_PIN, INPUT); // Connect Ultrasonic Echo (GPIO0)
  
  // Initialize WiFi in AP mode.
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("Access Point started. IP: ");
  Serial.println(WiFi.softAPIP());
  
  // Define web server endpoints.
  server.on("/data", handleData);
  server.on("/control", handleControl);
  server.begin();
}

void loop() {
  // Sensor reading every 1 second.
  sensor.read();
  pressures[sensorIdx] = sensor.pressure();
  temperatures[sensorIdx] = sensor.temperature();
  sensorIdx++;
  if (sensorIdx >= 60) { // changed condition: now aggregates readings for 1 minute
    iterationData = "";
    for (int i = 0; i < 60; i++) {
      iterationData += String(pressures[i]) + "," + String(temperatures[i]) + "\n";
    }
    sensorIdx = 0;
  }
  
  // Handle incoming client requests.
  server.handleClient();
  delay(1000);
}
