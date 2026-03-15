#include "headers.h"
#include "main.h"
#include "settings.h"

// Переменные для расчёта загрузки CPU
unsigned long loop_start_time = 0;
unsigned long loop_total_time = 0;
unsigned long loop_count = 0;
float cpu_load_percent = 0.0;

// Инициализация WiFi с использованием настроек из NVS или defaults
void wifi_setup()
{
  // Инициализация настроек из NVS
  settings_init();
  // Применяем WiFi настройки из NVS если они есть
  if (strlen(settings.wifi_ssid) > 0)
  {
    Serial.println("Using WiFi settings from NVS: " + String(settings.wifi_ssid));
    WiFi.begin(settings.wifi_ssid, settings.wifi_password);
  }
  else
  {
    // Используем настройки по умолчанию из secrets.h
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  Serial.println("Establishing connection to WiFi with SSID: " + String(WiFi.SSID()));

  // Ожидание подключения
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());

  // Обновляем интервал из настроек
  interval = settings.update_interval * 1000; // конвертируем секунды в миллисекунды
}

// Форматирование float в строку без экспоненты и с защитой от -0.0
String format_float(float value, int decimals = 1)
{
  if (!is_valid_float(value))
    return "0";
  // Убираем отрицательный ноль
  if (value < 0.0001 && value > -0.0001)
    value = 0.0;
  char buf[16];
  dtostrf(value, 1, decimals, buf);
  return String(buf);
}

void setup(void)
{
  Serial.begin(115200);
  // Создаём mutex для защиты I2C шины ПЕРЕД инициализацией датчиков!
  i2c_mutex = xSemaphoreCreateMutex();
  // Инициализация I2C с явной установкой частоты
  Wire.begin();
  Wire.setClock(100000); // 100 kHz для стабильности SCD40 и других датчиков
  Serial.println("[I2C] Mutex создан, частота 100 kHz");
  // Инициализация датчиков с задержкой между задачами для стабильности I2C
  htu_setup();
  vTaskDelay(pdMS_TO_TICKS(100)); // Даём задаче HTU время на старт
  MS5611_setup();
  vTaskDelay(pdMS_TO_TICKS(100)); // Даём задаче MS5611 время на старт
  bme_setup();
  vTaskDelay(pdMS_TO_TICKS(100)); // Даём задаче BME время на старт
  BH1750_setup();
  vTaskDelay(pdMS_TO_TICKS(100)); // Даём задаче BH1750 время на старт
  PMS_setup();
  CH2O_setup();
  VEML_setup();
  vTaskDelay(pdMS_TO_TICKS(100));
  display_setup();

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS could not initialize");
  }

  // Инициализация WiFi
  wifi_setup();
  // Инициализация SCD40 перед веб-сервером
  SCD40_setup();
  // Инициализация веб-сервера и WebSocket
  web_setup();
  // Инициализация микрофона и создание задач
  microphone_init();
  // Создание задачи WebSocket на ядре 1
  xTaskCreatePinnedToCore(webSocketTaskFunction, "WebSocket Task", WEBSOCKET_TASK_STACK, NULL, WEBSOCKET_TASK_PRI, &webSocketTaskHandle, 1);
  Serial.println("[WebSocket] Task created on core 1");
}

void loop()
{
  loop_start_time = micros(); // Засекаем время начала цикла
  unsigned long now = millis(); // read out the current "time" ("millis()" gives the time in ms since the Arduino started)
  // Обновляем интервал из настроек (на случай изменений)
  static int last_interval = 0;
  if (settings.update_interval != last_interval)
  {
    interval = settings.update_interval * 1000;
    last_interval = settings.update_interval;
    Serial.println("Interval updated: " + String(interval) + "ms");
  }

  if ((unsigned long)(now - previousMillis) > interval)
  { // check if "interval" ms has passed since last time the clients were updated
    previousMillis = now;

    // Отправка данных датчиков в WebSocket
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

    // Отправка данных о системе
    sendJson("esp32_cpu_freq", String(esp_clk_cpu_freq()));
    sendJson("esp32_cpu_temp", String(temperatureRead()));
    sendJson("esp32_free_heap", String(ESP.getFreeHeap()));

    // Расчёт загрузки CPU на основе времени выполнения loop
    cpu_load_percent = (float)loop_total_time / (loop_count * interval * 1000) * 100.0;
    if (cpu_load_percent > 100)
      cpu_load_percent = 100;
    sendJson("esp32_cpu_load", String(cpu_load_percent));

    // Сброс счётчиков каждые 100 циклов
    if (loop_count >= 100)
    {
      loop_total_time = 0;
      loop_count = 0;
    }

    sendJson("wifi_rssi", String(WiFi.RSSI()));

    send_data_to_pc();
  }

  // Измеряем время выполнения цикла
  unsigned long loop_time = micros() - loop_start_time;
  loop_total_time += loop_time;
  loop_count++;
}
