#include "headers.h"
#include "main.h"

void setup(void)
{
  Serial.begin(115200);

  LOG_INFO(SD, "=== ESP32 Air Quality Monitor ===");
  LOG_INFO(SD, "System initialization started");

  // Инициализация системы
  LOG_INFO(SENSORS, "Initializing sensors...");
  sensors_setup();      // Датчики (I2C, mutex, Wire, 9 датчиков + микрофон)
  LOG_INFO(SENSORS, "Sensors initialized");

  LOG_INFO(DISPLAY, "Initializing display...");
  display_setup();      // Дисплей
  LOG_INFO(DISPLAY, "Display initialized");

  LOG_INFO(SD, "Initializing SD card...");
  sdcard_setup();       // SD карта - основная файловая система
  LOG_INFO(SD, "SD card initialized");

  LOG_INFO(SD, "Initializing SD logging task...");
  sdcard_logging_task_init();  // Задача периодической записи на SD (каждые 10 сек)
  LOG_INFO(SD, "SD logging task started");

  LOG_INFO(WIFI, "Initializing WiFi...");
  wifi_setup();         // WiFi подключение
  LOG_INFO(WIFI, "WiFi initialized");

  // ПРИОРИТЕТ: HTTPS (HTTP и WebSocket отключены для экономии RAM)
  Config& cfg = config_get();
  
  if (cfg.ssl.https_enabled) {
    LOG_INFO(WEBSERVER, "HTTPS-ONLY mode - HTTP server and WebSocket disabled");
    LOG_INFO(WEBSERVER, "Initializing HTTPS server...");
    https_server_init();
    LOG_INFO(WEBSERVER, "HTTPS setup completed");
  } else {
    LOG_INFO(WEBSERVER, "Initializing web server (HTTP + WebSocket)...");
    web_setup();
    LOG_INFO(WEBSERVER, "Web server initialized");
  }

  LOG_INFO(TASKS, "Initializing application tasks...");
  app_tasks_init();     // Задачи приложения (WebSocket, SendData)
  LOG_INFO(TASKS, "Application tasks started");

  LOG_INFO(SD, "=== System initialization complete ===");
}

void loop()
{
  // Обработка HTTPS запросов (библиотека bmedici)
  https_handle_requests();
  
  // Пустой цикл - вся логика в задачах FreeRTOS
  vTaskDelay(pdMS_TO_TICKS(100));
}
