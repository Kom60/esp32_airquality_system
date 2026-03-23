#include "webheaders.h"
#include "headers.h"
#include "settings.h"
#include "secrets.h"

// global variables of the LED selected and the intensity of that LED
int random_intensity = 5;

const int ARRAY_LENGTH=10;
float sens_vals[ARRAY_LENGTH];

AsyncWebServer server(80);                         // the server uses port 80 (standard port for this website
WebSocketsServer webSocket = WebSocketsServer(81); // the websocket uses port 81

// Инициализация веб-сервера и WebSocket
void web_setup() {
  // Маршруты веб-сервера
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/settings.html", "text/html");
  });

  // API для настроек
  server.on("/api/settings", HTTP_GET, handleGetSettings);

  AsyncCallbackWebHandler* settingsPostHandler = new AsyncCallbackWebHandler();
  settingsPostHandler->setUri("/api/settings");
  settingsPostHandler->setMethod(HTTP_POST);
  settingsPostHandler->onBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    handleSaveSettings(request, data, len);
  });
  settingsPostHandler->onRequest([](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });
  server.addHandler(settingsPostHandler);

  server.on("/api/settings/reset", HTTP_POST, handleResetSettings);
  server.on("/api/reboot", HTTP_POST, handleReboot);

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "File not found");
  });

  server.serveStatic("/", SPIFFS, "/");

  // Инициализация WebSocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Запуск сервера
  server.begin();
}

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    LOG_INFO(WEBSOCKET, "Client disconnected");
    break;
  case WStype_CONNECTED:
    LOG_INFO_FMT(WEBSOCKET, "Client %d connected", num);
    LOG_INFO(WEBSOCKET, "Sending initial data...");

    // send variables to newly connected web client (используем скользящее среднее)
    sendJson("bme_temperature", String(meteo_buffer.get_avg_bme_temperature()));
    sendJson("bme_pressure", String(meteo_buffer.get_avg_bme_pressure()));
    sendJson("bme_humidity", String(meteo_buffer.get_avg_bme_humidity()));
    sendJson("htu_temperature", String(meteo_buffer.get_avg_htu_temperature()));
    sendJson("htu_humidity", String(meteo_buffer.get_avg_htu_humidity()));
    sendJson("scd4x_co2", String((int)meteo_buffer.get_avg_scd4x_co2()));
    sendJson("scd4x_temperature", String(meteo_buffer.get_avg_scd4x_temperature()));
    sendJson("scd4x_humidity", String(meteo_buffer.get_avg_scd4x_humidity()));
    sendJson("pms_pm1", String(meteo_buffer.get_avg_pms_pm1()));
    sendJson("pms_pm2_5", String(meteo_buffer.get_avg_pms_pm2_5()));
    sendJson("pms_pm10", String(meteo_buffer.get_avg_pms_pm10()));
    sendJson("ms5611_pressure", String(meteo_buffer.get_avg_ms5611_pressure()));
    sendJson("ms5611_temperature", String(meteo_buffer.get_avg_ms5611_temperature()));
    sendJson("bh1750_lighting", String(meteo_buffer.get_avg_bh1750_lighting()));
    sendJson("veml_uv", String(meteo_buffer.get_avg_veml_uv()));
    sendJson("ch2o_value", String(meteo_buffer.get_avg_ch2o_value()));
    sendJson("microphone_noise", String(meteo_buffer.get_ema_microphone_noise()));
    sendJson("ina226_voltage", String(meteo_buffer.get_avg_ina226_voltage()));
    sendJson("ina226_current", String(meteo_buffer.get_avg_ina226_current()));
    sendJson("ina226_power", String(meteo_buffer.get_avg_ina226_power()));
    sendJson("esp32_cpu_freq", String(esp_clk_cpu_freq()));
    sendJson("esp32_cpu_temp", String(temperatureRead()));
    sendJson("esp32_free_heap", String(ESP.getFreeHeap()));
    sendJson("wifi_rssi", String(WiFi.RSSI()));

    LOG_INFO(WEBSOCKET, "Initial data sent!");
    break;
  case WStype_TEXT:
    // try to decipher the JSON string received
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      LOG_ERROR_FMT(WEBSOCKET, "deserializeJson() failed: %s", error.f_str());
      return;
    }
    else
    {
      // JSON string was received correctly, so information can be retrieved:
      const char *l_type = doc["type"];
      const int l_value = doc["value"];
      LOG_DEBUG_FMT(WEBSOCKET, "Type: %s, Value: %d", l_type, l_value);

      // if random_intensity value is received -> update and write to all web clients
      if (String(l_type) == "random_intensity")
      {
        random_intensity = int(l_value);
        sendJson("random_intensity", String(l_value));
      }
    }
    break;
  }
}

