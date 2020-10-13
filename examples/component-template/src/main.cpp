#include <Husarnet.h>
#include <Arduino.h>
#include <WiFi.h>

#include <WebServer.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include <esp32cam.h>

#include "static_data.hpp"

// WiFi credentials
const char *ssid = "wifi-network";
const char *password = "wifi-password";

// Husarnet credentials
const char *hostName = "awesome-device";
const char *husarnetJoinCode = "fc94:b01d:1803:8dd8:b293:5c7d:7639:932a/xxxxxxxxxxxxxxxxxxxx";
const char *dashboardURL = "default";

// Setup webservers
// Right now we need two different webservers as WebServer is not fast enough to serve multiple
// static files at once but AsyncWebServer doesn't have support for MJPEG in the library we're using for the camera.
// Feel free to stick with AsyncWebServer if your usecase doesn't need the WebServer.
AsyncWebServer server(8000);
WebServer server2(8001);

// This is the pin used for the "flashlight" LED on the board. Change it to match your board!
const uint8_t ledPin = 16;

void handleMjpeg()
{
  // Configure stream
  static struct esp32cam::CameraClass::StreamMjpegConfig mjcfg;
  mjcfg.frameTimeout = 1000; // ms
  mjcfg.minInterval = 100;   // ms
  mjcfg.maxFrames = -1;      // -1 means - send frames until error occurs or client disconnects

  // Actually stream
  auto client = server2.client();

  auto startTime = millis();

  int res = esp32cam::Camera.streamMjpeg(client, mjcfg);
  if (res <= 0)
  {
    Serial.printf("Stream error: %d\n", res);
    return;
  }

  auto duration = millis() - startTime;
  Serial.printf("Stream end %d frames, on average %0.2f FPS\n", res, 1000.0 * res / duration);
}

// All files in the dynamic directory are going through the template processor of ESPAsyncWebServer
// https://github.com/me-no-dev/ESPAsyncWebServer#template-processing
// This function is the default processor for those. Feel free to change it if you need more variables.
String templateProcessor(const String &var)
{
  if (var == "SELF_HOSTNAME")
    return Husarnet.getHostname();
  if (var == "SELF_BASE_URL")
    return String("http://") + Husarnet.getHostname() + ":8000";
  if (var == "STATIC_BASE_URL")
    // In case you find your ESP to be too slow to serve static files - you can use a different server
    return String("http://") + Husarnet.getHostname() + ":8000";

  return String("%") + var + String("%");
}

void onNotFoundRequest(AsyncWebServerRequest *request)
{
  request->send(404);
}

// This method handles the special `/data.json` path. It returns things like the list of peers
// on the network to the JavaScript running component autodetection, and also the list of components
// that this node is requesting to be rendered. Feel free to change the list of components
// to fit your needs. There's also an option for changing the default ordering of components
// on the page, and option for changing ordering of peer tiles too. Feel free to configure them as well.
void onDataRequest(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  // We need to control CORS a little bit more. If you want, you can change it and make your devices
  // a little bit safer, but this is good enough for testing.
  response->addHeader("Access-Control-Allow-Origin", "*");

  DynamicJsonDocument doc(1024); // Be generous here. You'd be surprised how large JSON files can be.

  doc["hostname"] = Husarnet.getHostname();
  doc["order"] = 1; // Feel free to change it

  // This is the declaration of components
  auto components = doc.createNestedArray("components");

  auto stream_component = components.createNestedObject();
  stream_component["script"] = "stream.js"; // This field contains the URL of the component
  stream_component["order"] = 1;            // Feel free to change the ordering too
  stream_component["slug"] = "stream";      // This is the name that your component uses while
                                            // registering to `window.components`. Make sure it's
                                            // consistent here and in the .js file

  auto stream_led_component = components.createNestedObject();
  stream_led_component["script"] = "stream-led.js";
  stream_led_component["order"] = 2;
  stream_led_component["slug"] = "stream-led";

  // This is the list of other peers.
  auto peers = doc.createNestedArray("peers");

  for (auto const &host : Husarnet.listPeers())
  {
    auto peer = peers.createNestedObject();
    peer["address"] = host.first.toString();
    peer["hostname"] = host.second;
  }

  serializeJson(doc, *response);
  request->send(response);
}

void togglePin(uint8_t number)
{
  digitalWrite(number, !digitalRead(number));
}

void setup()
{
  // Setup serial
  Serial.begin(115200);
  Serial.println();

  // Configure some pins
  pinMode(ledPin, OUTPUT);

  // Configure camera
  esp32cam::Config cfg;

  // M5 ESP32 camera
  cfg.setPins(esp32cam::Pins{
    D0 : 17,
    D1 : 35,
    D2 : 34,
    D3 : 5,
    D4 : 39,
    D5 : 18,
    D6 : 36,
    D7 : 19,
    XCLK : 27,
    PCLK : 21,
    VSYNC : 22,
    HREF : 26,
    SDA : 25,
    SCL : 23,
    RESET : 15,
    PWDN : -1,
  });

  cfg.setResolution(esp32cam::Resolution::find(320, 240));
  cfg.setBufferCount(2);
  cfg.setJpeg(80);

  bool ok = esp32cam::Camera.begin(cfg);
  if (!ok)
  {
    Serial.println("Camera initialization failed");
  }

  // Connect to Wi-Fi
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("done!\r\nLocal IP: %s\r\n", WiFi.localIP().toString().c_str());

  // Start Husarnet
  Husarnet.selfHostedSetup(dashboardURL);
  Husarnet.join(husarnetJoinCode, hostName);
  Husarnet.start();

  // Setup the main webserver
  registerStaticAsyncWebServerHandlers(&server);
  registerDynamicAsyncWebServerHandlers(&server, templateProcessor);

  // This is the handler for LED component button. Feel free to remove it if you don't need it
  server.on("/stream_led", [](AsyncWebServerRequest *request) {
    togglePin(ledPin);
    request->send(200, "text/plain", "toggled!");
  });

  // Those are the handlers for the framework
  server.on("/", [](AsyncWebServerRequest *request) {
    request->redirect("/index.html");
  });
  server.on("/data.json", onDataRequest);
  server.onNotFound(onNotFoundRequest);

  server.begin();

  // Setup the stream webserver
  server2.on("/stream", handleMjpeg);
  server2.begin();

  // Output some user friendly data
  Serial.printf("Local URL:    http://%s:%d/\r\n", WiFi.localIP().toString().c_str(), 8000);
  Serial.printf("Husarnet URL: http://%s:%d/\r\n", hostName, 8000);
}

void loop()
{
  server2.handleClient();
  delay(10);
}