#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> // added header

// Replace with your network credentials.
const char* ssid     = "SSCFloat";
const char* password = "DT1234dt";

// Use ESP8266WebServer instead of WiFiServer.
ESP8266WebServer server(80);

// Stepper motor control pins (using NodeMCU pin labels)
const int dirPin = 5;    // Direction control pin
const int stepPin = 4;   // Step control pin

int motorSpeed = 1000;   // Delay in microseconds between step pulses

void setup() {
    // Initialize USB Serial for debugging output at 115200 baud.
    Serial.begin(115200);
    delay(100);
    
    // Create Access Point so you can connect to it.
    Serial.println();
    Serial.print("Creating WiFi network (AP mode) with SSID: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_AP);                         // AP mode active
    WiFi.softAP(ssid, password);                // Start the AP with given credentials
    
    IPAddress apIP = WiFi.softAPIP();            // Get AP IP
    Serial.print("AP IP address: ");
    Serial.println(apIP);
    
    // Register the "/control" route.
    server.on("/control", [](){
        if(server.hasArg("action") && server.arg("action") == "start") {
            // start the stepper motor turning (10000 steps clockwise)
            runStepper(10000, true);
            server.send(200, "text/html", "<html><body><p>Stepper motor turning.</p><a href='/control'>Back</a></body></html>");
        } else {
            // Provide the control page with a button to start the motor.
            server.send(200, "text/html", "<html><body><button onclick=\"location.href='/control?action=start'\">Start Stepper</button></body></html>");
        }
    });
    server.begin(); // start the web server

    // Initialize the stepper motor pins.
    pinMode(dirPin, OUTPUT);
    pinMode(stepPin, OUTPUT);
}

void runStepper(int steps, bool clockwise) {
    // Set motor direction: LOW for clockwise, HIGH for counterclockwise.
    digitalWrite(dirPin, (clockwise ? LOW : HIGH));
    for (int i = 0; i < steps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(motorSpeed);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(motorSpeed);
        yield(); // allow watchdog feeding
    }
}

void loop() {
    // Listen for an incoming client on WiFi port 80.
    server.handleClient(); // process incoming HTTP requests
}