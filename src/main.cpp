#include "main.h"

WebServer server(80);
const char* ssid = "Darren’s iPhone";
const char* password = "password";


Motor motor[6] = {
  Motor(0, motorDirPins[0], motorEnPins[0], PCNT_UNIT_FROM_ID(0), encoderAPins[0], encoderBPins[0]),
  Motor(1, motorDirPins[1], motorEnPins[1], PCNT_UNIT_FROM_ID(1), encoderAPins[1], encoderBPins[1]),
  Motor(2, motorDirPins[2], motorEnPins[2], PCNT_UNIT_FROM_ID(2), encoderAPins[2], encoderBPins[2]),
  Motor(3, motorDirPins[3], motorEnPins[3], PCNT_UNIT_FROM_ID(3), encoderAPins[3], encoderBPins[3]),
  Motor(4, motorDirPins[4], motorEnPins[4], PCNT_UNIT_FROM_ID(4), encoderAPins[4], encoderBPins[4]),
  Motor(5, motorDirPins[5], motorEnPins[5], PCNT_UNIT_FROM_ID(5), encoderAPins[5], encoderBPins[5])
};

void serveFile(String path, String type) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, type);
  file.close();
}

void setupRoutes() {
  server.on("/", HTTP_GET, []() {
    serveFile("/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, []() {
    serveFile("/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, []() {
    serveFile("/script.js", "application/javascript");
  });

  // Dynamic motor endpoints
  server.onNotFound([]() {
    String uri = server.uri();

    // Match /motor/{id}/encoder
    if (uri.startsWith("/motor/") && uri.endsWith("/encoder")) {
      int id = uri.substring(7, uri.indexOf("/encoder")).toInt();
      if (id >= 0 && id < 6) {
        server.send(200, "text/plain", String(motor[id].readEncoder()));
        return;
      }
    }

    // Match /motor/{id}/pwm?value=X
    if (uri.startsWith("/motor/") && uri.indexOf("/pwm") > 0) {
      int id = uri.substring(7, uri.indexOf("/pwm")).toInt();
      if (id >= 0 && id < 6 && server.hasArg("value")) {
        int pwm = server.arg("value").toInt();
        motor[id].driveRaw(pwm, 1); // direction set separately
        server.send(200, "text/plain", "OK");
        return;
      }
    }

    // Match /motor/{id}/direction?value=X
    if (uri.startsWith("/motor/") && uri.indexOf("/direction") > 0) {
      int id = uri.substring(7, uri.indexOf("/direction")).toInt();
      if (id >= 0 && id < 6 && server.hasArg("value")) {
        int dir = server.arg("value").toInt();
        motor[id].driveRaw(255, dir);  // Use last PWM or always full for now
        server.send(200, "text/plain", "OK");
        return;
      }
    }

    // Match /motor/{id}/invert?value=X
    if (uri.startsWith("/motor/") && uri.indexOf("/invert") > 0) {
      int id = uri.substring(7, uri.indexOf("/invert")).toInt();
      if (id >= 0 && id < 6 && server.hasArg("value")) {
        int inv = server.arg("value").toInt();
        motor[id].setInvertDirection(inv);
        server.send(200, "text/plain", "OK");
        return;
      }
    }

    server.send(404, "text/plain", "Not found");
  });
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    return;
  }

  for (int i = 0; i < 6; i++) {
    motor[i].setupEncoder();
  }

  setupRoutes();
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

