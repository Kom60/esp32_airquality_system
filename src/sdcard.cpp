#include "headers.h"
#include "sdcard.h"
#include "logger.h"
#include <SD.h>
#include <ArduinoJson.h>
#include <time.h>

extern Meteo_data AIR_data;

static TaskHandle_t sdLogTaskHandle = NULL;
static bool sd_ready = false;

// Прототип функции задачи
void sdLogTaskFunction(void *parameter);

// Получение текущей даты и времени в формате строки
static void get_timestamp_string(char* buffer, size_t size)
{
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100)) {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &timeinfo);
    } else {
        // Если время не синхронизировано, используем millis()
        snprintf(buffer, size, "uptime:%lu", millis());
    }
}

bool sdcard_setup()
{
    LOG_INFO(SD, "Initializing SD card...");

    // Инициализация SPI для SD карты (та же шина, что и дисплей)
    if (!SD.begin(SD_CS)) {
        LOG_ERROR(SD, "Card Mount Failed");
        sd_ready = false;
        return false;
    }

    sd_ready = true;
    LOG_INFO(SD, "Card Mount OK");
    LOG_INFO_FMT(SD, "Card size: %llu MB", SD.cardSize() / (1024 * 1024));
    LOG_INFO_FMT(SD, "Used space: %llu MB", SD.usedBytes() / (1024 * 1024));

    // Создание необходимой структуры папок
    sdcard_create_dirs();

    return true;
}

void sdcard_create_dirs()
{
    if (!sd_ready) {
        LOG_ERROR(SD, "SD card not ready, cannot create directories");
        return;
    }

    // Создаем основные директории
    const char* dirs[] = {
        SD_WWW_PATH,
        SD_CONFIG_PATH,
        SD_LOGS_PATH,
        SD_FIRMWARE_PATH
    };

    for (const char* dir : dirs) {
        if (!SD.exists(dir)) {
            if (SD.mkdir(dir)) {
                LOG_INFO_FMT(SD, "Created directory: %s", dir);
            } else {
                LOG_ERROR_FMT(SD, "Failed to create directory: %s", dir);
            }
        } else {
            LOG_INFO_FMT(SD, "Directory exists: %s", dir);
        }
    }
}

bool sdcard_file_exists(const char* path)
{
    if (!sd_ready) return false;
    return SD.exists(path);
}

bool sdcard_is_ready()
{
    return sd_ready;
}

void sdcard_write_log_data()
{
    if (!sd_ready) {
        LOG_ERROR(SD, "SD card not ready, skipping log write");
        return;
    }

    // Формируем имя файла по дате
    struct tm timeinfo;
    char filename[64];
    
    if (getLocalTime(&timeinfo, 100)) {
        strftime(filename, sizeof(filename), SD_LOGS_PATH "/%Y-%m-%d.json", &timeinfo);
    } else {
        snprintf(filename, sizeof(filename), SD_LOGS_PATH "/unknown.json");
    }

    // Проверяем размер файла перед записью
    if (SD.exists(filename)) {
        File checkFile = SD.open(filename, FILE_READ);
        if (checkFile) {
            uint32_t fileSizeMB = checkFile.size() / (1024 * 1024);
            checkFile.close();
            
            if (fileSizeMB >= SD_MAX_LOG_SIZE_MB) {
                LOG_WARNING_FMT(SD, "Log file too large (%u MB), rotating", fileSizeMB);
                // Можно добавить ротацию, но пока просто продолжаем
            }
        }
    }

    // Открываем файл в режиме APPEND
    File file = SD.open(filename, FILE_APPEND);

    if (!file) {
        LOG_ERROR_FMT(SD, "Failed to open file for writing: %s", filename);
        return;
    }

    // Получение текущей даты и времени
    char timestamp[32];
    get_timestamp_string(timestamp, sizeof(timestamp));

    // Формирование JSON документа
    JsonDocument doc;

    doc["timestamp"] = timestamp;
    doc["sensors"]["co2"] = AIR_data.scd4x_co2;
    doc["sensors"]["temperature_bme"] = AIR_data.bme_temperature;
    doc["sensors"]["temperature_htu"] = AIR_data.htu_temperature;
    doc["sensors"]["temperature_scd4x"] = AIR_data.scd4x_temperature;
    doc["sensors"]["humidity_bme"] = AIR_data.bme_humidity;
    doc["sensors"]["humidity_htu"] = AIR_data.htu_humidity;
    doc["sensors"]["humidity_scd4x"] = AIR_data.scd4x_humidity;
    doc["sensors"]["formaldehyde"] = AIR_data.ch2o_value;
    doc["sensors"]["uv"] = AIR_data.veml_uv;
    doc["sensors"]["light"] = AIR_data.bh1750_lighting;
    doc["sensors"]["pressure_bme"] = AIR_data.bme_pressure;
    doc["sensors"]["pressure_ms5611"] = AIR_data.ms5611_pressure;
    doc["sensors"]["pm1_0"] = AIR_data.pms_pm1;
    doc["sensors"]["pm2_5"] = AIR_data.pms_pm2_5;
    doc["sensors"]["pm10"] = AIR_data.pms_pm10;
    doc["sensors"]["microphone_noise"] = AIR_data.microphone_noise;
    doc["sensors"]["ina226_voltage"] = AIR_data.ina226_voltage;
    doc["sensors"]["ina226_current"] = AIR_data.ina226_current;
    doc["sensors"]["ina226_power"] = AIR_data.ina226_power;

    // Запись JSON в файл
    if (serializeJson(doc, file) > 0) {
        file.println(); // Новая строка для удобства чтения
        LOG_INFO_FMT(SD, "Data written to %s", filename);
    } else {
        LOG_ERROR(SD, "Failed to write JSON data");
    }

    file.close();
}

