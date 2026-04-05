#include "webheaders.h"
#include "headers.h"
#include "settings.h"
#include "secrets.h"
#include "sdcard.h"
#include <Update.h>

// global variables of the LED selected and the intensity of that LED
int random_intensity = 5;

const int ARRAY_LENGTH=10;
float sens_vals[ARRAY_LENGTH];

AsyncWebServer server(80);                         // the server uses port 80 (standard port for this website
WebSocketsServer webSocket = WebSocketsServer(81); // the websocket uses port 81

// Инициализация веб-сервера и WebSocket
void web_setup() {
  // Маршруты веб-сервера - файлы теперь на SD карте
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (SD.exists("/www/index.html")) {
      request->send(SD, "/www/index.html", "text/html");
    } else {
      request->send(404, "text/plain", "index.html not found on SD card");
    }
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (SD.exists("/www/settings.html")) {
      request->send(SD, "/www/settings.html", "text/html");
    } else {
      request->send(404, "text/plain", "settings.html not found on SD card");
    }
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (SD.exists("/www/config.html")) {
      request->send(SD, "/www/config.html", "text/html");
    } else {
      request->send(404, "text/plain", "config.html not found on SD card");
    }
  });
  
  server.on("/charts.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (SD.exists("/www/charts.html")) {
      request->send(SD, "/www/charts.html", "text/html");
    } else {
      request->send(404, "text/plain", "charts.html not found on SD card");
    }
  });

  // API для настроек (обратная совместимость)
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

  // API для JSON конфигурации (новые эндпоинты)
  server.on("/api/config", HTTP_GET, handleGetConfig);
  
  AsyncCallbackWebHandler* configPostHandler = new AsyncCallbackWebHandler();
  configPostHandler->setUri("/api/config");
  configPostHandler->setMethod(HTTP_POST);
  configPostHandler->onBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    handleSaveConfig(request, data, len);
  });
  configPostHandler->onRequest([](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });
  server.addHandler(configPostHandler);
  
  server.on("/api/config", HTTP_DELETE, handleDeleteConfig);
  server.on("/api/config/backup", HTTP_POST, handleBackupConfig);
  server.on("/api/config/restore", HTTP_POST, handleRestoreConfig);

  // API для обновления прошивки
  server.on("/api/firmware/status", HTTP_GET, handleGetFirmwareStatus);
  server.on("/api/firmware/flash", HTTP_POST, handleFlashFirmware);
  server.on("/api/firmware/delete", HTTP_DELETE, handleDeleteFirmware);
  
  // Загрузка файла прошивки
  AsyncCallbackWebHandler* firmwareUploadHandler = new AsyncCallbackWebHandler();
  firmwareUploadHandler->setUri("/api/firmware/upload");
  firmwareUploadHandler->setMethod(HTTP_POST);
  firmwareUploadHandler->onUpload(handleUploadFirmware);
  firmwareUploadHandler->onRequest([](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });
  server.addHandler(firmwareUploadHandler);

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "File not found");
  });

  // Раздаем статические файлы (CSS, JS, изображения) из /www на SD карте
  // Обработчик для CSS файлов
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/style.css", "text/css");
  });
  server.on("/main_style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/main_style.css", "text/css");
  });
  server.on("/header_style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/header_style.css", "text/css");
  });
  server.on("/dungeon.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/dungeon.css", "text/css");
  });
  server.on("/charts_style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/charts_style.css", "text/css");
  });
  
  // Обработчик для JS файлов
  server.on("/logic.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/logic.js", "application/javascript");
  });
  server.on("/js_logic.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/js_logic.js", "application/javascript");
  });
  server.on("/settings_logic.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/settings_logic.js", "application/javascript");
  });
  server.on("/charts_logic.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/charts_logic.js", "application/javascript");
  });
  
  // Обработчик для изображений и других файлов
  server.on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/favicon.png", "image/png");
  });
  server.on("/esp32_logo.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/esp32_logo.png", "image/png");
  });
  server.on("/on_bubl.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/on_bubl.png", "image/png");
  });
  server.on("/off_bubl.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/off_bubl.png", "image/png");
  });
  server.on("/manifest.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/www/manifest.json", "application/json");
  });

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
    JsonDocument doc;
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
  JsonDocument doc;
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

  JsonDocument doc;
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

