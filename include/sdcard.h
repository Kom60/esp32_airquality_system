#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>

// Пины SD карты (та же SPI шина, что и дисплей)
#define SD_CS 14

// Период записи данных на SD карту (мс)
#define SD_LOG_INTERVAL_MS 10000

// Стек и приоритет задачи логгирования
#define SD_LOG_TASK_STACK 4096
#define SD_LOG_TASK_PRI 1

bool sdcard_setup();
void sdcard_write_test_data();
void sdcard_logging_task_init();

#endif
