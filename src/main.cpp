#include <Arduino.h>

// Motor DIR and EN pins (DIR1 / EN1 and DIR2 / EN2)
const int motorDirPins[6] = {25, 32, 5, 16, 15, 0};   // DIR1 or DIR2
const int motorEnPins[6]  = {26, 33, 18, 17, 2, 4};   // EN1 or EN2

// Encoder A and B phase pins (Pin 5 & 6 from each connector J1–J6)
// const int encoderAPins[6] = {39, 35, 19, 22, 14, 12}; // Encoder A
// const int encoderBPins[6] = {36, 34, 21, 23, 27, 13}; // Encoder B

// PWM configuration
const int pwmFreq = 20000;       // 20 kHz
const int pwmResolution = 8;     // 8-bit resolution (0-255)
const int pwmDuty = 255;         // ~70% duty cycle

void setup() {
  // for (int i = 0; i < 6; i++) {
  //   pinMode(motorDirPins[i], OUTPUT);
  //   // pinMode(encoderAPins[i], INPUT);
  //   // pinMode(encoderBPins[i], INPUT);

  //   ledcSetup(i, pwmFreq, pwmResolution);
  //   ledcAttachPin(motorEnPins[i], i);
  // }

  for(int i = 0; i < 6; i++){
    pinMode(motorDirPins[i], OUTPUT);
    pinMode(motorEnPins[i], OUTPUT);
  }
}

void loop() {
  for(int i = 0; i < 6; i++){
    digitalWrite(motorEnPins[i], HIGH);
  }
  delay(2000);
  for(int i = 0; i < 6; i++){
    digitalWrite(motorDirPins[i], HIGH);
  }  
  delay(2000);

  for(int i = 0; i < 6; i++){
    digitalWrite(motorDirPins[i], LOW);
  }  
  delay(2000);
  // Clockwise (DIR = LOW)
  // for (int i = 0; i < 6; i++) {
  //   digitalWrite(motorDirPins[i], LOW);
  //   ledcWrite(i, pwmDuty);
  // }
  // delay(2000);

  // // Counterclockwise (DIR = HIGH)
  // for (int i = 0; i < 6; i++) {
  //   digitalWrite(motorDirPins[i], HIGH);
  //   ledcWrite(i, pwmDuty);
  // }
  // delay(2000);

  // // Stop all motors
  // for (int i = 0; i < 6; i++) {
  //   ledcWrite(i, 0);
  // }
  // delay(1000);
}
