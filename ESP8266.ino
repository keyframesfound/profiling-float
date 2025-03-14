#include <ESP8266WiFi.h>

// Replace with your network credentials.
const char* ssid     = "ssid";
const char* password = "123123123";

// WiFi server listening on port 80 for incoming HTTP requests
WiFiServer server(80);

// Stepper motor control pins (using NodeMCU pin labels)
const int dirPin = 5;    // Direction control pin
const int stepPin = 4;   // Step control pin

int motorSpeed = 1000;   // Delay in microseconds between step pulses

void setup() {
    // Initialize USB Serial for debugging output at 115200 baud.
    Serial.begin(115200);
    delay(100);

    // Connect to WiFi network using the WiFi port.
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    // Start the WiFi server on port 80.
    server.begin();

    // Initialize the stepper motor pins.
    pinMode(dirPin, OUTPUT);
    pinMode(stepPin, OUTPUT);
}

void runStepper(int steps) {
    // Set motor direction to clockwise (assuming LOW for clockwise).
    digitalWrite(dirPin, LOW);
  
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
    WiFiClient client = server.available();
    
    if (client) {
        Serial.println("New client pinged on WiFi port 80. Moving stepper motor 1000 steps clockwise.");
        
        // Wait until the client sends data then flush the incoming request.
        while (client.connected() && !client.available()) {
            delay(1);
        }
        while (client.available()) {
            client.read();
        }
        
        // Move the stepper motor 1000 steps clockwise.
        runStepper(1000);
          
        // Send HTTP response back to the client.
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("Stepper motor moved 1000 steps clockwise.");
        
        delay(1);
        client.stop();
        Serial.println("HTTP client disconnected.");
    }
}