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

// Define sensor I2C pins for ESP32 (cables connect as labeled)
#define CUSTOM_SDA_PIN 21 // Sensor SDA (GPIO21/D21)
#define CUSTOM_SCL_PIN 22 // Sensor SCL (GPIO22/D22)

// Create sensor instance
MS5837 sensor;

// WiFi credentials
const char* ssid     = "SSCFloat";
const char* password = "DT1234dt";

// Create server instance on port 80
WebServer server(80);  // Using ESP32 WebServer

#define BUFFER_SIZE       120
#define BOTTOM_DELAY_MS   45000     // 45-second delay

// Sensor iteration variables
float pressures[BUFFER_SIZE];      // using BUFFER_SIZE constant
float temperatures[BUFFER_SIZE];   // using BUFFER_SIZE constant
int sensorIdx = 0;
int iterationCount = 0;            // counter for total iterations

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
      sensorIdx = (sensorIdx + 1) % BUFFER_SIZE;
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
    int count = iterationCount < BUFFER_SIZE ? iterationCount : BUFFER_SIZE;
    String data = "";
    // Starting at sensorIdx yields the oldest reading in the circular buffer.
    for (int i = 0; i < count; i++) {
      int idx = (sensorIdx + i) % BUFFER_SIZE;
      data += String(pressures[idx]) + "," + String(temperatures[idx]) + "\n";
    }
    xSemaphoreGive(dataLock);
    server.send(200, "text/plain", data);
  }
}

// Simple debounce helper (tunable delay)
bool debounceRead(int pin, int delayMs = 50) {
    if(digitalRead(pin) == LOW) {
      delay(delayMs);
      return (digitalRead(pin) == LOW);
    }
    return false;
}

// Modified runStepperSequence to add 45-second delay after bottom detection
void runStepperSequence() {
    // Spin clockwise until BUTTON_PIN_1 is pressed with debounce check
    digitalWrite(dirPin, LOW);  // clockwise
    while(!debounceRead(BUTTON_PIN_1)) {  // wait until button is confirmed pressed
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorSpeed);
        taskYIELD(); // allow other tasks to run
    }
    // 45-second delay after hitting the bottom
    delay(BOTTOM_DELAY_MS);
    // Spin anticlockwise until BUTTON_PIN_2 is pressed with debounce check
    digitalWrite(dirPin, HIGH); // anticlockwise
    while(!debounceRead(BUTTON_PIN_2)) {  // wait until button is confirmed pressed
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorSpeed);
        taskYIELD();
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
// Modify handleControl
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
    } else {
      html += "<button onclick=\"location.href='/control?action=start'\">Start Stepper Sequence</button>";
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

// Modified depthHoldTask to implement sink, hold for 45s, then ascend.
void depthHoldTask(void *parameter) {
  const float targetDepth = 0.3;   // Target depth in meters.
  const float tolerance = 0.02;    // Tolerance in meters.
  float lastPressure = 0, currentDepth = 0;
  motorBusy = true;

  while(1) {
    float lastPressure = 0;
    // Retrieve the most recent pressure reading from the circular buffer.
    if(xSemaphoreTake(dataLock, portMAX_DELAY)) {
      int idx = (sensorIdx + 119) % BUFFER_SIZE;  // Last recorded index.
      lastPressure = pressures[idx];
      xSemaphoreGive(dataLock);
    }
    currentDepth = (lastPressure - 1013.25) * 100.0 / (997.0 * 9.81);
    // If the bottom limit button is pressed, stop descending.
    if (digitalRead(BUTTON_PIN_1) == LOW) break;
    // Exit if desired depth reached.
    if (currentDepth >= targetDepth - tolerance) break;
    // Sink: set direction downward (LOW) and step.
    digitalWrite(dirPin, LOW);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);
    yield();
  }

  // Hold phase: actively maintain target depth for 45 seconds.
  unsigned long holdStart = millis();
  while (millis() - holdStart < 45000) {
    if (xSemaphoreTake(dataLock, portMAX_DELAY)) {
      int idx = (sensorIdx + 119) % 120;
      lastPressure = pressures[idx];
      xSemaphoreGive(dataLock);
    }
    currentDepth = (lastPressure - 1013.25) * 100.0 / (997.0 * 9.81);
    float error = targetDepth - currentDepth;
    // If too shallow, sink a little.
    if (error > tolerance) {
      digitalWrite(dirPin, LOW);
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(motorSpeed);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(motorSpeed);
    }
    // If too deep, ascend a little.
    else if (error < -tolerance) {
      digitalWrite(dirPin, HIGH);
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(motorSpeed);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(motorSpeed);
    }
    // Abort adjustments if any limit button is triggered.
    if (digitalRead(BUTTON_PIN_1) == LOW || digitalRead(BUTTON_PIN_2) == LOW) break;
    yield();
  }

  // Ascend phase: raise float until top limit button is triggered.
  while (digitalRead(BUTTON_PIN_2) != LOW) {
    digitalWrite(dirPin, HIGH);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);
    yield();
  }
  motorBusy = false;
  vTaskDelete(NULL); // End task once complete.
}

