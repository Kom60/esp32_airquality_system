# Руководство по использованию SD карты

## 📋 Обзор

Проект полностью переведён на использование SD карты в качестве основной файловой системы. SPIFFS больше не используется.

### Что хранится на SD карте:
- ✅ **Веб-файлы** (HTML, CSS, JS, изображения)
- ✅ **Конфигурация** (config.json и резервные копии)
- ✅ **Логи данных** с датчиков (ежедневные файлы)
- ✅ **Прошивки** для OTA обновления

---

## 📁 Структура файлов на SD карте

```
SD:/
├── www/                    # Веб-файлы
│   ├── index.html
│   ├── settings.html
│   ├── config.html
│   ├── charts.html
│   ├── style.css
│   ├── main_style.css
│   ├── header_style.css
│   ├── dungeon.css
│   ├── charts_style.css
│   ├── logic.js
│   ├── js_logic.js
│   ├── settings_logic.js
│   ├── charts_logic.js
│   ├── manifest.json
│   ├── favicon.png
│   ├── esp32_logo.png
│   ├── on_bubl.png
│   └── off_bubl.png
├── config/
│   ├── config.json         # Основной файл конфигурации
│   └── config.backup.json  # Резервная копия
├── logs/                   # Логи данных с датчиков
│   ├── 2026-04-05.json     # Ежедневные файлы логов
│   ├── 2026-04-06.json
│   └── ...
└── firmware/               # Прошивки для OTA обновления
    └── firmware.bin        # Файл прошивки для обновления
```

---

## 🚀 Быстрый старт

### 1. Подготовка SD карты

#### Вариант A: Использование PowerShell скрипта (Windows)

```powershell
# Вставьте SD карту в ПК и узнайте её букву диска
# Запустите скрипт из корня проекта:
.\prepare_sd.ps1 -SdLetter F

# Где F - буква вашей SD карты
```

#### Вариант B: Ручное копирование

1. Откройте SD карту в проводнике
2. Создайте директории:
   - `/www/`
   - `/config/`
   - `/logs/`
   - `/firmware/`
3. Скопируйте все файлы из `data/` проекта в `/www/` на SD карте
4. Скопируйте `data/config.json.example` как `/config/config.json` на SD карте

### 2. Установка SD карты в ESP32

1. Извлеките SD карту из ПК
2. Вставьте в модуль SD карты ESP32
3. Убедитесь в правильности подключения пинов (CS = GPIO14)

### 3. Загрузка прошивки

```bash
# Из корня проекта
pio run -t upload

# Откройте Serial Monitor для проверки
pio device monitor
```

### 4. Проверка работы

В Serial Monitor вы должны увидеть:
```
[SD] Initializing SD card...
[SD] Card Mount OK
[SD] Card size: XXXX MB
[SD] Directory exists: /www
[SD] Directory exists: /config
[SD] Directory exists: /logs
[SD] Directory exists: /firmware
```

---

## 💾 Подготовка файлов для SD карты

### Копирование веб-файлов

Все HTML, CSS, JS и изображения должны быть скопированы в `/www/` на SD карте:

```bash
# PowerShell (Windows)
Copy-Item -Path "data\*" -Destination "F:\www\" -Recurse

# Или вручную через проводник
```

### Настройка конфигурации

1. Откройте `/config/config.json` на SD карте
2. Отредактируйте параметры (WiFi, датчики, и т.д.)
3. Сохраните файл

**Пример минимальной конфигурации:**
```json
{
  "version": "1.0.0",
  "wifi": {
    "ssid": "YourWiFiSSID",
    "password": "YourPassword"
  },
  "ntp": {
    "server": "pool.ntp.org",
    "gmt_offset_sec": 10800
  },
  "sensors": {
    "bme_interval": 5,
    "htu_interval": 5
  }
}
```

---

## 🔄 Обновление прошивки через веб-интерфейс

### Метод 1: Через веб-интерфейс

1. **Загрузка файла прошивки:**
   ```bash
   # Скомпилируйте прошивку
   pio run
   
   # Найдите файл прошивки в .pio/build/esp32doit-devkit-v1/
   # Обычно это firmware.bin
   ```

2. **Загрузите на SD через веб:**
   - Откройте `http://<ip-esp32>/api/firmware/upload`
   - Загрузите файл `firmware.bin` через POST запрос
   
   **Или используйте curl:**
   ```bash
   curl -X POST http://<ip-esp32>/api/firmware/upload \
        -F "file=@.pio/build/esp32doit-devkit-v1/firmware.bin"
   ```

3. **Проверьте статус:**
   ```bash
   curl http://<ip-esp32>/api/firmware/status
   ```

4. **Запустите обновление:**
   ```bash
   curl -X POST http://<ip-esp32>/api/firmware/flash
   ```

### Метод 2: Прямое копирование на SD карту

1. Скомпилируйте прошивку:
   ```bash
   pio run
   ```

2. Скопируйте файл прошивки в `/firmware/firmware.bin` на SD карте

3. Вставьте SD карту в ESP32 и используйте веб-API для прошивки

---

## 📊 Логирование данных

### Автоматическое создание файлов

Система автоматически создаёт ежедневные файлы логов:
- `/logs/2026-04-05.json`
- `/logs/2026-04-06.json`
- и т.д.

### Формат данных

Каждая запись - это JSON объект:
```json
{
  "timestamp": "2026-04-05 14:30:00",
  "sensors": {
    "co2": 450,
    "temperature_bme": 22.5,
    "humidity_bme": 45,
    "pm2_5": 12.3,
    "pm10": 18.7
  }
}
```

