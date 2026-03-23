#include "headers.h"
#include "sdcard.h"
#include <SD.h>
#include <ArduinoJson.h>
#include <time.h>

extern Meteo_data AIR_data;

static TaskHandle_t sdLogTaskHandle = NULL;

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
    Serial.println("[SD] Initializing SD card...");

    // Инициализация SPI для SD карты (та же шина, что и дисплей)
    if (!SD.begin(SD_CS)) {
        Serial.println("[SD] Card Mount Failed");
        return false;
    }

    Serial.println("[SD] Card Mount OK");
    Serial.printf("[SD] Card size: %llu MB\n", SD.cardSize() / (1024 * 1024));
    Serial.printf("[SD] Used space: %llu MB\n", SD.usedBytes() / (1024 * 1024));

    return true;
}

void sdcard_write_test_data()
{
    File file = SD.open("/data.json", FILE_APPEND);

    if (!file) {
        Serial.println("[SD] Failed to open file for writing");
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
    serializeJson(doc, file);
    file.println(); // Новая строка для удобства чтения

    file.close();
    Serial.println("[SD] Data written to /data.json");
}

// Задача периодической записи данных на SD карту
void sdLogTaskFunction(void *parameter)
{
    Serial.println("[SD] Logging task started (interval: 10 sec)");

    // Задержка 40 сек перед первой записью (ожидание синхронизации NTP)
    Serial.println("[SD] Waiting 40 sec for NTP sync before first log...");
    vTaskDelay(pdMS_TO_TICKS(40000));

    while (1) {
        sdcard_write_test_data();
        vTaskDelay(pdMS_TO_TICKS(SD_LOG_INTERVAL_MS));
    }
}

// Инициализация задачи логгирования
void sdcard_logging_task_init()
{
    xTaskCreatePinnedToCore(
        sdLogTaskFunction,      // Функция задачи
        "SD Log Task",          // Имя задачи
        SD_LOG_TASK_STACK,      // Размер стека
        NULL,                   // Параметры
        SD_LOG_TASK_PRI,        // Приоритет
        &sdLogTaskHandle,       // Дескриптор задачи
        1                       // Ядро 1
    );
    Serial.println("[SD] Logging task created on core 1");
}
