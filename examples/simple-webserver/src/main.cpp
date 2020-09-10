#include "Arduino.h"
#include <WiFi.h>
#include <Husarnet.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid     = "wifi-network";
const char* password = "wifi-password";

// Husarnet credentials
const char* hostName = "awesome-device";
const char* husarnetJoinCode = "fc94:b01d:1803:8dd8:b293:5c7d:7639:932a/xxxxxxxxxxxxxxxxxxxx";
const char* dashboardURL = "default";

// Setup http server
WebServer server(80);

void onHttpReqFunc() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", "Hello World!");
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("done!\r\nLocal IP: %s\r\n", WiFi.localIP().toString().c_str());

  // Start Husarnet
  Husarnet.selfHostedSetup(dashboardURL);
  Husarnet.join(husarnetJoinCode, hostName);
  Husarnet.start();

  // Configure http server
  server.on("/", HTTP_GET, onHttpReqFunc);
  server.begin();
}

void loop() {
  // Handle connections
  server.handleClient();
}
