// For ESP8266 update pin definitions (using NodeMCU pin labels)
const int dirPin = 5;    // direction control pin for ESP8266
const int stepPin = 4;   // step control pin for ESP8266

const int moveSteps = 200;    // test steps
char cmd;
int data;
int motorSpeed = 1000;

void setup() {
  pinMode(stepPin, OUTPUT);      // Set step pin as output
  pinMode(dirPin, OUTPUT);       // Set direction pin as output

  Serial.begin(9600);
  delay(2000);            // wait for connection to stabilize
  Serial.flush();         // flush any stale data
  Serial.println("++++++++ ESP8266 Single-Stepper Demo ++++++++");
  Serial.println("Please input motor command:");
}

void loop() {
  if (Serial.available()) {
    // Read complete command line terminated by newline.
    String inputLine = Serial.readStringUntil('\n');
    inputLine.trim();  // remove any stray whitespace/newline
    if(inputLine.length() >= 2) {
      cmd = inputLine.charAt(0);
      data = inputLine.substring(1).toInt();
      Serial.print("cmd = ");
      Serial.print(cmd);
      Serial.print(" , data = ");
      Serial.println(data);
      runUsrCmd();
    }
  }
}

// Updated: runUsrCmd without CNC hat commands
void runUsrCmd(){
  switch(cmd){
    case 'U': {
      // Set direction for upward movement
      digitalWrite(dirPin, HIGH);
      runStepper(motorSpeed, data);
      Serial.print("ACK: Moved Up ");
      Serial.print(data);
      Serial.println(" steps");
      break;
    }
    case 'D': {
      // Set direction for downward movement
      digitalWrite(dirPin, LOW);
      runStepper(motorSpeed, data);
      Serial.print("ACK: Moved Down ");
      Serial.print(data);
      Serial.println(" steps");
      break;
    }
    case 'd': {
      // Set motor speed
      motorSpeed = data;
      Serial.print("ACK: Motor speed set to ");
      Serial.println(motorSpeed);
      break;
    }
    default: {
      Serial.println("ERR: Unknown command");
      break;
    }
  }
}

void runStepper (int rotationSpeed, int stepNum){
  for(int x = 0; x < stepNum; x++) {
    if (Serial.available() && Serial.peek() == '!') {
      Serial.read();  // consume emergency stop command
      Serial.println("Emergency Stop!");
      break;
    }
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(rotationSpeed);
    yield(); // allow watchdog feeding
    digitalWrite(stepPin, LOW);
    delayMicroseconds(rotationSpeed);
    yield(); // allow watchdog feeding
  }
}