# Резюме изменений: Переход с SPIFFS на SD карту

## 📋 Что было сделано

### 1. Обновлённые файлы

#### `src/sdcard.cpp` - Полностью переписан
- ✅ Добавлено автоматическое создание структуры папок (`/www`, `/config`, `/logs`, `/firmware`)
- ✅ Улучшено логирование - теперь создаются ежедневные файлы логов (`/logs/2026-04-05.json`)
- ✅ Автоматическая очистка старых логов (макс. 30 файлов)
- ✅ Проверка размера файлов логов (макс. 32 МБ)
- ✅ Добавлена функция `sdcard_is_ready()` для проверки состояния

#### `src/config.cpp` - Обновлён для работы с SD
- ✅ Заменены все `SPIFFS.open()` → `SD.open()`
- ✅ Заменены пути: `CONFIG_FILE_PATH` → `SD_CONFIG_FILE` (`/config/config.json`)
- ✅ Заменены пути: backup → `SD_CONFIG_BACKUP` (`/config/config.backup.json`)
- ✅ Добавлены проверки `sdcard_is_ready()`
- ✅ Добавлен `file.flush()` для надёжной записи

#### `src/web.cpp` - Раздача файлов с SD
- ✅ Все маршруты теперь используют `SD` вместо `SPIFFS`
- ✅ Добавлены отдельные обработчики для каждого файла (CSS, JS, изображения)
- ✅ Добавлен маршрут `/charts.html`
- ✅ Добавлен полный API для обновления прошивки:
  - `GET /api/firmware/status` - статус прошивки
  - `POST /api/firmware/upload` - загрузка файла прошивки
  - `POST /api/firmware/flash` - запуск обновления
  - `DELETE /api/firmware/delete` - удаление файла прошивки

#### `src/main.cpp` - Удалён SPIFFS
- ✅ Убран вызов `spiffs_setup()`
- ✅ Все логи заменены с `SPIFFS` на `SD`

#### `src/system_init.cpp` - Удалена функция SPIFFS
- ✅ Удалена функция `spiffs_setup()`
- ✅ Осталась только `app_tasks_init()`

#### `src/settings.cpp` - Обновлён
- ✅ Заменён `#include <SPIFFS.h>` → `#include "sdcard.h"`
- ✅ Заменена проверка `SPIFFS.begin()` → `sdcard_is_ready()`

#### `include/headers.h`
- ✅ Заменён `#include <SPIFFS.h>` → `#include <SD.h>`

#### `include/sdcard.h`
- ✅ Добавлены константы путей:
  - `SD_WWW_PATH "/www"`
  - `SD_CONFIG_PATH "/config"`
  - `SD_LOGS_PATH "/logs"`
  - `SD_FIRMWARE_PATH "/firmware"`
  - `SD_CONFIG_FILE "/config/config.json"`
  - `SD_CONFIG_BACKUP "/config/config.backup.json"`
- ✅ Добавлены функции:
  - `sdcard_create_dirs()`
  - `sdcard_file_exists()`
  - `sdcard_write_log_data()`
  - `sdcard_cleanup_old_logs()`
  - `sdcard_is_ready()`

#### `include/config.h`
- ✅ Удалён `CONFIG_FILE_PATH` (перенесён в `sdcard.h`)
- ✅ Обновлены комментарии

#### `include/system_init.h`
- ✅ Удалено объявление `spiffs_setup()`

#### `include/webheaders.h`
- ✅ Добавлены объявления функций обновления прошивки

#### `include/logger.h`
- ✅ Заменены `LOG_SPIFFS` → `LOG_SD`
- ✅ Заменены `SPIFFS_LOG_LEVEL` → `SD_LOG_LEVEL`

#### `platformio.ini`
- ✅ Удалён `-D LOG_SPIFFS=1`
- ✅ Заменён `-D SPIFFS_LOG_LEVEL=2` → `-D SD_LOG_LEVEL=3`
- ✅ Убран дубликат `SD_LOG_LEVEL`

---

