// Test script for stepper motor push operation

// Stepper motor pins (same as main project)
const int dirPin = 5;   // GPIO5/D5
const int stepPin = 4;  // GPIO4/D4
const int buttonPin1 = 13;    // First button
const int buttonPin2 = 14;    // Second button for direction change
int motorSpeed = 600;   // Delay in microseconds between steps
bool isClockwise = true;      // Track direction

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(buttonPin1, INPUT_PULLUP);  // Changed to pullup
  pinMode(buttonPin2, INPUT_PULLUP);  // Added second button
  
  Serial.println("Stepper motor test ready");
  Serial.println("Button 1: Run motor");
  Serial.println("Button 2: Change direction");
}

void loop() {
  // Check for direction change
  if (digitalRead(buttonPin2) == LOW) {  // Button pressed (inverted logic)
    isClockwise = !isClockwise;
    digitalWrite(dirPin, isClockwise ? LOW : HIGH);
    delay(300);  // Debounce delay
  }
  
  if (digitalRead(buttonPin1) == LOW) {  // Button pressed (inverted logic)
    // Run the motor
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);
  }
  
  // Small delay to prevent overwhelming the processor
  delay(1);
}
