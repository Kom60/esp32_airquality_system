#ifndef logger_h
#define logger_h

#include <Arduino.h>

// ============================================================================
// Уровни логирования (как в syslog)
// ============================================================================
#define LOG_LEVEL_NONE      0  // Никаких логов
#define LOG_LEVEL_ERROR     1  // Только ошибки
#define LOG_LEVEL_WARNING   2  // Ошибки + предупреждения
#define LOG_LEVEL_INFO      3  // Ошибки + предупреждения + информация
#define LOG_LEVEL_DEBUG     4  // Все логи (включая отладочные)

// ============================================================================
// Настройки логирования для каждого компонента
// Изменяйте значения в platformio.ini через build_flags
// ============================================================================

// Глобальный уровень логирования (по умолчанию INFO)
#ifndef GLOBAL_LOG_LEVEL
    #define GLOBAL_LOG_LEVEL LOG_LEVEL_INFO
#endif

// Включение/отключение компонентов (по умолчанию включены)
#ifndef LOG_SENSORS
    #define LOG_SENSORS 1
#endif
#ifndef LOG_WIFI
    #define LOG_WIFI 1
#endif
#ifndef LOG_SD
    #define LOG_SD 1
#endif
#ifndef LOG_DISPLAY
    #define LOG_DISPLAY 1
#endif
#ifndef LOG_WEBSERVER
    #define LOG_WEBSERVER 1
#endif
#ifndef LOG_SD
    #define LOG_SD 1
#endif
#ifndef LOG_NETWORK
    #define LOG_NETWORK 1
#endif
#ifndef LOG_TASKS
    #define LOG_TASKS 1
#endif
#ifndef LOG_SETTINGS
    #define LOG_SETTINGS 1
#endif
#ifndef LOG_INA226
    #define LOG_INA226 1
#endif
#ifndef LOG_MICROPHONE
    #define LOG_MICROPHONE 1
#endif
#ifndef LOG_SCD4X
    #define LOG_SCD4X 1
#endif
#ifndef LOG_PMS
    #define LOG_PMS 1
#endif
#ifndef LOG_BH1750
    #define LOG_BH1750 1
#endif
#ifndef LOG_MS5611
    #define LOG_MS5611 1
#endif
#ifndef LOG_VEML
    #define LOG_VEML 1
#endif
#ifndef LOG_BME
    #define LOG_BME 1
#endif
#ifndef LOG_HTU
    #define LOG_HTU 1
#endif
#ifndef LOG_CH2O
    #define LOG_CH2O 1
#endif
#ifndef LOG_WEBSOCKET
    #define LOG_WEBSOCKET 1
#endif
#ifndef LOG_NTP
    #define LOG_NTP 1
#endif
#ifndef LOG_SEND_DATA
    #define LOG_SEND_DATA 1
#endif

// ============================================================================
// Уровни логирования для отдельных компонентов
// ============================================================================
#ifndef SENSORS_LOG_LEVEL
    #define SENSORS_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef WIFI_LOG_LEVEL
    #define WIFI_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef SD_LOG_LEVEL
    #define SD_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef DISPLAY_LOG_LEVEL
    #define DISPLAY_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef WEBSERVER_LOG_LEVEL
    #define WEBSERVER_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef SD_LOG_LEVEL
    #define SD_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef NETWORK_LOG_LEVEL
    #define NETWORK_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef TASKS_LOG_LEVEL
    #define TASKS_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef SETTINGS_LOG_LEVEL
    #define SETTINGS_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef INA226_LOG_LEVEL
    #define INA226_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef MICROPHONE_LOG_LEVEL
    #define MICROPHONE_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef SCD4X_LOG_LEVEL
    #define SCD4X_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef PMS_LOG_LEVEL
    #define PMS_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef BH1750_LOG_LEVEL
    #define BH1750_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef MS5611_LOG_LEVEL
    #define MS5611_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef VEML_LOG_LEVEL
    #define VEML_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef BME_LOG_LEVEL
    #define BME_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef HTU_LOG_LEVEL
    #define HTU_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef CH2O_LOG_LEVEL
    #define CH2O_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef WEBSOCKET_LOG_LEVEL
    #define WEBSOCKET_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef NTP_LOG_LEVEL
    #define NTP_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif
