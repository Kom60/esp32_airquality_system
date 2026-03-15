#include "headers.h"
#include "main.h"

void setup(void)
{
  Serial.begin(115200);

  // Инициализация системы
  sensors_setup();      // Датчики (I2C, mutex, Wire, 8 датчиков)
  display_setup();      // Дисплей
  spiffs_setup();       // SPIFFS файловая система
  wifi_setup();         // WiFi подключение
  web_setup();          // Веб-сервер и WebSocket
  microphone_init();    // Микрофон и I2S задачи
  app_tasks_init();     // Задачи приложения (WebSocket, SendData)
}

void loop()
{
  // Пустой цикл - вся логика в задачах FreeRTOS
  vTaskDelay(pdMS_TO_TICKS(100));
}
