#include <Wire.h>
#include <MS5837.h>
#include <ESP8266WiFi.h>      // WiFi library added
#include <ESP8266WebServer.h> // Web server library added

#ifndef D5
#define D5 14 // Define D5 if not already defined (modify as needed))
#endif
#ifndef D6
#define D6 12 // Define D6 if not already defined (modify as needed)
#endif

MS5837 sensor; // Create an instance of the MS5837 class

// Define custom I2C pins
#define CUSTOM_SDA_PIN D5 // Green cable is connected to D5 (SDA)
#define CUSTOM_SCL_PIN D6 // White cable is connected to D6 (SCL)

// WiFi credentials and server instance
const char* ssid = "SSCFloat";
const char* password = "DT1234dt";
ESP8266WebServer server(80);

String iterationData = ""; // Stores 10 iteration readings

// Web handler for /data endpoint
void handleData() {
  server.send(200, "text/plain", iterationData);
}

void setup() {
 // Initialize the serial monitor
 Serial.begin(115200);
 delay(1000);
 // Initialize the I2C communication with custom SDA and SCL pins
 Wire.begin(CUSTOM_SDA_PIN, CUSTOM_SCL_PIN);
 // Initialize the MS5837 sensor
 if (!sensor.init()) {
   Serial.println("Could not initialize the MS5837 sensor!");
   while (1); // Stop execution if the sensor is not detected
 }
 // Set the fluid density (default: 1029 kg/m^3 for seawater)
 // You can adjust this value for freshwater or other fluids
 sensor.setFluidDensity(997); // 997 kg/m^3 for freshwater
 Serial.println("MS5837 sensor initialized!");

 // Initialize WiFi in AP mode
 WiFi.mode(WIFI_AP);
 WiFi.softAP(ssid, password);
 Serial.print("Access Point started. IP: ");
 Serial.println(WiFi.softAPIP());

 // Start web server with /data endpoint
 server.on("/data", handleData);
 server.begin();
}

void loop() {
    static float pressures[10];
    static float temperatures[10];
    static int idx = 0;
    
    sensor.read();
    pressures[idx] = sensor.pressure();
    temperatures[idx] = sensor.temperature();
    idx++;
    
    if (idx >= 10) {
        iterationData = "";
        for (int i = 0; i < 10; i++) {
            iterationData += String(pressures[i]) + ","+
                             String(temperatures[i]) + "\n";
        }
        idx = 0;
    }
    
    server.handleClient();
    delay(1000);
}