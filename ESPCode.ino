/*
Arduino CNC电机扩展板驱动4个NEMA17步进电机示例程序
By 太极创客（http://www.taichi-maker.com）
2019-03-10
 
本示例程序旨在演示如何使用Arduino Uno开发板通过Arduino CNC电机扩展板来驱动4个NEMA17步进电机。
 
如需获得更多关于本示例程序的电路连接以及CNC电机扩展板的资料信息，
请参考太极创客网站（http://www.taichi-maker.com），并在首页搜索栏中搜索关键字：CNC扩展板
*/

// For ESP8266 update pin definitions (using NodeMCU pin labels)
const int dirPin = D1;    // direction control pin for ESP8266
const int stepPin = D2;   // step control pin for ESP8266

const int moveSteps = 200;    // test steps
char cmd;
int data;
int motorSpeed = 1000;

void setup() {
  pinMode(stepPin, OUTPUT);      // Set step pin as output
  pinMode(dirPin, OUTPUT);       // Set direction pin as output

  Serial.begin(9600);
  Serial.println("++++++++ ESP8266 Single-Stepper Demo ++++++++");
  Serial.println("Please input motor command:");
}

void loop() {
  if (Serial.available()) {
    cmd = Serial.read();
    Serial.print("cmd = ");
    Serial.print(cmd);
    Serial.print(" , ");
    data = Serial.parseInt();
    Serial.print("data = ");
    Serial.println(data);
    runUsrCmd();
  }
}

// Updated: runUsrCmd without CNC hat commands
void runUsrCmd(){
  switch(cmd){ 
    case 'x':  // Set motor rotation direction
      Serial.print("Set Rotation To ");
      if (data == 0){
        digitalWrite(dirPin, LOW);
        Serial.println("Counter Clockwise.");
      } else {
        digitalWrite(dirPin, HIGH);
        Serial.println("Clockwise.");
      }
      break;
      
    case 'z': // Run stepper for a number of steps using set motor speed
      runStepper(motorSpeed, data);
      break;
 
    case 'd': // Set motor step delay speed
      motorSpeed = data;
      Serial.print("Set Motor Speed To ");
      Serial.println(data);
      break;
              
    default:
      Serial.println("Unknown Command");
  }
}

void runStepper (int rotationSpeed, int stepNum){
  for(int x = 0; x < stepNum; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(rotationSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(rotationSpeed);
  }
}
