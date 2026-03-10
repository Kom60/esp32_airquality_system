#include "webheaders.h"
#include "headers.h"
#include "settings.h"

// global variables of the LED selected and the intensity of that LED
int random_intensity = 5;

const int ARRAY_LENGTH=10;
float sens_vals[ARRAY_LENGTH];

AsyncWebServer server(80);                         // the server uses port 80 (standard port for websites
WebSocketsServer webSocket = WebSocketsServer(81); // the websocket uses port 81 (standard port for websockets*/

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{ 
  switch (type)
  {                         
  case WStype_DISCONNECTED:
    Serial.println("Client " + String(num) + " disconnected");
    break;
  case WStype_CONNECTED:
    Serial.println("Client " + String(num) + " connected");

    // send variables to newly connected web client (в оригинальных единицах, без масштабирования)
    // BME280 sensor data
    sendJson("bme_temperature", String(AIR_data.bme_temperature));
    sendJson("bme_pressure", String(AIR_data.bme_pressure));
    sendJson("bme_humidity", String(AIR_data.bme_humidity));

    // HTU21DF sensor data
    sendJson("htu_temperature", String(AIR_data.htu_temperature));
    sendJson("htu_humidity", String(AIR_data.htu_humidity));

    // SCD4X sensor data
    sendJson("scd4x_co2", String(AIR_data.scd4x_co2));
    sendJson("scd4x_temperature", String(AIR_data.scd4x_temperature));
    sendJson("scd4x_humidity", String(AIR_data.scd4x_humidity));

    // PMS sensor data
    sendJson("pms_pm1", String(AIR_data.pms_pm1));
    sendJson("pms_pm2_5", String(AIR_data.pms_pm2_5));
    sendJson("pms_pm10", String(AIR_data.pms_pm10));

    // MS5611 sensor data
    sendJson("ms5611_pressure", String(AIR_data.ms5611_pressure));
    sendJson("ms5611_temperature", String(AIR_data.ms5611_temperature));

    // BH1750 sensor data
    sendJson("bh1750_lighting", String(AIR_data.bh1750_lighting));

    // VEML6070 sensor data
    sendJson("veml_uv", String(AIR_data.veml_uv));

    // CH2O sensor data
    sendJson("ch2o_value", String(AIR_data.ch2o_value));

    // Microphone data
    sendJson("microphone_noise", String(AIR_data.microphone_noise));

    // ESP32 system data
    sendJson("esp32_cpu_freq", String(esp_clk_cpu_freq()));
    sendJson("esp32_cpu_temp", String(temperatureRead()));
    sendJson("esp32_free_heap", String(ESP.getFreeHeap()));
    sendJson("wifi_rssi", String(WiFi.RSSI()));

    break;
  case WStype_TEXT:
    // try to decipher the JSON string received
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    else
    {
      // JSON string was received correctly, so information can be retrieved:
      const char *l_type = doc["type"];
      const int l_value = doc["value"];
      Serial.println("Type: " + String(l_type));
      Serial.println("Value: " + String(l_value));

      // if random_intensity value is received -> update and write to all web clients
      if (String(l_type) == "random_intensity")
      {
        random_intensity = int(l_value);
        sendJson("random_intensity", String(l_value));
      }
    }
    Serial.println("");
    break;
  }
}

// Simple function to send information to the web clients
void sendJson(String l_type, String l_value)
{
  String jsonString = "";                   // create a JSON string for sending data to the client
  StaticJsonDocument<200> doc;              // create JSON container
  JsonObject object = doc.to<JsonObject>(); // create a JSON Object
  object["type"] = l_type;                  // write data into the JSON object
  object["value"] = l_value;
  serializeJson(doc, jsonString);     // convert JSON object to string
  webSocket.broadcastTXT(jsonString); // send JSON string to all clients
}

// =====================================================
// API для настроек
// =====================================================

// Получить настройки (JSON)
void handleGetSettings(AsyncWebServerRequest *request) {
  String json = "{";
  json += "\"wifi_ssid\":\"" + String(settings.wifi_ssid) + "\",";
  json += "\"wifi_password\":\"" + String(settings.wifi_password) + "\",";
  json += "\"update_interval\":" + String(settings.update_interval) + ",";
  json += "\"temp_offset_bme\":" + String(settings.temp_offset_bme) + ",";
  json += "\"temp_offset_htu\":" + String(settings.temp_offset_htu) + ",";
  json += "\"temp_offset_scd\":" + String(settings.temp_offset_scd) + ",";
  json += "\"hum_offset_bme\":" + String(settings.hum_offset_bme) + ",";
  json += "\"hum_offset_htu\":" + String(settings.hum_offset_htu) + ",";
  json += "\"press_offset_bme\":" + String(settings.press_offset_bme) + ",";
  json += "\"press_offset_ms\":" + String(settings.press_offset_ms) + ",";
  json += "\"co2_warning\":" + String(settings.co2_warning) + ",";
  json += "\"co2_critical\":" + String(settings.co2_critical) + ",";
  json += "\"pm25_warning\":" + String(settings.pm25_warning) + ",";
  json += "\"pm25_critical\":" + String(settings.pm25_critical) + ",";
  json += "\"night_mode_start\":" + String(settings.night_mode_start) + ",";
  json += "\"night_mode_end\":" + String(settings.night_mode_end);
  json += "}";
  request->send(200, "application/json", json);
}

// Сохранить настройки (JSON POST)
void handleSaveSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  if (len == 0) {
    request->send(400, "application/json", "{\"error\":\"No data\"}");
    return;
  }
  
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, data, len);
  
  if (error) {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  // WiFi настройки
  const char* ssid = doc["wifi_ssid"];
  const char* password = doc["wifi_password"];
  if (ssid && strlen(ssid) > 0 && strlen(ssid) <= 32) {
    strncpy(settings.wifi_ssid, ssid, 32);
    settings.wifi_ssid[32] = '\0';
  }
  if (password && strlen(password) > 0 && strlen(password) <= 64) {
    strncpy(settings.wifi_password, password, 64);
    settings.wifi_password[64] = '\0';
  }
  
  // Сохраняем остальные настройки
  settings.update_interval = doc["update_interval"] | 10;
  settings.temp_offset_bme = doc["temp_offset_bme"] | 0.0;
  settings.temp_offset_htu = doc["temp_offset_htu"] | 0.0;
  settings.temp_offset_scd = doc["temp_offset_scd"] | 0.0;
  settings.hum_offset_bme = doc["hum_offset_bme"] | 0;
  settings.hum_offset_htu = doc["hum_offset_htu"] | 0;
  settings.press_offset_bme = doc["press_offset_bme"] | 0;
  settings.press_offset_ms = doc["press_offset_ms"] | 0;
  settings.co2_warning = doc["co2_warning"] | 1000;
  settings.co2_critical = doc["co2_critical"] | 1400;
  settings.pm25_warning = doc["pm25_warning"] | 35;
  settings.pm25_critical = doc["pm25_critical"] | 50;
  settings.night_mode_start = doc["night_mode_start"] | 23;
  settings.night_mode_end = doc["night_mode_end"] | 7;
  
  settings_save();
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

// Сброс настроек
void handleResetSettings(AsyncWebServerRequest *request) {
  settings_reset();
  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

// Перезагрузка ESP32
void handleReboot(AsyncWebServerRequest *request) {
  request->send(200, "application/json", "{\"status\":\"ok\"}");
  delay(1000);
  ESP.restart();
}