void setup() {
    Serial.begin(115200);  // Re-enable Serial
    delay(1000);
    
    // Test serial communication
    Serial.println("Serial communication test...");
    Serial.println("If you can read this, serial RX/TX is working!");
    
    // Initialize I2C with custom pins (Sensor: SDA on GPIO21, SCL on GPIO22)
    Wire.begin(CUSTOM_SDA_PIN, CUSTOM_SCL_PIN);

    // Initialize the sensor (remove Serial messages)
    if (!sensor.init()) {
        while (1); // halt if sensor not detected
    }
    sensor.setFluidDensity(997);
    
    // Initialize stepper motor pins with connection labels
    pinMode(dirPin, OUTPUT);  // Connect to Stepper Direction pin (GPIO5/D5)
    pinMode(stepPin, OUTPUT); // Connect to Stepper Step pin (GPIO4/D4)
    
    // Initialize new pins for buttons with labels
    pinMode(BUTTON_PIN_1, INPUT_PULLUP); // Changed from INPUT
    pinMode(BUTTON_PIN_2, INPUT_PULLUP); // Changed from INPUT
    
    // Initialize WiFi in AP mode.
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("Access Point started. IP: ");
    Serial.println(WiFi.softAPIP());
    
    // ArduinoOTA setup
    ArduinoOTA.onStart([]() {
      String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nUpdate Complete");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress * 100) / total);
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("OTA Ready");
    
    // Define web server endpoints.
    server.on("/data", handleData);
    server.on("/control", handleControl);
    
    server.begin();

    // Create mutex and queue
    dataLock = xSemaphoreCreateMutex();
    motorCommandQueue = xQueueCreate(1, sizeof(bool));
    
    // Add this before creating tasks
    motorStatusLock = xSemaphoreCreateMutex();
    
    // Create tasks with different priorities
    xTaskCreatePinnedToCore(
      sensorTask,
      "SensorTask",
      4096,
      NULL,
      2,
      &sensorTaskHandle,
      0  // Run on Core 0
    );
    
    xTaskCreatePinnedToCore(
      webTask,
      "WebTask",
      4096,
      NULL,
      1,
      &webTaskHandle,
      0  // Run on Core 0
    );
    
    xTaskCreatePinnedToCore(
      motorTask,
      "MotorTask",
      4096,
      NULL,
      3,  // Highest priority
      &motorTaskHandle,
      1  // Run on Core 1
    );
    
    // Create depth hold task on Core 1 with a low priority
    xTaskCreatePinnedToCore(
      depthHoldTask,
      "DepthHoldTask",
      4096,
      NULL,
      1,
      NULL,
      1  // Run on Core 1
    );
}

void loop() {
  ArduinoOTA.handle();      // Check for OTA updates via ArduinoOTA
  // Other tasks are handled by FreeRTOS tasks.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
