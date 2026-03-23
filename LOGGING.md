# Система логирования для ESP32 Air Quality Monitor

## Обзор

Система предоставляет гибкое логирование через последовательный порт с настраиваемыми уровнями для каждого компонента.

## Уровни логирования

| Уровень | Значение | Описание |
|---------|----------|----------|
| `LOG_LEVEL_NONE` | 0 | Никаких логов |
| `LOG_LEVEL_ERROR` | 1 | Только критические ошибки |
| `LOG_LEVEL_WARNING` | 2 | Ошибки + предупреждения |
| `LOG_LEVEL_INFO` | 3 | Ошибки + предупреждения + информация |
| `LOG_LEVEL_DEBUG` | 4 | Все логи (включая отладочные) |

## Компоненты

| Компонент | Макрос | Описание |
|-----------|--------|----------|
| Датчики | `SENSORS` | Общий лог всех датчиков |
| SCD4X | `SCD4X` | Датчик CO2 (Sensirion SCD40) |
| PMS | `PMS` | Датчик пыли (PMSx003) |
| BME280 | `BME` | Температура/влажность/давление |
| HTU21D | `HTU` | Температура/влажность |
| MS5611 | `MS5611` | Барометр |
| BH1750 | `BH1750` | Освещённость |
| VEML6070 | `VEML` | UV излучение |
| INA226 | `INA226` | Ток/напряжение/мощность |
| CH2O | `CH2O` | Формальдегид |
| Микрофон | `MICROPHONE` | INMP441, шум |
| WiFi | `WIFI` | Подключение к сети |
| NTP | `NTP` | Синхронизация времени |
| SD карта | `SD` | Инициализация, запись файлов |
| Дисплей | `DISPLAY` | Инициализация TFT, отрисовка |
| Веб-сервер | `WEBSERVER` | HTTP запросы |
| WebSocket | `WEBSOCKET` | WebSocket соединения |
| SendData | `SEND_DATA` | Задача отправки данных |
| SPIFFS | `SPIFFS` | Файловая система |
| Сеть | `NETWORK` | Сетевые настройки |
| Задачи | `TASKS` | FreeRTOS задачи |
| Настройки | `SETTINGS` | Чтение/запись настроек |

## Макросы для логирования

### Базовые макросы

```cpp
// Ошибка (всегда выводится если компонент включен)
LOG_ERROR(SENSORS, "Sensor not found");
LOG_E(SENSORS, "Sensor not found");  // короткая версия

// Предупреждение
LOG_WARNING(WIFI, "Weak signal strength");
LOG_W(WIFI, "Weak signal strength");  // короткая версия

// Информация
LOG_INFO(SD, "File opened successfully");
LOG_I(SD, "File opened successfully");  // короткая версия

// Отладка
LOG_DEBUG(TASKS, "Task started");
LOG_D(TASKS, "Task started");  // короткая версия
```

### Вывод значений

```cpp
// Информация со значением
LOG_INFO_VAL(SENSORS, temperature, 25.6);
// Вывод: INFO [readTemperature] temperature = 25.6

// Отладка со значением
LOG_DEBUG_VAL(WIFI, rssi, -65);
// Вывод: DEBUG [checkSignal] rssi = -65
```

### Форматированный вывод (printf-style)

```cpp
LOG_INFO_FMT(SENSORS, "Temp: %.2f°C, Humidity: %.1f%%", temperature, humidity);
// Вывод: INFO [readSensors] Temp: 25.60°C, Humidity: 65.0%

LOG_DEBUG_FMT(TASKS, "Task %s stack: %d bytes", taskName, stackSize);
// Вывод: DEBUG [monitorStack] Task SensorTask stack: 1024 bytes
```

## Настройка в platformio.ini

### Глобальный уровень логирования

```ini
; Все логи на уровне INFO
-D GLOBAL_LOG_LEVEL=3

; Только ошибки и предупреждения
-D GLOBAL_LOG_LEVEL=2

; Полная отладка
-D GLOBAL_LOG_LEVEL=4
```

### Включение/отключение компонентов

