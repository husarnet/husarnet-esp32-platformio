#pragma once
#include <Husarnet.h>
#include <Arduino.h>

#if __has_include(<WebServer.h>)
#define USE_WEBSERVER 1
#include <WebServer.h>
#endif

#if __has_include(<ESPAsyncWebServer.h>)
#define USE_ASYNCWEBSERVER 1
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#endif

extern const char STATIC_FAVICON_ICO[15422] PROGMEM;
extern const size_t STATIC_FAVICON_ICO_LEN;
extern const char STATIC_FAVICON_ICO_MIME[] PROGMEM;

extern const char STATIC_WATER_CSS_DARK_STANDALONE_MIN_CSS[7049] PROGMEM;
extern const size_t STATIC_WATER_CSS_DARK_STANDALONE_MIN_CSS_LEN;
extern const char STATIC_WATER_CSS_DARK_STANDALONE_MIN_CSS_MIME[] PROGMEM;

extern const char STATIC_ZOID_LICENSE_TXT[1468] PROGMEM;
extern const size_t STATIC_ZOID_LICENSE_TXT_LEN;
extern const char STATIC_ZOID_LICENSE_TXT_MIME[] PROGMEM;

extern const char STATIC_WATER_CSS_LICENSE_TXT[1083] PROGMEM;
extern const size_t STATIC_WATER_CSS_LICENSE_TXT_LEN;
extern const char STATIC_WATER_CSS_LICENSE_TXT_MIME[] PROGMEM;

extern const char STATIC_WATER_CSS_DARK_STANDALONE_MIN_CSS_MAP[12095] PROGMEM;
extern const size_t STATIC_WATER_CSS_DARK_STANDALONE_MIN_CSS_MAP_LEN;
extern const char STATIC_WATER_CSS_DARK_STANDALONE_MIN_CSS_MAP_MIME[] PROGMEM;

extern const char STATIC_ZOID_FRAME_MIN_JS_MAP[382383] PROGMEM;
extern const size_t STATIC_ZOID_FRAME_MIN_JS_MAP_LEN;
extern const char STATIC_ZOID_FRAME_MIN_JS_MAP_MIME[] PROGMEM;

extern const char STATIC_ZOID_FRAME_MIN_JS[72916] PROGMEM;
extern const size_t STATIC_ZOID_FRAME_MIN_JS_LEN;
extern const char STATIC_ZOID_FRAME_MIN_JS_MIME[] PROGMEM;

extern const char STATIC_INDEX_HTML[4404] PROGMEM;
extern const size_t STATIC_INDEX_HTML_LEN;
extern const char STATIC_INDEX_HTML_MIME[] PROGMEM;

extern const char STATIC_STREAM_HTML[256] PROGMEM;
extern const size_t STATIC_STREAM_HTML_LEN;
extern const char STATIC_STREAM_HTML_MIME[] PROGMEM;

extern const char STATIC_STREAM_LED_JS[329] PROGMEM;
extern const size_t STATIC_STREAM_LED_JS_LEN;
extern const char STATIC_STREAM_LED_JS_MIME[] PROGMEM;

extern const char STATIC_STREAM_LED_HTML[795] PROGMEM;
extern const size_t STATIC_STREAM_LED_HTML_LEN;
extern const char STATIC_STREAM_LED_HTML_MIME[] PROGMEM;

extern const char STATIC_STREAM_JS[318] PROGMEM;
extern const size_t STATIC_STREAM_JS_LEN;
extern const char STATIC_STREAM_JS_MIME[] PROGMEM;

#ifdef USE_WEBSERVER
void registerStaticWebServerHandlers(WebServer *server);
#endif

#ifdef USE_ASYNCWEBSERVER
void registerStaticAsyncWebServerHandlers(AsyncWebServer *server);
void registerDynamicAsyncWebServerHandlers(AsyncWebServer *server, AwsTemplateProcessor callback=nullptr);
#endif