### Очистка старых логов

- Автоматическая очистка каждые 30 дней
- Максимум 30 файлов логов
- Максимальный размер одного файла: 32 МБ

---

## 🔧 API Эндпоинты

### Конфигурация

| Метод | Эндпоинт | Описание |
|-------|----------|----------|
| GET | `/api/config` | Получить текущую конфигурацию |
| POST | `/api/config` | Сохранить конфигурацию (JSON в теле) |
| DELETE | `/api/config` | Удалить конфигурацию |
| POST | `/api/config/backup` | Создать резервную копию |
| POST | `/api/config/restore` | Восстановить из резервной копии |

### Обновление прошивки

| Метод | Эндпоинт | Описание |
|-------|----------|----------|
| GET | `/api/firmware/status` | Получить статус прошивки на SD |
| POST | `/api/firmware/upload` | Загрузить файл прошивки на SD |
| POST | `/api/firmware/flash` | Прошить ESP32 из файла на SD |
| DELETE | `/api/firmware/delete` | Удалить файл прошивки с SD |

### Примеры использования

**Получить конфигурацию:**
```bash
curl http://<ip>/api/config
```

**Сохранить конфигурацию:**
```bash
curl -X POST http://<ip>/api/config \
     -H "Content-Type: application/json" \
     -d '{"wifi":{"ssid":"MyWiFi","password":"12345678"}}'
```

**Создать резервную копию:**
```bash
curl -X POST http://<ip>/api/config/backup
```

---

## ⚠️ Важные замечания

### Надёжность SD карт

1. **Используйте качественные карты** Class 10 от проверенных производителей
2. **Избегайте дешёвых no-name карт** - они быстрее выходят из строя
3. **Рекомендуемый объём:** 8-16 ГБ (не имеет смысла больше)

### Защита от потери данных

- Всегда закрывайте файлы правильно (система делает это автоматически)
- Резервная копия `config.json` создаётся автоматически
- Регулярно копируйте логи на ПК для анализа

### Безопасное извлечение

Перед извлечением SD карты:
1. Остановите логирование (отключите питание ESP32)
2. Извлеките карту
3. Скопируйте данные на ПК
4. Верните карту в ESP32

---

## 🐛 Устранение проблем

### SD карта не монтируется

**Симптомы:**
```
[SD] Card Mount Failed
```

**Решения:**
1. Проверьте подключение пинов (CS, MOSI, MISO, SCLK)
2. Убедитесь, что SD карта отформатирована в FAT32
3. Попробуйте другую SD карту
4. Проверьте питание модуля SD карты

### Файлы не найдены

**Симптомы:**
```
404: index.html not found on SD card
```

**Решения:**
1. Проверьте структуру папок на SD карте
2. Убедитесь, что файлы находятся в `/www/`
3. Запустите `prepare_sd.ps1` скрипт заново

### Конфигурация не загружается

**Симптомы:**
```
[SETTINGS] Config file not found or empty
```

**Решения:**
1. Проверьте наличие `/config/config.json` на SD карте
2. Убедитесь, что файл валидный JSON
3. Используйте `config.json.example` как шаблон

### Логи не создаются

**Симптомы:**
- Папка `/logs/` пустая

**Решения:**
1. Проверьте, что прошло 40 секунд после запуска (ожидание NTP)
2. Проверьте права записи на SD карту
3. Посмотрите логи в Serial Monitor

---

## 📝 Технические детали

### Изменённые файлы

| Файл | Изменения |
|------|-----------|
| `src/sdcard.cpp` | Полностью переписан для SD карты |
| `src/config.cpp` | Заменён SPIFFS на SD |
| `src/web.cpp` | Раздача файлов с SD вместо SPIFFS |
| `src/main.cpp` | Убран вызов spiffs_setup() |
| `src/settings.cpp` | Заменён SPIFFS на SD |
| `src/system_init.cpp` | Удалена функция spiffs_setup() |
| `include/headers.h` | Заменён #include <SPIFFS.h> на <SD.h> |
| `include/sdcard.h` | Добавлены константы и функции для SD |
| `include/config.h` | Обновлены пути конфигурации |

### Распиновка SD карты

| Сигнал | GPIO | Примечание |
|--------|------|------------|
| CS | GPIO14 | Chip Select |
| MOSI | GPIO23 | Master Out Slave In |
| MISO | GPIO19 | Master In Slave Out |
| SCLK | GPIO18 | Clock |
| VCC | 3.3V | Питание |
| GND | GND | Земля |

> **Примечание:** SD карта использует ту же SPI шину, что и дисплей ILI9486

---

## 📚 Дополнительные ресурсы

- [ESP32 SD Card Interface](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/sdspi_host.html)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [Arduino SD Library](https://www.arduino.cc/en/reference/SD)

---

## 🔄 Миграция с SPIFFS

Если у вас уже есть работающая система с SPIFFS:

1. **Сделайте резервную копию config.json:**
   ```bash
   curl http://<ip>/api/config/backup
   ```

2. **Подготовьте SD карту** с помощью скрипта `prepare_sd.ps1`

3. **Загрузите новую прошивку:**
   ```bash
   pio run -t upload
   ```

4. **Вставьте SD карту** и перезагрузите ESP32

5. **Проверьте работу** через Serial Monitor и веб-интерфейс

---

**Дата последнего обновления:** 5 апреля 2026 г.
