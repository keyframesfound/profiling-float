#define BUTTON_PIN_1 35    // Button connected to D18
#define BUTTON_PIN_2 34    // Button connected to D21
#define LED_PIN_1 4        // LED connected to D4
#define LED_PIN_2 13       // LED connected to D13

void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN_1, INPUT_PULLUP);
    pinMode(BUTTON_PIN_2, INPUT_PULLUP);
    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
}

void loop() {
    // Read button states (pressed = LOW due to internal pull-ups)
    int buttonState1 = digitalRead(BUTTON_PIN_1);
    int buttonState2 = digitalRead(BUTTON_PIN_2);
    
    // Control LEDs based on button state
    digitalWrite(LED_PIN_1, buttonState1 == LOW ? HIGH : LOW);
    digitalWrite(LED_PIN_2, buttonState2 == LOW ? HIGH : LOW);
    
    // Optional: print button states to Serial for debugging
    Serial.print("D18: "); Serial.print(buttonState1);
    Serial.print(" | D21: "); Serial.println(buttonState2);
    
    delay(500);
}