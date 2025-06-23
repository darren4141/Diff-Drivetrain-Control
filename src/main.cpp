#include <Arduino.h>
#include "driver/pcnt.h"
#include "motor.cpp"
#include "module.cpp"
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Bluepad32.h>

#define PCNT_UNIT_FROM_ID(ID) PCNT_UNIT_##ID

GamepadPtr myGamepad = nullptr;

// Motor DIR and EN pins (DIR1 / EN1 and DIR2 / EN2)
const int motorDirPins[6] = {25, 32, 5, 16, 15, 0};   // DIR1 or DIR2
const int motorEnPins[6]  = {26, 33, 18, 17, 2, 4};   // EN1 or EN2

// Encoder A and B phase pins (Pin 5 & 6 from each connector J1–J6)
const int encoderAPins[6] = {39, 35, 19, 22, 14, 12}; // Encoder A
const int encoderBPins[6] = {36, 34, 21, 23, 27, 13}; // Encoder B
const char* ssid = "Darren’s iPhone";
const char* password = "password";

AsyncWebServer server(80);

Motor motor[6] = {
  Motor(0, motorDirPins[0], motorEnPins[0], PCNT_UNIT_FROM_ID(0), encoderAPins[0], encoderBPins[0]),
  Motor(1, motorDirPins[1], motorEnPins[1], PCNT_UNIT_FROM_ID(1), encoderAPins[1], encoderBPins[1]),
  Motor(2, motorDirPins[2], motorEnPins[2], PCNT_UNIT_FROM_ID(2), encoderAPins[2], encoderBPins[2]),
  Motor(3, motorDirPins[3], motorEnPins[3], PCNT_UNIT_FROM_ID(3), encoderAPins[3], encoderBPins[3]),
  Motor(4, motorDirPins[4], motorEnPins[4], PCNT_UNIT_FROM_ID(4), encoderAPins[4], encoderBPins[4]),
  Motor(5, motorDirPins[5], motorEnPins[5], PCNT_UNIT_FROM_ID(5), encoderAPins[5], encoderBPins[5])
};

void setup() {
  Serial.begin(115200);

  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("\nConnected: ");
  // Serial.println(WiFi.localIP());

  // if (!SPIFFS.begin(true)) {
  //   Serial.println("SPIFFS mount failed");
  //   return;
  // }

  // server.serveStatic("/", SPIFFS, "/")
  //   .setDefaultFile("index.html")
  //   .setCacheControl("no-cache");

  // server.onNotFound([](AsyncWebServerRequest *request) {
  //   String uri = request->url();

  //   if (uri.startsWith("/motor/")) {
  //     int secondSlash = uri.indexOf('/', 7);
  //     if (secondSlash == -1) {
  //       request->send(400, "text/plain", "Bad motor path");
  //       return;
  //     }

  //     int id = uri.substring(7, secondSlash).toInt();
  //     if (id < 0 || id >= 6) {
  //       request->send(400, "text/plain", "Invalid motor ID");
  //       return;
  //     }

  //     String action = uri.substring(secondSlash + 1);

  //     if (action == "encoder") {
  //       request->send(200, "text/plain", String(motor[id].readEncoder()));
  //       return;
  //     }

  //     if (action == "pwm" && request->hasParam("value")) {
  //       int pwm = request->getParam("value")->value().toInt();
  //       motor[id].driveRaw(pwm, 1);  // For simplicity, dir 1 (adjust if needed)
  //       request->send(200, "text/plain", "OK");
  //       return;
  //     }

  //     if (action == "direction" && request->hasParam("value")) {
  //       int dir = request->getParam("value")->value().toInt();
  //       motor[id].driveRaw(255, dir);  // Apply direction, fixed speed 255 for now
  //       request->send(200, "text/plain", "OK");
  //       return;
  //     }

  //     if (action == "invert" && request->hasParam("value")) {
  //       int inv = request->getParam("value")->value().toInt();
  //       motor[id].setInvertDirection(inv);
  //       request->send(200, "text/plain", "OK");
  //       return;
  //     }

  //     request->send(404, "text/plain", "Action not found");
  //     return;
  //   }

  //   request->send(404, "text/plain", "Not found");
  // });

  // server.begin();

  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But it might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();

  // Enables mouse / touchpad support for gamepads that support them.
  // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
  // - First one: the gamepad
  // - Second one, which is a "virtual device", is a mouse.
  // By default, it is disabled.
  BP32.enableVirtualDevice(false);

}

void loop() {
  bool dataUpdated = BP32.update();
  
  if (dataUpdated)
      processControllers();


  

  delay(100);

}

