#include "headers.h"
#include "main.h"

void setup(void)
{
  Serial.begin(115200);

  // Инициализация всех датчиков (I2C, mutex, Wire)
  sensors_setup();

  // Инициализация дисплея
  display_setup();

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS could not initialize");
  }

  // Инициализация WiFi
  wifi_setup();

  // Инициализация веб-сервера и WebSocket
  web_setup();

  // Инициализация микрофона и создание задач
  microphone_init();

  // Создание задачи WebSocket на ядре 1
  xTaskCreatePinnedToCore(webSocketTaskFunction, "WebSocket Task", WEBSOCKET_TASK_STACK, NULL, WEBSOCKET_TASK_PRI, &webSocketTaskHandle, 1);
  Serial.println("[WebSocket] Task created on core 1");

  // Создание задачи отправки данных на ядре 1
  xTaskCreatePinnedToCore(sendDataTaskFunction, "SendData Task", SEND_DATA_TASK_STACK, NULL, SEND_DATA_TASK_PRI, &sendDataTaskHandle, 1);
  Serial.println("[SendData] Task created on core 1");
}

void loop()
{
  // Пустой цикл - вся логика в задачах FreeRTOS
  vTaskDelay(pdMS_TO_TICKS(100));
}
