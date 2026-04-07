# Инструкция по деплою оптимизаций HTTPS

## 📦 Что изменилось

### Критические исправления (ERR_CONNECTION_RESET):
1. **Увеличено соединений HTTPS**: 2 → **8** (браузеру нужно 6+)
2. **Убраны блокирующие CDN из `<head>`**:
   - Google Fonts → загружается асинхронно (`preload`)
   - Chart.js → перемещён в конец `<body>` с `defer`
3. **Оптимизирован буфер SSL**: 512 байт (баланс скорость/RAM)
4. **Добавлена обработка ошибок** при отключении клиента

### Улучшения скорости:
- Мгновенная загрузка данных через `fetchInitialData()`
- Анимированные индикаторы "⋯" вместо "---"
- Кэширование статики (1 час)
- Экспоненциальный реконнект WebSocket

## 🚀 Шаги деплоя

### 1. Прошивка ESP32

```powershell
# Подключите ESP32 по USB
# Определите COM-порт (в диспетчере устройств)

# Прошивка через PlatformIO
C:\Users\hf_kk\.platformio\penv\Scripts\platformio.exe run -t upload -e esp32doit-devkit-v1

# Или через VSCode: Ctrl+Alt+U (Upload)
```

### 2. Копирование файлов на SD карту

```powershell
# Вставьте SD карту в компьютер
# Определите букву диска (например, L:)

# Запустите скрипт подготовки SD
.\prepare_sd.ps1 -SdLetter L

# Или вручную скопируйте:
# data/ → L:/www/
# Убедитесь что есть SSL сертификаты: L:/ssl/server.crt, L:/ssl/server.key
```

### 3. Проверка файлов на SD

Обязательно должны быть:
```
L:/
├── www/
│   ├── index.html          ← ОБНОВЛЁННЫЙ
│   ├── style.css
│   ├── main_style.css
│   ├── header_style.css
│   ├── dungeon.css
│   ├── logic.js            ← ОБНОВЛЁННЫЙ
│   ├── charts_logic.js
│   ├── charts_style.css
│   ├── settings.html
│   ├── settings_style.css
│   ├── settings_logic.js
│   ├── config.html
│   └── favicon.png
└── ssl/
    ├── server.crt
    └── server.key
```

### 4. Перезагрузка ESP32

```
- Отключите питание ESP32
- Вставьте SD карту
- Включите питание
```

### 5. Тестирование

#### HTTP режим (порт 80):
```
http://<ESP32_IP>/
```

#### HTTPS режим (порт 443):
```
https://<ESP32_IP>/
```

⚠️ **При первом посещении HTTPS** браузер покажет предупреждение о самоподписанном сертификате:
- Нажмите "Дополнительно" → "Перейти на сайт (небезопасно)"
- Или установите сертификат в доверенные

## ✅ Ожидаемое поведение

### До оптимизаций:
- ❌ Страница грузилась 5-10 секунд
- ❌ CSS прогружался после нескольких перезагрузок
- ❌ Данные показывали "---" долго
- ❌ Ошибки `ERR_CONNECTION_RESET` в консоли

### После оптимизаций:
- ✅ Страница грузится 1-2 секунды
- ✅ CSS загружается сразу (нет блокирующих ресурсов)
- ✅ Данные появляются через 0.5-1 сек (fetchInitialData)
- ✅ Пульсирующий "⋯" пока данные грузятся
- ✅ Нет ошибок соединений (8 соединений)

## 🔍 Мониторинг

Откройте Serial Monitor для просмотра логов:

```powershell
# PlatformIO monitor
pio device monitor

# Или через VSCode: Ctrl+Alt+M
```

Ожидаемые логи:
```
[WEBSERVER] ✓ HTTPS server started on port 443 (max 8 clients)
[WEBSERVER]   Free heap after server start: 150000 bytes
[WEBSERVER] Sent /www/index.html (15234 bytes)
[WEBSERVER] Sent /style.css (9876 bytes)
```

## 🐛 Решение проблем

### ERR_CONNECTION_RESET всё ещё появляется:

1. **Проверьте свободную память:**
   - Откройте `https://<ESP32_IP>/api/sensors`
   - Поле `esp32_free_heap` должно быть > 100 KB

2. **Уменьшите соединения (если нужно):**
   ```cpp
   // В https_server.cpp строка ~309
   httpsServer = new HTTPSServer(cert, cfg.ssl.https_port, 6);  // Было 8
   ```

3. **Проверьте SD карту:**
   - Скорость чтения должна быть > 1 MB/s
   - Файловая система: FAT32 (не exFAT)

### Стили не загружаются:

1. **Очистите кэш браузера:**
   - `Ctrl + Shift + Delete` → Кэшированные изображения и файлы
   - Или `Ctrl + F5` (жесткая перезагрузка)

2. **Проверьте файлы на SD:**
   ```powershell
   # Все CSS файлы должны существовать
   Test-Path L:/www/style.css
   Test-Path L:/www/main_style.css
   Test-Path L:/www/header_style.css
   Test-Path L:/www/dungeon.css
   ```

### Данные не появляются:

1. **Проверьте API:**
   ```
   https://<ESP32_IP>/api/sensors
   ```
   Должен вернуть JSON со всеми датчиками.

2. **Проверьте WebSocket:**
   - Откройте консоль браузера (F12)
   - Должно быть: `[WS] WebSocket connected`

3. **Проверьте датчики:**
   - Убедитесь что датчики подключены
   - Serial Monitor покажет инициализацию

## 📊 Метрики производительности

### Размер файлов:
- index.html: ~15 KB
- style.css: ~10 KB
- main_style.css: ~5 KB
- header_style.css: ~3 KB
- dungeon.css: ~2 KB
- logic.js: ~55 KB
- **ИТОГО**: ~90 KB

### Время загрузки (ориентировочно):
- HTTP: 1-2 секунды
- HTTPS: 1.5-3 секунды (SSL handshake +0.5-1с)

### Потребление RAM:
- HTTPS сервер (8 соединений): ~40-50 KB
- SSL буферы: ~20 KB
- Свободная память: > 120 KB (норма)

## 🎯 Следующие улучшения (опционально)

1. **Сжать CSS/JS** (gzip на SD):
   ```powershell
   Compress-Archive -Path data/www/*.css -DestinationPath data/www/compressed
   ```

2. **Объединить CSS в один файл** (уменьшит соединения):
   ```powershell
   Get-Content data/www/header_style.css,data/www/style.css,data/www/main_style.css,data/www/dungeon.css | Set-Content data/www/all_styles.css
   ```

3. **Добавить Service Worker** для offline работы

4. **HTTP/2** (требуется другая библиотека)

---

**Дата обновления**: 7 апреля 2026  
**Версия**: 2.0.0 (HTTPS Optimized)  
**Автор**: AI Assistant
