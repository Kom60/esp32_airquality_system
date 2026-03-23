#include "headers.h"
#include "main.h"

void setup(void)
{
  Serial.begin(115200);
  
  LOG_INFO(SPIFFS, "=== ESP32 Air Quality Monitor ===");
  LOG_INFO(SPIFFS, "System initialization started");

  // Инициализация системы
  LOG_INFO(SENSORS, "Initializing sensors...");
  sensors_setup();      // Датчики (I2C, mutex, Wire, 9 датчиков + микрофон)
  LOG_INFO(SENSORS, "Sensors initialized");
  
  LOG_INFO(DISPLAY, "Initializing display...");
  display_setup();      // Дисплей
  LOG_INFO(DISPLAY, "Display initialized");
  
  LOG_INFO(SPIFFS, "Initializing SPIFFS...");
  spiffs_setup();       // SPIFFS файловая система
  LOG_INFO(SPIFFS, "SPIFFS initialized");
  
  LOG_INFO(SD, "Initializing SD card...");
  sdcard_setup();       // SD карта
  LOG_INFO(SD, "SD card initialized");
  
  LOG_INFO(SD, "Initializing SD logging task...");
  sdcard_logging_task_init();  // Задача периодической записи на SD (каждые 10 сек)
  LOG_INFO(SD, "SD logging task started");
  
  LOG_INFO(WIFI, "Initializing WiFi...");
  wifi_setup();         // WiFi подключение
  LOG_INFO(WIFI, "WiFi initialized");
  
  LOG_INFO(WEBSERVER, "Initializing web server...");
  web_setup();          // Веб-сервер и WebSocket
  LOG_INFO(WEBSERVER, "Web server initialized");
  
  LOG_INFO(TASKS, "Initializing application tasks...");
  app_tasks_init();     // Задачи приложения (WebSocket, SendData)
  LOG_INFO(TASKS, "Application tasks started");
  
  LOG_INFO(SPIFFS, "=== System initialization complete ===");
}

void loop()
{
  // Пустой цикл - вся логика в задачах FreeRTOS
  vTaskDelay(pdMS_TO_TICKS(100));
}
