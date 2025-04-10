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

// Web endpoint handler for sensor data (/data)
void handleData() {
  server.send(200, "text/plain", iterationData);
}

// Web endpoint handler for stepper motor control (/control)
void handleControl() {
  if(server.hasArg("action") && server.arg("action") == "start") {
    runStepper(10000, true);  // Run clockwise for 10000 steps
    delay(45000);             // Pause for 45 seconds
    runStepper(10000, false); // Run anticlockwise for 10000 steps
    server.send(200, "text/html", "<html><body><p>Stepper executed: 10000 steps clockwise, 45 sec pause, 10000 steps anticlockwise.</p><a href='/control'>Back</a></body></html>");
  } else {
    server.send(200, "text/html", "<html><body><button onclick=\"location.href='/control?action=start'\">Start Stepper</button></body></html>");
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
