#define TRIG_PIN 25
#define ECHO_PIN 26
#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);         // ...new code: set LED pin mode...
  digitalWrite(LED_PIN, HIGH);      // ...new code: LED default state ON...
}

void loop() {
  // Clear the trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Send 10 microsecond pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read the echo pin with timeout
  long duration = pulseIn(ECHO_PIN, HIGH, 23200);
  
  // Only process valid readings (changed condition from >= 0 to > 0)
  if (duration > 0) {
    float distance = duration * 0.0343 / 2;
    
    // Check distance: turn off LED if distance is 4 cm or less, else keep LED on
    if (distance <= 4) {
      digitalWrite(LED_PIN, LOW);  // LED off
    } else {
      digitalWrite(LED_PIN, HIGH); // LED on
    }
  
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }
  
  delay(100);
}
