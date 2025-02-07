/*
Arduino CNC电机扩展板驱动4个NEMA17步进电机示例程序
By 太极创客（http://www.taichi-maker.com）
2019-03-10
 
本示例程序旨在演示如何使用Arduino Uno开发板通过Arduino CNC电机扩展板来驱动4个NEMA17步进电机。
 
如需获得更多关于本示例程序的电路连接以及CNC电机扩展板的资料信息，
请参考太极创客网站（http://www.taichi-maker.com），并在首页搜索栏中搜索关键字：CNC扩展板
*/
#include <AccelStepper.h>  //本示例程序使用AccelStepper库
 
// 定义电机控制用常量
const int enPin = 8;  // 使能控制引脚
 
const int dirPin = 5;     // x方向控制引脚
const int stepPin = 2;    // x步进控制引脚

 
const int moveSteps = 200;    //测试电机运行使用的运行步数

char cmd;
int data;
int motorSpeed = 1000;
 
AccelStepper stepper1(1,stepPin,dirPin);//建立步进电机对象1

 
void setup() {
  
  pinMode(stepPin,OUTPUT);     // Arduino控制A4988x步进引脚为输出模式
  pinMode(dirPin,OUTPUT);      // Arduino控制A4988x方向引脚为输出模式
  
  
  pinMode(enPin,OUTPUT);   // Arduino控制A4988使能引脚为输出模式
  digitalWrite(enPin,LOW); // 将使能控制引脚设置为低电平从而让
                               // 电机驱动板进入工作状态
                                
  stepper1.setMaxSpeed(1000.0);     // 设置电机最大速度300 
  stepper1.setAcceleration(100.0);  // 设置电机加速度20.0  
  stepper1.setSpeed(1000);

  Serial.begin(9600);
  Serial.println("++++++++++++++++++++++++++++++++++");     
  Serial.println("+ SSC UNO CNC/A4988 Stepper Demo +");   
  Serial.println("+     www.taichi-maker.com       +");  
  Serial.println("++++++++++++++++++++++++++++++++++");  
  Serial.println("");  
  Serial.println("Please input motor command:");
}
 
void loop() {
  if (Serial.available()) {     // 检查串口缓存是否有数据等待传输 
    cmd = Serial.read();        // 获取电机指令中电机编号信息    
    Serial.print("cmd = ");
    Serial.print(cmd);    
    Serial.print(" , "); 
 
    data = Serial.parseInt();
    Serial.print("data = ");
    Serial.print(data);   
    Serial.println("");    
 
    runUsrCmd();
  }
} 
 
//此函数用于运行用户指令
void runUsrCmd(){
  switch(cmd){ 
    case 'x':    // 设置步进电机旋转(顺时针/逆时针)
      Serial.print("Set Rotation To "); 
      if (data == 0){
        digitalWrite(dirPin, 0);
        Serial.println("Counter Clockwise."); 
      } else {
        digitalWrite(dirPin, 1);
        Serial.println("Clockwise."); 
      }
      break;
    
    case 'g':   // 设置A4988 enable功能
      Serial.print("Set Motor To "); 
      if (data == 0){
        digitalWrite(enPin, 1);
        Serial.println("Disable."); 
      } else {
        digitalWrite(enPin, 0);
        Serial.println("Enable."); 
      }
      break;
      
//    case 'm':  // 设置A4988 sleep功能
//      Serial.print("Set Motor To "); 
//      if (data == 0){
//        digitalWrite(sleepPin, 0);
//        Serial.println("Sleep."); 
//      } else {
//        digitalWrite(sleepPin, 1);
//        Serial.println("Awake."); 
//      }
//      break;
// 
//    case 'b':   // 设置步进模式
//      if (data == 1 || data == 2  || data == 4  || data == 8 || data == 16){
//        Serial.print("Set Motor Step Control To "); 
//        setStepMode(data);
//      } else {
//        Serial.println("Wrong Step Mode Cmd!");
//      }
//      break;
 
    case 'z': // 设置步进电机运行步数
      runStepper(motorSpeed, data);
      break;
 
    case 'd': // 设置步进电机运行速度      
      motorSpeed = data;
      Serial.print("Set Motor Speed To ");
      Serial.println(data);
      break;
          
    default:  // 未知指令
      Serial.println("Unknown Command");
  }
}
 
//运行步进电机
void runStepper (int rotationSpeed, int stepNum){
  for(int x = 0; x < stepNum; x++) {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(rotationSpeed); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(rotationSpeed); 
  }  
}
 
 //设置步进模式
//void setStepMode(int modeNum){ 
//  switch(modeNum){ 
//    case 1:   // 全步进
//    digitalWrite(ms1Pin, LOW); 
//    digitalWrite(ms2Pin, LOW); 
//    digitalWrite(ms3Pin, LOW);  
//    Serial.println(F("Stepping Mode: Full"));
//    break; 
// 
//    case 2:  // 半步进  
//    digitalWrite(ms1Pin, HIGH); 
//    digitalWrite(ms2Pin, LOW); 
//    digitalWrite(ms3Pin, LOW);  
//    Serial.println(F("Stepping Mode: 1/2"));
//    break; 
// 
//    case 4:  // 1/4 步进   
//    digitalWrite(ms1Pin, LOW); 
//    digitalWrite(ms2Pin, HIGH); 
//    digitalWrite(ms3Pin, LOW);  
//    Serial.println(F("Stepping Mode: 1/4"));
//    break;     
// 
//    case 8:  // 1/8 步进   
//    digitalWrite(ms1Pin, HIGH); 
//    digitalWrite(ms2Pin, HIGH); 
//    digitalWrite(ms3Pin, LOW);  
//    Serial.println(F("Stepping Mode: 1/8"));
//    break;  
// 
//    case 16:  // 1/16 步进   
//    digitalWrite(ms1Pin, HIGH); 
//    digitalWrite(ms2Pin, HIGH); 
//    digitalWrite(ms3Pin, HIGH); 
//    Serial.println(F("Stepping Mode: 1/16")); 
//    break;    
//  }
//}