void sdcard_cleanup_old_logs(uint32_t max_files)
{
    if (!sd_ready) return;

    File logsDir = SD.open(SD_LOGS_PATH);
    if (!logsDir || !logsDir.isDirectory()) {
        LOG_ERROR(SD, "Cannot open logs directory");
        return;
    }

    // Считаем файлы и удаляем самые старые, если их больше max_files
    uint32_t fileCount = 0;
    File file = logsDir.openNextFile();
    
    while (file) {
        if (!file.isDirectory()) {
            fileCount++;
        }
        file.close();
        file = logsDir.openNextFile();
    }

    LOG_INFO_FMT(SD, "Found %u log files, max allowed: %u", fileCount, max_files);

    // Если файлов больше нормы - удаляем старые
    if (fileCount > max_files) {
        uint32_t toDelete = fileCount - max_files;
        LOG_INFO_FMT(SD, "Cleaning up %u old log files", toDelete);

        logsDir.rewindDirectory();
        file = logsDir.openNextFile();
        
        while (file && toDelete > 0) {
            if (!file.isDirectory()) {
                String filePath = String(SD_LOGS_PATH) + "/" + file.name();
                file.close();
                
                if (SD.remove(filePath)) {
                    LOG_INFO_FMT(SD, "Deleted old log: %s", filePath.c_str());
                    toDelete--;
                } else {
                    LOG_ERROR_FMT(SD, "Failed to delete: %s", filePath.c_str());
                }
            } else {
                file.close();
            }
            file = logsDir.openNextFile();
        }
    }

    logsDir.close();
}

// Задача периодической записи данных на SD карту
void sdLogTaskFunction(void *parameter)
{
    LOG_INFO(SD, "Logging task started (interval: 10 sec)");

    // Задержка 40 сек перед первой записью (ожидание синхронизации NTP)
    LOG_INFO(SD, "Waiting 40 sec for NTP sync before first log...");
    vTaskDelay(pdMS_TO_TICKS(40000));

    while (1) {
        sdcard_write_log_data();
        
        // Раз в день (каждые 8640 записей) чистим старые логи
        static uint32_t writeCount = 0;
        writeCount++;
        if (writeCount % 8640 == 0) {
            sdcard_cleanup_old_logs(30);
        }
        
        vTaskDelay(pdMS_TO_TICKS(SD_LOG_INTERVAL_MS));
    }
}

// Инициализация задачи логгирования
void sdcard_logging_task_init()
{
    if (!sd_ready) {
        LOG_ERROR(SD, "Cannot create logging task - SD not ready");
        return;
    }

    xTaskCreatePinnedToCore(
        sdLogTaskFunction,      // Функция задачи
        "SD Log Task",          // Имя задачи
        SD_LOG_TASK_STACK,      // Размер стека
        NULL,                   // Параметры
        SD_LOG_TASK_PRI,        // Приоритет
        &sdLogTaskHandle,       // Дескриптор задачи
        1                       // Ядро 1
    );
    LOG_INFO(SD, "Logging task created on core 1");
}
