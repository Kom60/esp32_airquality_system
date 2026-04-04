# JSON Конфигурация - ESP32 Air Quality Monitor

## 📋 Обзор

Система конфигурации переведена на JSON-файлы, хранящиеся в SPIFFS. Это позволяет:
- ✅ Редактировать настройки через веб-интерфейс
- ✅ Делать резервные копии конфигурации
- ✅ Быстро развёртывать одинаковые настройки на нескольких устройствах
- ✅ Версионировать конфигурацию в Git

## 📁 Файлы конфигурации

| Файл | Описание |
|------|----------|
| `/config.json` | Основной файл конфигурации (в SPIFFS) |
| `/config.json.backup` | Резервная копия (в SPIFFS) |
| `data/config.json.example` | Шаблон для загрузки в SPIFFS |

## 🌐 Веб-интерфейс

### Страницы конфигурации
- **`/config`** - Полноценный редактор JSON конфигурации с вкладками
- **`/settings`** - Старая страница настроек (обратная совместимость)

### API эндпоинты

| Метод | URL | Описание |
|-------|-----|----------|
| `GET` | `/api/config` | Получить полную конфигурацию в JSON |
| `POST` | `/api/config` | Сохранить конфигурацию из JSON |
| `DELETE` | `/api/config` | Удалить конфиг (сброс к дефолту) |
| `POST` | `/api/config/backup` | Создать резервную копию |
| `POST` | `/api/config/restore` | Восстановить из резервной копии |

### Примеры запросов

**Получить конфигурацию:**
```bash
curl http://192.168.1.100/api/config
```

**Сохранить конфигурацию:**
```bash
curl -X POST http://192.168.1.100/api/config \
  -H "Content-Type: application/json" \
  -d '{"wifi":{"ssid":"MyWiFi","password":"secret"}}'
```

**Создать резервную копию:**
```bash
curl -X POST http://192.168.1.100/api/config/backup
```

## 📝 Структура JSON

### WiFi настройки
```json
{
  "wifi": {
    "ssid": "YourWiFiSSID",
    "password": "YourWiFiPassword",
    "reconnect_interval": 30,
    "reconnect_max_attempts": 0,
    "auto_connect": true,
    "static_ip": "",
    "gateway": "",
    "subnet": "",
    "dns1": "",
    "dns2": ""
  }
}
```

### NTP настройки
```json
{
  "ntp": {
    "server": "pool.ntp.org",
    "gmt_offset_sec": 10800,
    "daylight_offset_sec": 0,
    "auto_sync": true,
    "sync_interval": 3600
  }
}
```

### Датчики
```json
{
  "sensors": {
    "intervals": {
      "bme": 5,
      "htu": 5,
      "scd": 10,
      "pms": 5,
      "ms": 10,
      "bh": 5,
      "veml": 10,
      "ch2o": 10,
      "mic": 5,
      "ina": 10
    },
    "calibration": {
      "temp_bme": 0.0,
      "temp_htu": 0.0,
      "temp_scd": 0.0,
      "hum_bme": 0,
      "hum_htu": 0,
      "hum_scd": 0,
      "press_bme": 0,
      "press_ms": 0
    },
    "enabled": {
      "bme": true,
      "htu": true,
      "scd": true,
      "pms": true,
      "ms": true,
      "bh": true,
      "veml": true,
      "ch2o": true,
      "mic": true,
      "ina": true
    },
    "addresses": {
      "bme": 118,
      "htu": 64,
      "scd": 98,
      "pms_baudrate": 9600,
      "ms": 118,
      "bh": 35,
      "veml": 56,
      "ch2o": 64,
      "ina": 64
    }
  }
}
```

### Предупреждения
```json
{
  "alerts": {
    "thresholds": {
      "co2": {
        "warning": 1000,
        "critical": 1400
      },
      "pm25": {
        "warning": 35,
        "critical": 50
      },
      "pm10": {
        "warning": 50,
        "critical": 100
      },
      "temp": {
        "warning_low": 15.0,
        "warning_high": 30.0,
        "critical_low": 5.0,
        "critical_high": 40.0
      },
      "hum": {
        "warning_low": 30,
        "warning_high": 70,
        "critical_low": 20,
        "critical_high": 80
      },
      "press": {
        "warning_low": 980,
        "warning_high": 1050
      },
      "ch2o": {
        "warning": 0.05,
        "critical": 0.10
      },
      "uv": {
        "warning": 6,
        "critical": 10
      },
      "noise": {
        "warning": 70,
        "critical": 85
      }
    },
    "enabled": {
      "co2": true,
      "pm": true,
      "temp": true,
      "hum": true,
      "press": false,
      "ch2o": true,
      "uv": false,
      "noise": true
    }
  }
}
```

### Дисплей
```json
{
  "display": {
    "enabled": true,
    "brightness": 80,
    "brightness_night": 20,
    "night_mode_start": 23,
    "night_mode_end": 7,
    "screen_timeout": 300,
    "update_interval": 2,
    "auto_rotate": false,
    "rotation": 3
  }
}
```

### SD карта
```json
{
  "sd": {
    "enabled": true,
    "cs_pin": 5,
    "auto_log": true,
    "log_interval": 10,
    "log_format": "csv",
    "create_hourly_files": false,
    "max_file_size_mb": 32,
    "auto_backup": true,
    "backup_interval_hours": 24
  }
}
```