### 2. Новые файлы

#### `prepare_sd.ps1` - PowerShell скрипт
Автоматическое копирование файлов из `data/` на SD карту с созданием правильной структуры.

**Использование:**
```powershell
.\prepare_sd.ps1 -SdLetter F
```

#### `docs/SD_CARD_GUIDE.md` - Полная документация
Подробное руководство по использованию SD карты с примерами.

---

## 🎯 Что теперь работает иначе

### До изменений:
- Веб-файлы хранились в SPIFFS
- Конфигурация в SPIFFS (`/config.json`)
- Логи писались в один файл `/data.json`
- Не было обновления прошивки через веб

### После изменений:
- Веб-файлы на SD карте (`/www/`)
- Конфигурация на SD (`/config/config.json`)
- Логи пишутся в ежедневные файлы (`/logs/YYYY-MM-DD.json`)
- Доступно обновление прошивки через веб-API
- Автоматическое создание структуры папок
- Автоматическая очистка старых логов

---

## 📡 API Эндпоинты

### Конфигурация (без изменений)
- `GET /api/config` - получить конфигурацию
- `POST /api/config` - сохранить конфигурацию
- `DELETE /api/config` - удалить конфигурацию
- `POST /api/config/backup` - создать резервную копию
- `POST /api/config/restore` - восстановить из резервной копии

### Обновление прошивки (НОВОЕ)
- `GET /api/firmware/status` - получить статус прошивки
- `POST /api/firmware/upload` - загрузить файл прошивки на SD
- `POST /api/firmware/flash` - запустить обновление
- `DELETE /api/firmware/delete` - удалить файл прошивки

---

## 🚀 Как использовать

### 1. Подготовка SD карты

**Автоматически (рекомендуется):**
```powershell
.\prepare_sd.ps1 -SdLetter F
```

**Вручную:**
1. Создайте папки: `/www/`, `/config/`, `/logs/`, `/firmware/`
2. Скопируйте файлы из `data/` в `/www/`
3. Скопируйте `data/config.json.example` в `/config/config.json`

### 2. Загрузка прошивки

```bash
# Через PlatformIO
pio run -t upload

# Или через VS Code PlatformIO
```

### 3. Проверка работы

Откройте Serial Monitor и убедитесь в появлении сообщений:
```
[SD] Card Mount OK
[SD] Directory exists: /www
[SD] Directory exists: /config
[SD] Directory exists: /logs
[SD] Directory exists: /firmware
```

### 4. Обновление прошивки через веб

```bash
# 1. Загрузите файл прошивки
curl -X POST http://<ip>/api/firmware/upload \
     -F "file=@firmware.bin"

# 2. Проверьте статус
curl http://<ip>/api/firmware/status

# 3. Запустите обновление
curl -X POST http://<ip>/api/firmware/flash
```

---

## ⚠️ Важные замечания

1. **SD карта обязательна** - без неё система не загрузит веб-файлы и конфигурацию
2. **Форматирование** - SD карта должна быть в FAT32
3. **Качество карты** - используйте Class 10 от проверенных производителей
4. **Безопасное извлечение** - перед извлечением отключите питание ESP32

---

## 🔍 Проверка работоспособности

Перед загрузкой на устройство убедитесь:

- ✅ Все `.cpp` файлы компилируются без ошибок
- ✅ Нет зависимостей от `SPIFFS.h`
- ✅ Все пути к файлам правильные
- ✅ SD карта подготовлена с правильной структурой
- ✅ Файлы скопированы в `/www/` на SD карте

---

## 📝 Дальнейшие улучшения

Возможные следующие шаги:

1. Добавить веб-интерфейс для загрузки прошивки через форму
2. Добавить автоматическую загрузку прошивки при обнаружении нового файла
3. Добавить шифрование конфигурации
4. Добавить поддержку сжатия веб-файлов (gzip)
5. Добавить мониторинг свободного места на SD карте

---

**Дата создания:** 5 апреля 2026 г.
**Автор:** AI Assistant
