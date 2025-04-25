#include <Wire.h>
#include <MS5837.h>
#include <WiFi.h>              // Replaced ESP8266WiFi.h for ESP32
#include <WebServer.h>         // Replaced ESP8266WebServer.h for ESP32
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <ArduinoOTA.h>
#include <Update.h>

// Prototypes
void runStepper(int steps, bool clockwise);
void my_bottle_go_up();
void my_bottle_go_down();
void handleDepthHold();  // New prototype
void myStepperSequence(int depth);  // Update prototype

// Define sensor I2C pins for ESP32 (cables connect as labeled)
#define CUSTOM_SDA_PIN 21 // Sensor SDA (GPIO21/D21)
#define CUSTOM_SCL_PIN 22 // Sensor SCL (GPIO22/D22)

// Create sensor instance
MS5837 sensor;

// WiFi credentials
const char* ssid     = "SSCFloatTest";
const char* password = "DT1234dt";

// Create server instance on port 80
WebServer server(80);  // Using ESP32 WebServer

// Sensor iteration variables
float pressures[120];      // Changed size: now stores 120 iterations
float temperatures[120];   // Changed size: now stores 120 iterations
int sensorIdx = 0;
int iterationCount = 0;    // New counter to track total iterations

// Stepper motor pins and settings with connection labels for ESP32
const int dirPin  = 5;  // Connect Stepper Direction to GPIO5/D5
const int stepPin = 4;  // Connect Stepper Step to GPIO4/D4
int motorSpeed = 600;  // Delay in microseconds between steps

// New definitions for buttons with connection labels
#define BUTTON_PIN_1 13    // Changed from GPIO15 to GPIO13 to avoid strapping pin
#define BUTTON_PIN_2 14    // Changed from GPIO16 (unavailable) to GPIO14 

// Mutex for protecting shared resources
SemaphoreHandle_t dataLock = NULL;
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t webTaskHandle = NULL;
TaskHandle_t motorTaskHandle = NULL;

// Queue for motor commands
QueueHandle_t motorCommandQueue = NULL;

// Add these near other global variables
volatile bool motorBusy = false;
SemaphoreHandle_t motorStatusLock = NULL;

// Sensor reading task
void sensorTask(void *parameter) {
  while(1) {
    sensor.read();
    
    if (xSemaphoreTake(dataLock, portMAX_DELAY)) {
      pressures[sensorIdx] = sensor.pressure();
      temperatures[sensorIdx] = sensor.temperature();
      sensorIdx = (sensorIdx + 1) % 120;
      iterationCount++;
      xSemaphoreGive(dataLock);
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second delay
  }
}

// Web server task
void webTask(void *parameter) {
  while(1) {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to prevent watchdog triggers
  }
}

// Updated web endpoint handler for /data to always display the latest readings
void handleData() {
  if (xSemaphoreTake(dataLock, portMAX_DELAY)) {
    int count = iterationCount < 120 ? iterationCount : 120;
    String data = "";
    for (int i = 0; i < count; i++) {
      int idx = (sensorIdx + i) % 120;
      data += String(pressures[idx]) + "," + String(temperatures[idx]) + "\n";
    }
    xSemaphoreGive(dataLock);
    server.send(200, "text/plain", data);
  }
}

// New handler for /depthhold
void handleDepthHold() {
    if (server.hasArg("depth")) {
        int depth = server.arg("depth").toInt();  // Get depth from URL
        myStepperSequence(depth);  // Pass depth to the sequence
        server.send(200, "text/html", 
            "<html><body>"
            "<p>Depth hold sequence started for depth: " + String(depth) + ".</p>"
            "<a href='/control'>Back</a>"
            "</body></html>");
    } else {
        server.send(400, "text/html", 
            "<html><body>"
            "<p>Invalid request. Please provide a depth parameter.</p>"
            "<a href='/control'>Back</a>"
            "</body></html>");
    }
}

// Modified runStepperSequence to add 45-second delay after bottom detection
void runStepperSequence() {
    digitalWrite(dirPin, LOW);  // clockwise
    while(digitalRead(BUTTON_PIN_1) == LOW) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorSpeed);
        taskYIELD(); // allow other tasks to run
    }
    delay(5000); // 5-second delay after hitting the bottom
    digitalWrite(dirPin, HIGH); // anticlockwise
    while(digitalRead(BUTTON_PIN_2) == LOW) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorSpeed);
        taskYIELD();
    }
}

