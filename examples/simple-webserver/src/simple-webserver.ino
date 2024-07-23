#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <husarnet.h>

// WiFi credentials
#define WIFI_SSID = "wifi-network";
#define char* WIFI_PASS = "wifi-password";

// Husarnet credentials
#define HOSTNAME = "esp32-pio-webserver";
#define JOIN_CODE = "xxxxxxxxxxxxxxxxxxxx";

HusarnetClient husarnet;
WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "Hello Husarnet!");
}

void setup() {
  Serial.begin(115200);

  // Connect to the WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.println("Connecting to WiFi");
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi connection failure (err: %d)\n", WiFi.status());
    delay(5000);
    ESP.restart();
  }

  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  // Join the Husarnet network
  husarnet.join(HOSTNAME, JOIN_CODE);

  while(!husarnet.isJoined()) {
    Serial.println("Waiting for Husarnet network...");
    delay(1000);
  }
  Serial.println("Husarnet network joined");

  Serial.print("Husarnet IP: ");
  Serial.println(husarnet.getIpAddress().c_str());

  // Start the web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle incoming connections
  server.handleClient();
  delay(2);
}
