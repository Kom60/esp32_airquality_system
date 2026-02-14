#include "headers.h"
#include "main.h"

const char* PC_IP = "192.168.0.220";  // ← ЗАМЕНИТЕ НА РЕАЛЬНЫЙ IP ВАШЕГО ПК!
const int PC_PORT = 8080;

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
    
    // Формируем ВАЛИДНЫЙ JSON
    String json = "{";
    json += "\"outdoor_temp\":" + format_float(AIR_data.Outdoor_temp) + ",";
    json += "\"outdoor_pressure\":" + format_float(AIR_data.Outdoor_pressure) + ",";
    json += "\"outdoor_humidity\":" + format_float(AIR_data.Outdoor_Humidity) + ",";
    json += "\"indoor_temp\":" + format_float(AIR_data.Indoor_temp) + ",";
    json += "\"indoor_pressure\":" + format_float(AIR_data.Indoor_pressure) + ",";
    json += "\"indoor_humidity\":" + format_float(AIR_data.Indoor_humidity) + ",";
    json += "\"pm1\":" + format_float(AIR_data.Indoor_PM1) + ",";
    json += "\"pm25\":" + format_float(AIR_data.Indoor_PM2) + ",";
    json += "\"pm10\":" + format_float(AIR_data.Indoor_PM10) + ",";
    json += "\"co2\":" + String(is_valid_float(AIR_data.CO2) ? (int)AIR_data.CO2 : 0) + ",";
    json += "\"ch2o\":" + format_float(AIR_data.CH2O, 3) + ",";
    json += "\"lighting\":" + format_float(AIR_data.Lighting);
    json += "}";
    
    // Отладка: выводим JSON в Serial
    Serial.print("[PC] Отправка: ");
    Serial.println(json);
    
    int httpResponseCode = http.POST(json);
    
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
  htu_setup();
  
  MS5611_setup();
  bme_setup();
  SCD40_setup();
  BH1750_setup();
  
  // pms.init();
  PMS_setup();
  CH2O_setup();
  VEML_setup();
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS could not initialize");
  }

  WiFi.begin(ssid, password);
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));

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
    tft.fillScreen(TFT_BLACK);
    display_bme();
    display_indoor();

    sendJson("cpu_voltage", String(random(360)));
    sendJson("indoor_radiation", String(0));
    sendJson("outdoor_light", String(0));
    sendJson("outdoor_CO2", String(0));
    sendJson("outdoor_CH2O", String(0));
    sendJson("esp32_cpu_freq", String(esp_clk_cpu_freq()));
    send_data_to_pc();
  }
}
