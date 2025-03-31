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

// Sensor iteration variables
float pressures[120];      // changed size: now stores 120 iterations
float temperatures[120];   // changed size: now stores 120 iterations
int sensorIdx = 0;
int iterationCount = 0;    // new counter to track total iterations

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

// Add new global variable for protecting motor control actions.
SemaphoreHandle_t motorControlLock = NULL;

// Add global variables for PID control and depth target
const float targetDepth = 0.5; // target depth in meters
float Kp = 2.0;              // Proportional gain
float Ki = 0.1;              // Integral gain
float Kd = 0.5;              // Derivative gain

float integral = 0.0;
float lastError = 0.0;
const float deadband = 0.02; // 2cm deadband to avoid constant adjustments

// Conversion function from pressure (in appropriate units) to depth in meters.
// You may need to calibrate the conversion factor based on your sensor.
float pressureToDepth(float pressure) {
    const float atmosphericPressure = 101325.0;
    return (pressure - atmosphericPressure) / (997.0 * 9.81);
}

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
    // Starting at sensorIdx yields the oldest reading in the circular buffer.
    for (int i = 0; i < count; i++) {
      int idx = (sensorIdx + i) % 120;
      data += String(pressures[idx]) + "," + String(temperatures[idx]) + "\n";
    }
    xSemaphoreGive(dataLock);
    server.send(200, "text/plain", data);
  }
}

// New function: runDepthHoldSequence replaces the previous two-phase sequence.
void runDepthHoldSequence() {
  // Phase 1: drive motor clockwise ("sucking in water") until within deadband
  while (true) {
    sensor.read();
    float currentDepth = pressureToDepth(sensor.pressure());
    float error = targetDepth - currentDepth;
    if (fabs(error) < deadband)
      break;
    xSemaphoreTake(motorControlLock, portMAX_DELAY);
      digitalWrite(dirPin, LOW); // clockwise
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(motorSpeed);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(motorSpeed);
    xSemaphoreGive(motorControlLock);
    taskYIELD();
  }
  // Phase 2: hold state for 45 seconds (active depthhold)
  unsigned long holdStart = millis();
  while(millis() - holdStart < 45000) {
    delay(100);
  }
  // Phase 3: run anticlockwise ("raising the float") until BUTTON_PIN_2 is pressed
  xSemaphoreTake(motorControlLock, portMAX_DELAY);
    digitalWrite(dirPin, HIGH); // anticlockwise
  xSemaphoreGive(motorControlLock);
  while(digitalRead(BUTTON_PIN_2) == HIGH) {
    xSemaphoreTake(motorControlLock, portMAX_DELAY);
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(motorSpeed);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(motorSpeed);
    xSemaphoreGive(motorControlLock);
    taskYIELD();
  }
}

// Update motorTask
void motorTask(void *parameter) {
  bool command;
  while(1) {
    if(xQueueReceive(motorCommandQueue, &command, portMAX_DELAY)) {
      if (xSemaphoreTake(motorStatusLock, portMAX_DELAY)) {
        motorBusy = true;
        xSemaphoreGive(motorStatusLock);
      }
      
      runDepthHoldSequence(); // replaced runStepperSequence call
      
      if (xSemaphoreTake(motorStatusLock, portMAX_DELAY)) {
        motorBusy = false;
        xSemaphoreGive(motorStatusLock);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Update handleControl messaging to reflect the depth control sequence
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
        "<p>Depth control sequence started.</p>"
        "<p>Float is adjusting to target depth and will hold for 45 seconds.</p>"
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
      html += "<p>Motor is currently running depth control sequence...</p>";
      html += "<button disabled>Start Depth Control Sequence</button>";
    } else {
      html += "<button onclick=\"location.href='/control?action=start'\">Start Depth Control Sequence</button>";
    }
    html += "</body></html>";
    server.send(200, "text/html", html);
  }
}

// Stepper motor routine
void runStepper(int steps, bool clockwise) {
  xSemaphoreTake(motorControlLock, portMAX_DELAY); // Lock motor control
  digitalWrite(dirPin, (clockwise ? LOW : HIGH));
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);
    yield(); // Feed watchdog
  }
  xSemaphoreGive(motorControlLock); // Release lock
}

// New PID control task to continuously adjust motor to maintain target depth.
void maintainDepthTask(void *parameter) {
  const TickType_t delayTicks = pdMS_TO_TICKS(200); // control loop period ~200ms
  while(1) {
    // If depth control sequence is running, skip PID adjustments.
    if(xSemaphoreTake(motorStatusLock, portMAX_DELAY)) {
      if(motorBusy) {
        xSemaphoreGive(motorStatusLock);
        vTaskDelay(delayTicks);
        continue;
      }
      xSemaphoreGive(motorStatusLock);
    }
    
    float currentPressure = 0.0;
    float currentDepth = 0.0;
    // Read the latest sensor pressure value from the circular buffer.
    if(xSemaphoreTake(dataLock, portMAX_DELAY)) {
      int latestIdx = (iterationCount == 0) ? 0 : (sensorIdx == 0 ? 119 : sensorIdx - 1); // corrected index
      currentPressure = pressures[latestIdx];
      xSemaphoreGive(dataLock);
    }
    // Convert pressure to depth.
    currentDepth = pressureToDepth(currentPressure);
    
    // Compute error between target depth and current depth.
    float error = targetDepth - currentDepth;
    
    // Implement deadband to prevent over adjustment for small variations.
    if(fabs(error) < deadband) {
      integral = 0; // reset integral when within deadband to avoid windup
      lastError = error;
    } else {
      // PID calculations
      integral += error * 0.2; // 0.2 sec loop time (approx)
      float derivative = (error - lastError) / 0.2;
      float pidOutput = Kp * error + Ki * integral + Kd * derivative;
      lastError = error;
      
      // Determine motor action from PID output.
      // Here, if pidOutput is positive, we need to add water (or adjust motor in one direction)
      // and if negative, remove water (or adjust motor in opposite direction).
      // Adjust motor speed/step count based on magnitude of pidOutput.
      int steps = abs(pidOutput) * 10; // scale factor (tuning required)
      if(steps > 0) {
        // Use motorControlLock inside runStepper.
        runStepper(steps, (error > 0 ? false : true));
      }
    }
    vTaskDelay(delayTicks);
  }
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
    motorControlLock = xSemaphoreCreateMutex(); // Added mutex for motor control
    
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
    
    // Create the new PID control task for depth maintenance.
    xTaskCreatePinnedToCore(
      maintainDepthTask,
      "MaintainDepthTask",
      4096,
      NULL,
      2,
      NULL,
      1  // Chooses appropriate core
    );
}

void loop() {
  ArduinoOTA.handle();      // Check for OTA updates via ArduinoOTA
  // Other tasks are handled by FreeRTOS tasks.
  vTaskDelay(pdMS_TO_TICKS(1000));
}
