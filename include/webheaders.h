#include <WiFi.h>              // needed to connect to WiFi
#include <ESPAsyncWebServer.h> // needed to create a simple webserver (make sure tools -> board is set to ESP32, otherwise you will get a "WebServer.h: No such file or directory" error)
#include <WebSocketsServer.h>  // needed for instant communication between client and server through Websockets
#include <ArduinoJson.h>       // needed for JSON encapsulation (send multiple variables with one string)

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length);
void sendJson(String l_type, String l_value);
void sendJsonArray(String l_type, float l_array_values[]);