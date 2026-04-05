#include "headers.h"
#include "system_init.h"

// Инициализация задач приложения (WebSocket, SendData)
void app_tasks_init()
{
  // Создание задачи WebSocket на ядре 1
  xTaskCreatePinnedToCore(webSocketTaskFunction, "WebSocket Task", WEBSOCKET_TASK_STACK, NULL, WEBSOCKET_TASK_PRI, &webSocketTaskHandle, 1);
  LOG_INFO(WEBSOCKET, "Task created on core 1");

  // Создание задачи отправки данных на ядре 1
  xTaskCreatePinnedToCore(sendDataTaskFunction, "SendData Task", SEND_DATA_TASK_STACK, NULL, SEND_DATA_TASK_PRI, &sendDataTaskHandle, 1);
  LOG_INFO(SEND_DATA, "Task created on core 1");
}