```ini
; Включить логи датчиков
-D LOG_SENSORS=1

; Отключить логи дисплея
-D LOG_DISPLAY=0

; Отключить все логи (только ошибки)
-D LOG_SENSORS=0
-D LOG_WIFI=0
-D LOG_SD=0
-D LOG_DISPLAY=0
-D LOG_WEBSERVER=0
```

### Индивидуальные уровни для компонентов

```ini
; Датчики - полная отладка
-D SENSORS_LOG_LEVEL=4

; WiFi - только ошибки
-D WIFI_LOG_LEVEL=1

; Дисплей - предупреждения и выше
-D DISPLAY_LOG_LEVEL=2

; SD карта - информация
-D SD_LOG_LEVEL=3
```

## Примеры использования

### Пример 1: Логирование в функции датчика

```cpp
float readTemperature() {
    LOG_DEBUG(SENSORS, "Reading temperature from BME280");
    
    float temp = bme.readTemperature();
    
    if (isnan(temp)) {
        LOG_ERROR(SENSORS, "Failed to read temperature");
        return -999;
    }
    
    LOG_INFO_VAL(SENSORS, temperature, temp);
    return temp;
}
```

### Пример 2: Логирование WiFi подключения

```cpp
void connectToWiFi() {
    LOG_INFO(WIFI, "Connecting to WiFi network...");
    LOG_INFO_FMT(WIFI, "SSID: %s", WiFi.SSID().c_str());
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        attempts++;
        LOG_DEBUG(WIFI, "Waiting for connection...");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        LOG_INFO(WIFI, "WiFi connected successfully");
        LOG_INFO_FMT(WIFI, "IP address: %s", WiFi.localIP().toString().c_str());
    } else {
        LOG_ERROR(WIFI, "Failed to connect to WiFi");
    }
}
```

### Пример 3: Логирование SD карты

```cpp
void logToSD(const char* data) {
    LOG_DEBUG(SD, "Opening file for writing");
    
    File file = SD.open("/log.txt", FILE_WRITE);
    if (!file) {
        LOG_ERROR(SD, "Failed to open log.txt");
        return;
    }
    
    LOG_DEBUG(SD, "Writing data to SD");
    file.println(data);
    file.close();
    
    LOG_INFO(SD, "Data written to SD successfully");
}
```

### Пример 4: Логирование в задачах FreeRTOS

```cpp
void sensorTask(void* pvParameters) {
    LOG_INFO(TASKS, "Sensor task started");
    LOG_DEBUG_FMT(TASKS, "Stack size: %d", uxTaskGetStackHighWaterMark(NULL));
    
    while (1) {
        LOG_DEBUG(SENSORS, "Starting sensor reading cycle");
        
        readAllSensors();
        
        LOG_DEBUG(SENSORS, "Sensor reading cycle complete");
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

## Быстрая настройка

### Режим "Только ошибки" (минимум логов)

```ini
-D GLOBAL_LOG_LEVEL=1
-D LOG_SENSORS=1
-D LOG_WIFI=1
-D LOG_SD=1
```

### Режим "Информация" (рекомендуется для продакшена)

```ini
-D GLOBAL_LOG_LEVEL=3
-D DISPLAY_LOG_LEVEL=2
-D SPIFFS_LOG_LEVEL=2
```

### Режим "Отладка" (полные логи для разработки)

```ini
-D GLOBAL_LOG_LEVEL=4
-D LOG_SENSORS=1
-D LOG_WIFI=1
-D LOG_SD=1
-D LOG_DISPLAY=1
-D LOG_WEBSERVER=1
-D LOG_TASKS=1
```

### Режим "Отключить все логи" (максимальная производительность)

```ini
-D GLOBAL_LOG_LEVEL=0
```

## Мониторинг через последовательный порт

```bash
# Запуск мониторинга в PlatformIO
pio device monitor

# Мониторинг с фильтрацией
pio device monitor --filter esp32_exception_decoder
```

## Советы

1. **Используйте уровни логирования разумно**: `ERROR` для критических проблем, `INFO` для важных событий, `DEBUG` для отладки
2. **Отключайте DEBUG в продакшене**: это уменьшает объем логов и повышает производительность
3. **Используйте форматированный вывод** для сложных сообщений
4. **Настраивайте уровни по компонентам**: например, отключите логи дисплея, но оставьте логи датчиков