// Simple function to send information to the web clients
void sendJson(String l_type, String l_value)
{
  String jsonString = "";
  StaticJsonDocument<200> doc;
  JsonObject object = doc.to<JsonObject>();
  object["type"] = l_type;
  object["value"] = l_value;
  serializeJson(doc, jsonString);

  // Отладка
  //Serial.print("[WS] Sending: ");
  //Serial.println(jsonString);

  webSocket.broadcastTXT(jsonString);
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

// =====================================================
// Отправка данных на ПК
// =====================================================

// Форматирование float значения для JSON
static String format_float(float value, int decimals = 1)
{
  if (!is_valid_float(value))
    value = 0.0;
  char buf[16];
  dtostrf(value, 1, decimals, buf);
  return String(buf);
}

// Отправка данных на ПК (используем скользящее среднее)
void send_data_to_pc()
{
  HTTPClient http;

  String url = "http://" + String(PC_IP) + ":" + String(PC_PORT) + "/api/data";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Формируем ВАЛИДНЫЙ JSON с использованием скользящего среднего
  String json = "{";

  // BME280 sensor data (скользящее среднее)
  json += "\"bme_temperature\":" + format_float(meteo_buffer.get_avg_bme_temperature()) + ",";
  json += "\"bme_pressure\":" + format_float(meteo_buffer.get_avg_bme_pressure()) + ",";
  json += "\"bme_humidity\":" + format_float(meteo_buffer.get_avg_bme_humidity()) + ",";

  // HTU21DF sensor data (скользящее среднее)
  json += "\"htu_temperature\":" + format_float(meteo_buffer.get_avg_htu_temperature()) + ",";
  json += "\"htu_humidity\":" + format_float(meteo_buffer.get_avg_htu_humidity()) + ",";

  // SCD4X sensor data (скользящее среднее)
  json += "\"scd4x_co2\":" + String((int)meteo_buffer.get_avg_scd4x_co2()) + ",";
  json += "\"scd4x_temperature\":" + format_float(meteo_buffer.get_avg_scd4x_temperature()) + ",";
  json += "\"scd4x_humidity\":" + format_float(meteo_buffer.get_avg_scd4x_humidity()) + ",";

  // PMS sensor data (скользящее среднее)
  json += "\"pms_pm1\":" + String(meteo_buffer.get_avg_pms_pm1()) + ",";
  json += "\"pms_pm2_5\":" + String(meteo_buffer.get_avg_pms_pm2_5()) + ",";
  json += "\"pms_pm10\":" + String(meteo_buffer.get_avg_pms_pm10()) + ",";

  // MS5611 sensor data (скользящее среднее)
  json += "\"ms5611_pressure\":" + format_float(meteo_buffer.get_avg_ms5611_pressure()) + ",";
  json += "\"ms5611_temperature\":" + format_float(meteo_buffer.get_avg_ms5611_temperature()) + ",";


  // BH1750 sensor data (скользящее среднее)
  json += "\"bh1750_lighting\":" + format_float(meteo_buffer.get_avg_bh1750_lighting()) + ",";

  // VEML6070 sensor data (скользящее среднее)
  json += "\"veml_uv\":" + String(meteo_buffer.get_avg_veml_uv()) + ",";

  // CH2O sensor data (скользящее среднее)
  json += "\"ch2o_value\":" + format_float(meteo_buffer.get_avg_ch2o_value(), 3) + ",";

  // Microphone data (EMA)
  json += "\"microphone_noise\":" + format_float(meteo_buffer.get_ema_microphone_noise()) + ",";

  // INA226 sensor data (скользящее среднее)
  json += "\"ina226_voltage\":" + format_float(meteo_buffer.get_avg_ina226_voltage(), 3) + ",";
  json += "\"ina226_current\":" + format_float(meteo_buffer.get_avg_ina226_current(), 3) + ",";
  json += "\"ina226_power\":" + format_float(meteo_buffer.get_avg_ina226_power(), 3);

  json += "}";

  // Отладка: выводим JSON в Serial
  LOG_DEBUG_FMT(NETWORK, "Отправка: %s", json.c_str());

  int httpResponseCode = http.POST(json);
  // yield();
  if (httpResponseCode == 200)
  {
    LOG_INFO(NETWORK, "Данные приняты сервером");
  }
  else
  {
    LOG_ERROR_FMT(NETWORK, "Ошибка отправки. Код: %d", httpResponseCode);
    // Доп. отладка при ошибке
    if (httpResponseCode > 0)
    {
      String payload = http.getString();
      LOG_DEBUG_FMT(NETWORK, "Ответ сервера: %s", payload.c_str());
    }
  }

  http.end();
}
