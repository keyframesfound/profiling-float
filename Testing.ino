#include <Wire.h>
#include <MS5837.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Define sensor I2C pins (use D5/D6 as custom pins)
#ifndef D5
#define D5 14 // Modify if necessary
#endif
#ifndef D6
#define D6 12 // Modify if necessary
#endif
#define CUSTOM_SDA_PIN D5 // SDA pin for sensor
#define CUSTOM_SCL_PIN D6 // SCL pin for sensor

// Create sensor instance
MS5837 sensor;

// WiFi credentials
const char* ssid     = "SSCFloat";
const char* password = "DT1234dt";

// Create server instance on port 80
ESP8266WebServer server(80);

// Sensor iteration variables
String iterationData = "";
float pressures[60];      // changed from 10 to 60 for 1 minute aggregation
float temperatures[60];   // changed from 10 to 60 for 1 minute aggregation
int sensorIdx = 0;

// Stepper motor pins and settings
const int dirPin  = 5;  // Direction control
const int stepPin = 4;  // Step control
int motorSpeed = 600;  // Delay in microseconds between steps

// New definitions for buttons and ultrasonic sensor
#define BUTTON_PIN_1 15    // Pin for first button
#define BUTTON_PIN_2 16    // Pin for second button
#define ULTRASONIC_TRIGGER_PIN 2  // Ultrasonic sensor trigger pin
#define ULTRASONIC_ECHO_PIN 0     // Ultrasonic sensor echo pin
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
  
  // Initialize I2C with custom pins
  Wire.begin(CUSTOM_SDA_PIN, CUSTOM_SCL_PIN);

  // Initialize the sensor
  if (!sensor.init()) {
      Serial.println("Could not initialize the MS5837 sensor!");
      while (1); // halt if sensor not detected
  }
  sensor.setFluidDensity(997); // 997 kg/m^3 for freshwater
  Serial.println("MS5837 sensor initialized!");
  
  // Initialize stepper motor pins.
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  
  // Initialize new pins for buttons and ultrasonic sensor
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  
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