#ifndef SEND_DATA_LOG_LEVEL
    #define SEND_DATA_LOG_LEVEL GLOBAL_LOG_LEVEL
#endif

// ============================================================================
// Макросы для логирования
// ============================================================================

// Внутренний макрос для печати с префиксом
#define _LOG_PREFIX(prefix) do { \
    Serial.print("[" prefix "] "); \
    Serial.print("["); \
    Serial.print(__FUNCTION__); \
    Serial.print("] "); \
} while(0)

// ERROR логи (всегда выводятся, если компонент включен)
#define LOG_ERROR(component, msg) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_ERROR)) { \
            _LOG_PREFIX("ERROR"); \
            Serial.println(msg); \
        } \
    } while(0)

// WARNING логи
#define LOG_WARNING(component, msg) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_WARNING)) { \
            _LOG_PREFIX("WARN"); \
            Serial.println(msg); \
        } \
    } while(0)

// INFO логи
#define LOG_INFO(component, msg) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_INFO)) { \
            _LOG_PREFIX("INFO"); \
            Serial.println(msg); \
        } \
    } while(0)

// DEBUG логи
#define LOG_DEBUG(component, msg) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_DEBUG)) { \
            _LOG_PREFIX("DEBUG"); \
            Serial.println(msg); \
        } \
    } while(0)

// Макросы для вывода значений
#define LOG_ERROR_VAL(component, var, val) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_ERROR)) { \
            _LOG_PREFIX("ERROR"); \
            Serial.print(#var); Serial.print(" = "); Serial.println(val); \
        } \
    } while(0)

#define LOG_INFO_VAL(component, var, val) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_INFO)) { \
            _LOG_PREFIX("INFO"); \
            Serial.print(#var); Serial.print(" = "); Serial.println(val); \
        } \
    } while(0)

#define LOG_DEBUG_VAL(component, var, val) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_DEBUG)) { \
            _LOG_PREFIX("DEBUG"); \
            Serial.print(#var); Serial.print(" = "); Serial.println(val); \
        } \
    } while(0)

// Форматированный вывод (printf-style) - для ESP32
#define LOG_DEBUG_FMT(component, fmt, ...) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_DEBUG)) { \
            _LOG_PREFIX("DEBUG"); \
            Serial.printf(fmt, ##__VA_ARGS__); \
            Serial.println(); \
        } \
    } while(0)

#define LOG_INFO_FMT(component, fmt, ...) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_INFO)) { \
            _LOG_PREFIX("INFO"); \
            Serial.printf(fmt, ##__VA_ARGS__); \
            Serial.println(); \
        } \
    } while(0)

#define LOG_WARNING_FMT(component, fmt, ...) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_WARNING)) { \
            _LOG_PREFIX("WARN"); \
            Serial.printf(fmt, ##__VA_ARGS__); \
            Serial.println(); \
        } \
    } while(0)

#define LOG_ERROR_FMT(component, fmt, ...) \
    do { \
        if (LOG_##component && (component##_LOG_LEVEL >= LOG_LEVEL_ERROR)) { \
            _LOG_PREFIX("ERROR"); \
            Serial.printf(fmt, ##__VA_ARGS__); \
            Serial.println(); \
        } \
    } while(0)

// Короткие алиасы для существующих логов в проекте
#define LOG_E(component, msg) LOG_ERROR(component, msg)
#define LOG_W(component, msg) LOG_WARNING(component, msg)
#define LOG_I(component, msg) LOG_INFO(component, msg)
#define LOG_D(component, msg) LOG_DEBUG(component, msg)

#endif
