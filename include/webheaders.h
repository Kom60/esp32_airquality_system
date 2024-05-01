#include <WiFi.h>              // needed to connect to WiFi
#include <ESPAsyncWebServer.h> // needed to create a simple webserver (make sure tools -> board is set to ESP32, otherwise you will get a "WebServer.h: No such file or directory" error)
#include <WebSocketsServer.h>  // needed for instant communication between client and server through Websockets
#include <ArduinoJson.h>       // needed for JSON encapsulation (send multiple variables with one string)

// global variables of the LED selected and the intensity of that LED
extern int random_intensity;

extern const int ARRAY_LENGTH;
extern float sens_vals[];

extern AsyncWebServer server;                         // the server uses port 80 (standard port for websites
extern WebSocketsServer webSocket; // the websocket uses port 81 (standard port for websockets*/

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length);
void sendJson(String l_type, String l_value);
void sendJsonArray(String l_type, float l_array_values[]);