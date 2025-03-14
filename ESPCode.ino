#include <ESP8266WiFi.h>  // include WiFi library

// Replace with your network credentials.
const char* ssid     = "ssid";
const char* password = "password";

// For ESP8266 update pin definitions (using NodeMCU pin labels)
const int dirPin = 5;    // direction control pin for ESP8266
const int stepPin = 4;   // step control pin for ESP8266

const int moveSteps = 200;    // (unused now) test steps
int motorSpeed = 1000;        // delay in microseconds between step pulses

// Create a WiFi server on port 80
WiFiServer server(80);

void setup() {
  pinMode(stepPin, OUTPUT);      // Set step pin as output
  pinMode(dirPin, OUTPUT);       // Set direction pin as output
  
  // Initialize Serial for debug output
  Serial.begin(9600);
  Serial.println("++++++++ ESP8266 Stepper HTTP Demo ++++++++");

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();
}

void loop() {
  // Listen for incoming clients
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("HTTP ping received, moving stepper motor...");
    
    // Wait until client sends request data (can be empty)
    while (client.connected() && !client.available()) {
      delay(1);
    }
    // Read (and discard) the incoming request
    while(client.available()){
      client.read();
    }
    
    // Set motor direction to clockwise (LOW)
    digitalWrite(dirPin, LOW);
    
    // Move the stepper motor 1000 steps
    runStepper(motorSpeed, 1000);
    
    // Send HTTP response
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

void runStepper (int rotationSpeed, int stepNum){
  for(int x = 0; x < stepNum; x++) {
    // Check for an emergency stop command coming in over WiFi
    // (this example assumes no emergency stop in HTTP mode; modify as needed)
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(rotationSpeed);
    yield(); // allow watchdog feeding
    digitalWrite(stepPin, LOW);
    delayMicroseconds(rotationSpeed);
    yield(); // allow watchdog feeding
  }
}