void myStepperSequence(int depth) {
  int myStepperCount = 0;
  while (digitalRead(BUTTON_PIN_2) == LOW) {
    runStepper(10, false);
  }

  while (digitalRead(BUTTON_PIN_1) == LOW && myStepperCount <= depth) {
    runStepper(10, true);
    myStepperCount += 10;
  }

  while (digitalRead(BUTTON_PIN_1) == LOW && myStepperCount <= 4700) {
    runStepper(10, true);
    myStepperCount += 10;
  }

  unsigned long startTime = millis();
  motorSpeed = 2000;
  while ((millis() - startTime) / 1000 <= 300 && myStepperCount <= 4780 && myStepperCount >= 4620) {
    if (sensor.pressure() > 1030) { // Use your desired threshold here
      my_bottle_go_up();
    } else {
      my_bottle_go_down();
    }
  }
}

void my_bottle_go_up(){
  if(digitalRead(BUTTON_PIN_2) == LOW){
    runStepper(10, false);
  }
}

void my_bottle_go_down(){
  if(digitalRead(BUTTON_PIN_1) == LOW){
    runStepper(10, true);
  }
}

// Modify motorTask
void motorTask(void *parameter) {
  bool command;
  while(1) {
    if(xQueueReceive(motorCommandQueue, &command, portMAX_DELAY)) {
      if (xSemaphoreTake(motorStatusLock, portMAX_DELAY)) {
        motorBusy = true;
        xSemaphoreGive(motorStatusLock);
      }
      
      runStepperSequence();
      
      if (xSemaphoreTake(motorStatusLock, portMAX_DELAY)) {
        motorBusy = false;
        xSemaphoreGive(motorStatusLock);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Update the web endpoint handler to trigger the new sequence
void handleControl() {
  if(server.hasArg("action") && server.arg("action") == "start") {
    bool canStart = false;
    if (xSemaphoreTake(motorStatusLock, portMAX_DELAY)) {
      canStart = !motorBusy;
      xSemaphoreGive(motorStatusLock);
    }
    if (canStart) {
      bool command = true;
      xQueueSend(motorCommandQueue, &command, portMAX_DELAY);
      server.send(200, "text/html", 
        "<html><body>"
        "<p>Stepper sequence started.</p>"
        "<p>Motor is busy. Please wait for completion.</p>"
        "<a href='/control'>Back</a>"
        "</body></html>");
    } else {
      server.send(200, "text/html", 
        "<html><body>"
        "<p>Motor is currently busy. Please wait.</p>"
        "<a href='/control'>Back</a>"
        "</body></html>");
    }
  } else {
    bool isBusy = false;
    if (xSemaphoreTake(motorStatusLock, portMAX_DELAY)) {
      isBusy = motorBusy;
      xSemaphoreGive(motorStatusLock);
    }
    String html = "<html><body>";
    if (isBusy) {
      html += "<p>Motor is currently running...</p>";
      html += "<button disabled>Start Stepper Sequence</button>";
    }
    html += "</body></html>";
    server.send(200, "text/html", html);
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

    runStepper(2000, true);
    Serial.println("Serial communication test...");

    Wire.begin(CUSTOM_SDA_PIN, CUSTOM_SCL_PIN);
    if (!sensor.init()) {
        while (1); // Halt if sensor not detected
    }
    sensor.setFluidDensity(997);

    pinMode(dirPin, OUTPUT);
    pinMode(stepPin, OUTPUT);
    pinMode(BUTTON_PIN_1, INPUT_PULLUP);
    pinMode(BUTTON_PIN_2, INPUT_PULLUP);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("Access Point started. IP: ");
    Serial.println(WiFi.softAPIP());

    ArduinoOTA.begin();
    Serial.println("OTA Ready");

    // Define web server endpoints.
    server.on("/data", handleData);
    server.on("/control", handleControl);
    server.on("/depthhold", handleDepthHold);  // New endpoint

    server.begin();

    dataLock = xSemaphoreCreateMutex();
    motorCommandQueue = xQueueCreate(1, sizeof(bool));
    motorStatusLock = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 2, &sensorTaskHandle, 0);
    xTaskCreatePinnedToCore(webTask, "WebTask", 4096, NULL, 1, &webTaskHandle, 0);
    xTaskCreatePinnedToCore(motorTask, "MotorTask", 4096, NULL, 3, &motorTaskHandle, 1);
    myStepperSequence(0);  // Initial call with depth 0
}

void loop() {
  ArduinoOTA.handle();      // Check for OTA updates via ArduinoOTA
  vTaskDelay(pdMS_TO_TICKS(1000));
}