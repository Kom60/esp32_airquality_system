#include "headers.h"
#include "main.h"

// Функция проверки валидности числа
bool is_valid_float(float value) {
    return !isnan(value) && !isinf(value) && value < 1e6 && value > -1e6;
}

// Форматирование float в строку без экспоненты и с защитой от -0.0
String format_float(float value, int decimals = 1) {
    if (!is_valid_float(value)) return "0";
    // Убираем отрицательный ноль
    if (value < 0.0001 && value > -0.0001) value = 0.0;
    char buf[16];
    dtostrf(value, 1, decimals, buf);
    return String(buf);
}

// Отправка данных на ПК
void send_data_to_pc() {
    HTTPClient http;

    String url = "http://" + String(PC_IP) + ":" + String(PC_PORT) + "/api/data";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    // Формируем ВАЛИДНЫЙ JSON с использованием новых имен переменных
    String json = "{";
    
    // BME280 sensor data
    json += "\"bme_temperature\":" + format_float(AIR_data.bme_temperature) + ",";
    json += "\"bme_pressure\":" + format_float(AIR_data.bme_pressure) + ",";
    json += "\"bme_humidity\":" + format_float(AIR_data.bme_humidity) + ",";
    
    // HTU21DF sensor data
    json += "\"htu_temperature\":" + format_float(AIR_data.htu_temperature) + ",";
    json += "\"htu_humidity\":" + format_float(AIR_data.htu_humidity) + ",";
    
    // SCD4X sensor data
    json += "\"scd4x_co2\":" + String(is_valid_float(AIR_data.scd4x_co2) ? (int)AIR_data.scd4x_co2 : 0) + ",";
    json += "\"scd4x_temperature\":" + format_float(AIR_data.scd4x_temperature) + ",";
    json += "\"scd4x_humidity\":" + format_float(AIR_data.scd4x_humidity) + ",";
    
    // PMS sensor data
    json += "\"pms_pm1\":" + String(AIR_data.pms_pm1) + ",";
    json += "\"pms_pm2_5\":" + String(AIR_data.pms_pm2_5) + ",";
    json += "\"pms_pm10\":" + String(AIR_data.pms_pm10) + ",";
    
    // MS5611 sensor data
    json += "\"ms5611_pressure\":" + format_float(AIR_data.ms5611_pressure) + ",";
    json += "\"ms5611_temperature\":" + format_float(AIR_data.ms5611_temperature) + ",";
    
    // BH1750 sensor data
    json += "\"bh1750_lighting\":" + format_float(AIR_data.bh1750_lighting) + ",";
    
    // VEML6070 sensor data
    json += "\"veml_uv\":" + String(AIR_data.veml_uv) + ",";
    
    // CH2O sensor data
    json += "\"ch2o_value\":" + format_float(AIR_data.ch2o_value, 3) + ",";
    
    // Microphone data
    json += "\"microphone_noise\":" + format_float(AIR_data.microphone_noise);
    
    json += "}";
    
    // Отладка: выводим JSON в Serial
    Serial.print("[PC] Отправка: ");
    Serial.println(json);
    
    int httpResponseCode = http.POST(json);
    //yield();
    if (httpResponseCode == 200) {
        Serial.println("[PC] ✓ Данные приняты сервером");
    } else {
        Serial.print("[PC] ✗ Ошибка отправки. Код: ");
        Serial.println(httpResponseCode);
        // Доп. отладка при ошибке
        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.print("[PC] Ответ сервера: ");
            Serial.println(payload);
        }
    }
    
    http.end();
}

void setup(void)
{
  Serial.begin(115200);
  
  // Создаём mutex для защиты I2C шины ПЕРЕД инициализацией датчиков!
  i2c_mutex = xSemaphoreCreateMutex();
  
  // Инициализация I2C с явной установкой частоты
  Wire.begin();
  Wire.setClock(100000);  // 100 kHz для стабильности SCD40 и других датчиков
  
  Serial.println("[I2C] Mutex создан, частота 100 kHz");
  
  // Инициализация датчиков с задержкой между задачами для стабильности I2C
  htu_setup();
  vTaskDelay(pdMS_TO_TICKS(100));  // Даём задаче HTU время на старт
  
  MS5611_setup();
  vTaskDelay(pdMS_TO_TICKS(100));  // Даём задаче MS5611 время на старт
  
  bme_setup();
  vTaskDelay(pdMS_TO_TICKS(100));  // Даём задаче BME время на старт
  
  BH1750_setup();
  vTaskDelay(pdMS_TO_TICKS(100));  // Даём задаче BH1750 время на старт
  
  PMS_setup();
  CH2O_setup();
  VEML_setup();
  vTaskDelay(pdMS_TO_TICKS(100));
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS could not initialize");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Establishing connection to WiFi with SSID: " + String(WIFI_SSID));

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { // define here wat the webserver needs to do
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "File not found"); });

  server.serveStatic("/", SPIFFS, "/");

  webSocket.begin();                 // start websocket
  webSocket.onEvent(webSocketEvent); // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"
  SCD40_setup();
  server.begin();
  // Create FreeRTOS queue
  samples_queue = xQueueCreate(8, sizeof(sum_queue_t));

  // Create the I2S reader FreeRTOS task
  // NOTE: Current version of ESP-IDF will pin the task
  //       automatically to the first core it happens to run on
  //       (due to using the hardware FPU instructions).
  //       For manual control see: xTaskCreatePinnedToCore
  xTaskCreate(mic_i2s_reader_task, "Mic I2S Reader", I2S_TASK_STACK, NULL, I2S_TASK_PRI, NULL);
  xTaskCreatePinnedToCore(INMP441_measurementTaskFunction, "INMP441MeasurementTask", 2048, NULL, 1, &INMP441_measurementTask, 0);
}

void loop()
{
  webSocket.loop();             // Update function for the webSockets
  unsigned long now = millis(); // read out the current "time" ("millis()" gives the time in ms since the Arduino started)
  if ((unsigned long)(now - previousMillis) > interval)
  { // check if "interval" ms has passed since last time the clients were updated
    previousMillis = now;
    //tft.fillScreen(TFT_BLACK);
    display_all_data();

    // Отправка данных о системе
    sendJson("esp32_cpu_freq", String(esp_clk_cpu_freq()));
    sendJson("esp32_cpu_temp", String(temperatureRead() * 100));
    
    send_data_to_pc();
  }
}
