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

// Пути на SD карте
#define SD_WWW_PATH "/www"
#define SD_CONFIG_PATH "/config"
#define SD_LOGS_PATH "/logs"
#define SD_FIRMWARE_PATH "/firmware"
#define SD_CONFIG_FILE "/config/config.json"
#define SD_CONFIG_BACKUP "/config/config.backup.json"

// Максимальный размер файла лога перед ротацией (МБ)
#define SD_MAX_LOG_SIZE_MB 32

bool sdcard_setup();
void sdcard_create_dirs();
bool sdcard_file_exists(const char* path);
void sdcard_write_log_data();
void sdcard_cleanup_old_logs(uint32_t max_files = 30);
void sdcard_logging_task_init();
bool sdcard_is_ready();

#endif
