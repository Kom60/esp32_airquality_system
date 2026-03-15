#include "headers.h"
#include "system_init.h"

// Инициализация SPIFFS
bool spiffs_setup()
{
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS could not initialize");
    return false;
  }
  Serial.println("[SPIFFS] Mounted successfully");
  return true;
}

// Инициализация задач приложения (WebSocket, SendData)
void app_tasks_init()
{
  // Создание задачи WebSocket на ядре 1
  xTaskCreatePinnedToCore(webSocketTaskFunction, "WebSocket Task", WEBSOCKET_TASK_STACK, NULL, WEBSOCKET_TASK_PRI, &webSocketTaskHandle, 1);
  Serial.println("[WebSocket] Task created on core 1");

  // Создание задачи отправки данных на ядре 1
  xTaskCreatePinnedToCore(sendDataTaskFunction, "SendData Task", SEND_DATA_TASK_STACK, NULL, SEND_DATA_TASK_PRI, &sendDataTaskHandle, 1);
  Serial.println("[SendData] Task created on core 1");
}
