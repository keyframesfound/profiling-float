void setup() {
  Serial.begin(115200);  // Initialize serial communication at 115200 baud rate
}

void loop() {
  if (Serial.available() > 0) {
    String received = Serial.readStringUntil('\n');  // Read until newline
    received.trim();  // Remove any whitespace
    
    if (received == "D100") {
      Serial.println("ok received");
    }
  }
}
