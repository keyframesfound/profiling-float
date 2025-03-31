// Simple stepper motor clockwise rotation program

// Stepper motor pins (from existing codebase)
const int dirPin = 5;   // GPIO5/D5 - Direction control
const int stepPin = 4;  // GPIO4/D4 - Step control
int motorSpeed = 600;   // Delay in microseconds between steps

void setup() {
  // Initialize stepper motor pins
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  
  // Set direction to clockwise
  digitalWrite(dirPin, LOW);  // LOW = clockwise
}

void loop() {
  // Single step clockwise
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(motorSpeed);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(motorSpeed);
}