// ============================================================================
// API для работы с JSON конфигурацией
// ============================================================================

/**
 * @brief Получить полную конфигурацию в формате JSON
 */
void handleGetConfig(AsyncWebServerRequest *request) {
  LOG_INFO(WEBSERVER, "Config API: Getting full configuration");
  
  char json[CONFIG_MAX_SIZE];
  size_t len = config_get_json(json, sizeof(json));
  
  request->send(200, "application/json", String(json));
}

/**
 * @brief Сохранить конфигурацию из JSON
 */
void handleSaveConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
  LOG_INFO_FMT(WEBSERVER, "Config API: Saving configuration (%d bytes)", len);
  
  if (len == 0) {
    request->send(400, "application/json", "{\"error\":\"No data\"}");
    return;
  }
  
  if (len > CONFIG_MAX_SIZE) {
    request->send(400, "application/json", "{\"error\":\"Data too large\"}");
    return;
  }
  
  // Копируем данные в нуль-терминированную строку
  char* json_str = new char[len + 1];
  memcpy(json_str, data, len);
  json_str[len] = '\0';
  
  bool result = config_set_json(json_str);
  delete[] json_str;
  
  if (result) {
    config_save();
    settings_load();  // Обновляем settings из config
    request->send(200, "application/json", "{\"status\":\"ok\"}");
    LOG_INFO(WEBSERVER, "Config API: Configuration saved successfully");
  } else {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    LOG_ERROR(WEBSERVER, "Config API: Failed to save configuration");
  }
}

/**
 * @brief Удалить файл конфигурации (сброс к дефолтному)
 */
void handleDeleteConfig(AsyncWebServerRequest *request) {
  LOG_INFO(WEBSERVER, "Config API: Deleting configuration");
  
  bool result = config_delete();
  
  if (result) {
    config_reset();
    settings_load();
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    request->send(500, "application/json", "{\"error\":\"Failed to delete\"}");
  }
}

/**
 * @brief Создать резервную копию конфигурации
 */
void handleBackupConfig(AsyncWebServerRequest *request) {
  LOG_INFO(WEBSERVER, "Config API: Creating backup");
  
  bool result = config_backup();
  
  if (result) {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    request->send(500, "application/json", "{\"error\":\"Failed to backup\"}");
  }
}

/**
 * @brief Восстановить конфигурацию из резервной копии
 */