### Веб-сервер и WebSocket
```json
{
  "webserver": {
    "enabled": true,
    "port": 80,
    "cors_enabled": true,
    "cors_origin": "*",
    "max_clients": 5,
    "auth_enabled": false,
    "auth_username": "admin",
    "auth_password": "",
    "session_timeout": 3600
  },
  "websocket": {
    "enabled": true,
    "port": 81,
    "ping_interval": 30,
    "max_clients": 10,
    "broadcast_enabled": true,
    "broadcast_interval": 2
  }
}
```

### MQTT (для будущей интеграции)
```json
{
  "mqtt": {
    "enabled": false,
    "broker": "",
    "port": 1883,
    "username": "",
    "password": "",
    "client_id": "",
    "topic_prefix": "esp32_airquality",
    "qos": 0,
    "retain": true,
    "keepalive": 60,
    "auto_reconnect": true
  }
}
```

### OTA обновления
```json
{
  "ota": {
    "enabled": true,
    "password": "",
    "port": 3232,
    "reboot_on_success": true
  }
}
```

### Системные настройки
```json
{
  "system": {
    "device_name": "ESP32-AirQuality",
    "device_id": "",
    "led_enabled": true,
    "led_pin": 2,
    "led_inverted": false,
    "log_level": 3,
    "serial_log_enabled": true,
    "serial_baudrate": 115200,
    "watchdog_timeout": 10,
    "factory_reset_pin": false,
    "factory_reset_pin_num": 0,
    "factory_reset_hold_ms": 5000
  }
}
```

## 🚀 Быстрый старт

### 1. Загрузка конфигурации в SPIFFS

```bash
# Через PlatformIO
pio run -t uploadfs

# Или через esptool
esptool.py write_flash 0x310000 data/config.json
```

### 2. Редактирование через веб-интерфейс

1. Откройте `http://<ip-адрес-esp32>/config`
2. Перейдите по вкладкам и измените настройки
3. Нажмите "💾 Сохранить конфигурацию"

### 3. Редактирование через JSON редактор

1. Откройте `http://<ip-адрес-esp32>/config`
2. Перейдите на вкладку "📝 JSON редактор"
3. Отредактируйте JSON
4. Нажмите "📥 Загрузить в форму" для проверки
5. Нажмите "💾 Сохранить конфигурацию"

## 🔄 Миграция со старой системы (NVS/Preferences)

Старая система настроек через `Preferences` автоматически мигрирует в JSON:

1. При первом запуске `config_init()` проверяет наличие `/config.json`
2. Если файл не найден, создаётся конфигурация по умолчанию
3. Старые настройки из `settings.h` можно перенести вручную или через веб-интерфейс

## 🛠️ Программный API

### В коде прошивки

```cpp
#include "config.h"

// Инициализация
config_init();

// Загрузка
if (config_load()) {
  Serial.println("Config loaded");
}

// Получение конфигурации
Config& cfg = config_get();
Serial.println(cfg.wifi.ssid);
Serial.println(cfg.alerts.co2_warning);

// Изменение и сохранение
cfg.display.brightness = 90;
config_save();

// Сброс к дефолту
config_reset();

// Резервная копия
config_backup();

// Восстановление
config_restore();

// Получение JSON
char json[4096];
config_get_json(json, sizeof(json));
Serial.println(json);

// Установка из JSON
const char* newJson = R"({"wifi":{"ssid":"NewSSID"}})";
config_set_json(newJson);
config_save();
```

## ✅ Валидация

Система автоматически проверяет:
- Диапазоны значений (яркость 0-100, интервалы 1-3600 сек)
- Логичность порогов (warning < critical)
- Корректность IP-адресов
- Валидность JSON при парсинге

При ошибках значения заменяются на дефолтные.

## 📊 Мониторинг

Логи системы конфигурации:
```
INFO [SETTINGS] Initializing configuration system...
INFO [SETTINGS] Configuration loaded from /config.json
INFO [SETTINGS] Configuration validation passed
INFO [SETTINGS] Settings initialized from JSON config
```

## 🔐 Безопасность

⚠️ **Важно:**
- Пароли WiFi и MQTT хранятся в открытом виде в SPIFFS
- Включите `webserver.auth_enabled` для защиты веб-интерфейса
- Установите пароль OTA для защиты от несанкционированной прошивки
- Не коммитьте `config.json` с реальными паролями в Git

## 🎯 Преимущества новой системы

| Характеристика | Старая (NVS) | Новая (JSON) |
|---------------|-------------|--------------|
| Хранение | NVS partition | SPIFFS файл |
| Редактирование | Через веб-форму | Веб + прямой JSON |
| Резервное копирование | Нет | Есть |
| Массовое развёртывание | Сложно | Копирование файла |
| Версионирование | Нет | Git-friendly |
| Размер | Ограничено NVS | До 4KB |
| Скорость чтения | Быстро | Быстро |
| Скорость записи | Быстро | Медленнее (SPIFFS) |

## 📚 Связанные файлы

- `include/config.h` - Заголовочный файл с структурами
- `src/config.cpp` - Реализация функций
- `include/settings.h` - Обратная совместимость
- `src/settings.cpp` - Wrapper для старой системы
- `src/web.cpp` - Веб API эндпоинты
- `data/config.html` - Веб-интерфейс редактора
- `data/config.json.example` - Шаблон конфигурации
