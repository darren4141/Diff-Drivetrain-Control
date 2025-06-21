#include <Arduino.h>
#include "driver/pcnt.h"
#include "motor.cpp"
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>

#define PCNT_UNIT_FROM_ID(ID) PCNT_UNIT_##ID

// Motor DIR and EN pins (DIR1 / EN1 and DIR2 / EN2)
const int motorDirPins[6] = {25, 32, 5, 16, 15, 0};   // DIR1 or DIR2
const int motorEnPins[6]  = {26, 33, 18, 17, 2, 4};   // EN1 or EN2

// Encoder A and B phase pins (Pin 5 & 6 from each connector J1–J6)
const int encoderAPins[6] = {39, 35, 19, 22, 14, 12}; // Encoder A
const int encoderBPins[6] = {36, 34, 21, 23, 27, 13}; // Encoder B