void handleRestoreConfig(AsyncWebServerRequest *request) {
  LOG_INFO(WEBSERVER, "Config API: Restoring from backup");
  
  bool result = config_restore();
  
  if (result) {
    settings_load();
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    request->send(500, "application/json", "{\"error\":\"Failed to restore\"}");
  }
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

// ============================================================================
// API для обновления прошивки с SD карты
// ============================================================================

/**
 * @brief Получить статус прошивки на SD карте
 */
void handleGetFirmwareStatus(AsyncWebServerRequest *request) {
  String json = "{";
  
  if (SD.exists(SD_FIRMWARE_PATH "/firmware.bin")) {
    File fw = SD.open(SD_FIRMWARE_PATH "/firmware.bin", FILE_READ);
    if (fw) {
      json += "\"exists\":true,";
      json += "\"size\":" + String(fw.size()) + ",";
      json += "\"message\":\"Firmware file found on SD card\"";
      fw.close();
    } else {
      json += "\"exists\":false,\"message\":\"Error reading firmware file\"";
    }
  } else {
    json += "\"exists\":false,\"message\":\"No firmware file on SD card\"";
  }
  
  json += "}";
  request->send(200, "application/json", json);
}

/**
 * @brief Загрузить файл прошивки на SD карту
 */
void handleUploadFirmware(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  static File uploadFile;
  
  if (!index) {
    // Начало загрузки - проверяем место на SD
    if (!sdcard_is_ready()) {
      request->send(500, "application/json", "{\"error\":\"SD card not ready\"}");
      return;
    }
    
    // Создаем директорию если нет
    if (!SD.exists(SD_FIRMWARE_PATH)) {
      SD.mkdir(SD_FIRMWARE_PATH);
    }
    
    // Удаляем старый файл если есть
    if (SD.exists(SD_FIRMWARE_PATH "/firmware.bin")) {
      SD.remove(SD_FIRMWARE_PATH "/firmware.bin");
    }
    
    uploadFile = SD.open(SD_FIRMWARE_PATH "/firmware.bin", FILE_WRITE);
    if (!uploadFile) {
      request->send(500, "application/json", "{\"error\":\"Cannot create firmware file\"}");
      return;
    }
    
    LOG_INFO(WEBSERVER, "Firmware upload started");
  }
  
  if (uploadFile && len) {
    uploadFile.write(data, len);
  }
  
  if (final) {
    if (uploadFile) {
      size_t fileSize = uploadFile.size();
      uploadFile.close();
      LOG_INFO_FMT(WEBSERVER, "Firmware upload completed: %d bytes", fileSize);
      request->send(200, "application/json", "{\"status\":\"ok\",\"size\":" + String(fileSize) + "}");
    } else {
      request->send(500, "application/json", "{\"error\":\"Upload failed\"}");
    }
  }
}

/**
 * @brief Запустить обновление прошивки с SD карты
 */
void handleFlashFirmware(AsyncWebServerRequest *request) {
  if (!sdcard_is_ready()) {
    request->send(500, "application/json", "{\"error\":\"SD card not ready\"}");
    return;
  }
  
  if (!SD.exists(SD_FIRMWARE_PATH "/firmware.bin")) {
    request->send(404, "application/json", "{\"error\":\"Firmware file not found\"}");
    return;
  }
  
  File firmwareFile = SD.open(SD_FIRMWARE_PATH "/firmware.bin", FILE_READ);
  if (!firmwareFile) {
    request->send(500, "application/json", "{\"error\":\"Cannot open firmware file\"}");
    return;
  }
  
  size_t firmwareSize = firmwareFile.size();
  LOG_INFO_FMT(WEBSERVER, "Flashing firmware: %d bytes", firmwareSize);
  
  // Начинаем обновление
  if (!Update.begin(firmwareSize)) {
    firmwareFile.close();
    LOG_ERROR(WEBSERVER, "Update.begin failed");
    request->send(500, "application/json", "{\"error\":\"Update.begin failed\"}");
    return;
  }
  
  // Записываем прошивку
  size_t written = Update.writeStream(firmwareFile);
  firmwareFile.close();
  
  LOG_INFO_FMT(WEBSERVER, "Firmware written: %d/%d bytes", written, firmwareSize);
  
  if (written != firmwareSize) {
    request->send(500, "application/json", "{\"error\":\"Size mismatch\"}");
    return;
  }
  
  // Завершаем обновление
  if (Update.end(true)) {
    LOG_INFO(WEBSERVER, "Firmware flash successful. Rebooting...");
    request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Firmware flashed. Rebooting...\"}");
    
    // Перезагрузка через 2 секунды
    delay(2000);
    ESP.restart();
  } else {
    LOG_ERROR(WEBSERVER, "Update.end failed");
    request->send(500, "application/json", "{\"error\":\"Update.end failed\"}");
  }
}

/**
 * @brief Удалить файл прошивки с SD карты
 */
void handleDeleteFirmware(AsyncWebServerRequest *request) {
  if (SD.exists(SD_FIRMWARE_PATH "/firmware.bin")) {
    if (SD.remove(SD_FIRMWARE_PATH "/firmware.bin")) {
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      request->send(500, "application/json", "{\"error\":\"Cannot delete firmware\"}");
    }
  } else {
    request->send(404, "application/json", "{\"error\":\"Firmware not found\"}");
  }
}
