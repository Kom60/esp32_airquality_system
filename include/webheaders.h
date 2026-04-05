#include <WiFi.h>              // needed to connect to WiFi
#include <HTTPClient.h>        // needed for HTTP client requests
#include <ESPAsyncWebServer.h> // needed to create a simple webserver
#include <WebSocketsServer.h>  // needed for instant communication between client and server through Websockets
#include <ArduinoJson.h>       // needed for JSON encapsulation
#include "esp32/clk.h"         // for esp_clk_cpu_freq()

// global variables of the LED selected and the intensity of that LED
extern int random_intensity;

extern const int ARRAY_LENGTH;
extern float sens_vals[];

extern AsyncWebServer server;                         // the server uses port 80 (standard port for websites
extern WebSocketsServer webSocket; // the websocket uses port 81 (standard port for websockets*/

void web_setup();
void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length);
void sendJson(String l_type, String l_value);
void sendJsonArray(String l_type, float l_array_values[]);

// Обработчики API настроек
void handleGetSettings(AsyncWebServerRequest *request);
void handleSaveSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
void handleResetSettings(AsyncWebServerRequest *request);
void handleReboot(AsyncWebServerRequest *request);

// Обработчики API конфигурации (JSON config)
void handleGetConfig(AsyncWebServerRequest *request);
void handleSaveConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len);
void handleDeleteConfig(AsyncWebServerRequest *request);
void handleBackupConfig(AsyncWebServerRequest *request);
void handleRestoreConfig(AsyncWebServerRequest *request);

// Отправка данных на ПК
void send_data_to_pc();

// Обработчики API обновления прошивки
void handleGetFirmwareStatus(AsyncWebServerRequest *request);
void handleUploadFirmware(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
void handleFlashFirmware(AsyncWebServerRequest *request);
void handleDeleteFirmware(AsyncWebServerRequest *request);
