This project implements an air quality monitoring system using the ESP32 microcontroller. It integrates various sensors to measure and log environmental data such as PM2.5, PM10, temperature, and humidity. The system features real-time data visualization, remote monitoring capabilities, and data storage on a cloud platform. Developed using C/C++ and the Arduino framework, it demonstrates proficiency in IoT development, sensor integration, and cloud connectivity.

## 📋 Оглавление

- [Поддерживаемые датчики](#-поддерживаемые-датчики)
- [Установка и настройка](#-установка-и-настройка)
- [Конфигурация](#-конфигурация)
- [Веб-интерфейс](#-веб-интерфейс)
- [Документация](#-документация)

## Поддерживаемые датчики

| Датчик | Модель | Назначение | Единицы измерения |
|--------|--------|-----------|-------------------|
| **BME280** | BME280 | Наружная температура, давление, влажность | °C, гПа (hPa), % |
| **HTU21DF** | HTU21DF | Внутренняя температура, влажность | °C, % |
| **SCD40** | SCD40 (Sensirion) | CO₂, температура, влажность | ppm, °C, % |
| **PMS5003** | PMS5003 | Твёрдые частицы PM1.0, PM2.5, PM10 | мкг/м³ (µg/m³) |
| **MS5611** | MS5611 | Точное барометрическое давление, температура | гПа (hPa), °C |
| **BH1750** | BH1750 | Освещённость | люкс (lux, лк) |
| **VEML6070** | VEML6070 | УФ-излучение | UV index (индекс) |
| **Формальдегид** | WS160725115 | Концентрация формальдегида (CH₂O) | ppm |
| **Микрофон** | INMP441 | Уровень шума | дБ (dB) |

### Диапазоны измерений и точность

| Параметр | Диапазон | Точность |
|----------|----------|----------|
| Температура (BME280/HTU21DF) | -40...+85 °C | ±0.5 °C |
| Температура (SCD40) | -10...+60 °C | ±0.5 °C |
| Влажность (BME280/HTU21DF) | 0...100 % | ±2-3 % |
| Влажность (SCD40) | 0...100 % | ±5 % |
| Давление (BME280/MS5611) | 300...1100 гПа | ±1 гПа |
| CO₂ (SCD40) | 400...5000 ppm | ±40 ppm ±5 % |
| PM1.0/PM2.5/PM10 | 0...500 мкг/м³ | ±10 % |
| Освещённость (BH1750) | 1...65535 лк | ±20 % |
| УФ-индекс (VEML6070) | 0...15 | ±10 % |
| Формальдегид (WS160725115) | 0...0.5 ppm | ±10 % |
| Шум (INMP441) | 30...100 дБ | ±2 дБ |

---

<img width="2523" height="920" alt="image" src="https://github.com/user-attachments/assets/1af0e709-f831-4d62-aefa-845e57ca557a" />
<img width="4369" height="2210" alt="bme_pressure_20260222_213930" src="https://github.com/user-attachments/assets/2b115d8a-ea4a-40bc-9287-2a4c06b9621c" />
<img width="4263" height="2210" alt="bme_temperature_20260222_213511" src="https://github.com/user-attachments/assets/584e9cc7-e25e-4868-91fa-8b1ac0738603" />
<img src="https://github.com/Kom60/esp32_airquality_system/blob/main/222.jpg" />
<img width="1234" height="763" alt="image" src="https://github.com/user-attachments/assets/8e18eee6-f9ba-4119-b5d3-224d0dc8f607" />
<img width="1237" height="852" alt="image" src="https://github.com/user-attachments/assets/8e68e797-f1c4-46e4-a7d1-0b030840f532" />

---

## 🔧 Установка и настройка

### 1. Клонирование репозитория

```bash
git clone <repository-url>
cd esp32_airquality_ILI9486
```

### 2. Установка PlatformIO

```bash
# Установка через VS Code
# Установите расширение "PlatformIO IDE"

# Или через pip
pip install platformio
```

### 3. Настройка секретных данных

```bash
# Скопируйте шаблон secrets.h.example в secrets.h
cp include/secrets.h.example include/secrets.h

# Отредактируйте secrets.h, указав ваши WiFi credentials
```

### 4. Загрузка прошивки

```bash
# Подключите ESP32 через USB
# Замените COM-порт на ваш
pio run -t upload -p COM3

# Загрузка файловой системы SPIFFS
pio run -t uploadfs
```

### 5. Настройка конфигурации

После первого запуска:
1. Подключитесь к WiFi сети
2. Откройте `http://<ip-адрес-esp32>/config`
3. Настройте параметры через веб-интерфейс

Или загрузите готовый конфиг:
```bash
# Отредактируйте data/config.json.example
# Переименуйте в config.json
# Загрузите через pio run -t uploadfs
```

---

## ⚙️ Конфигурация

Система использует JSON-конфигурацию, хранящуюся в SPIFFS.

### Основные разделы конфигурации:

- **📡 WiFi** - настройки сети, статический IP
- **🕐 NTP** - синхронизация времени
- **📊 Датчики** - интервалы опроса, калибровка, адреса I2C
- **🚨 Предупреждения** - пороги для CO₂, PM2.5, температуры и т.д.
- **🖥️ Дисплей** - яркость, ночной режим, поворот
- **💾 SD карта** - логирование данных
- **🌐 Сеть** - веб-сервер, WebSocket, MQTT
- **⚙️ Система** - имя устройства, логирование, OTA

### Быстрые команды:

```bash
# Получить конфигурацию
curl http://<ip>/api/config

# Сохранить конфигурацию
curl -X POST http://<ip>/api/config -d @config.json

# Создать резервную копию
curl -X POST http://<ip>/api/config/backup

# Сбросить к дефолту
curl -X DELETE http://<ip>/api/config
```

📖 **Полная документация:** [docs/CONFIG_JSON.md](docs/CONFIG_JSON.md)

---

## 🌐 Веб-интерфейс

| Страница | URL | Описание |
|----------|-----|----------|
| Главная | `/` | Дашборд с показаниями датчиков |
| Настройки | `/settings` | Старые настройки (обратная совместимость) |
| Конфигурация | `/config` | **Новый** полноценный редактор JSON |
| Графики | `/charts.html` | Исторические данные |

### Функции веб-интерфейса:

- ✅ Просмотр показаний в реальном времени
- ✅ Редактирование всех настроек
- ✅ Резервное копирование конфигурации
- ✅ OTA обновление прошивки
- ✅ Перезагрузка устройства

---

## 📚 Документация

| Файл | Описание |
|------|----------|
| [docs/CONFIG_JSON.md](docs/CONFIG_JSON.md) | Полное руководство по JSON конфигурации |
| [LOGGING.md](LOGGING.md) | Система логирования |
| [include/config.h](include/config.h) | Заголовочный файл конфигурации |
| [data/config.json.example](data/config.json.example) | Шаблон конфигурации |

---

## 🛠️ Устранение проблем

### Ошибка компиляции "secrets.h not found"

```bash
# Скопируйте шаблон
cp include/secrets.h.example include/secrets.h
```

### ESP32 не подключается к WiFi

1. Проверьте `secrets.h` - правильность SSID и пароля
2. Убедитесь, что WiFi 2.4GHz (ESP32 не поддерживает 5GHz)
3. Проверьте уровень сигнала

### SPIFFS ошибка при загрузке

```bash
# Очистите SPIFFS
pio run -t erase
# Загрузите заново
pio run -t uploadfs
```

### Сброс к заводским настройкам

1. Через веб-интерфейс: `/config` → "🔄 Сброс к дефолту"
2. Через API: `curl -X DELETE http://<ip>/api/config`
3. Аппаратный сброс: загрузите `data/config.json.example` через `uploadfs`

---

## 📈 Roadmap

Смотрите полный [ROADMAP](docs/ROADMAP.md) со следующими направлениями:

- 🔲 MQTT интеграция (Home Assistant)
- 🔲 Веб-графики (Chart.js)
- 🔲 Добавление SGP40 (TVOC)
- 🔲 OTA обновления
- 🔲 Telegram уведомления

---

## 📊 Лицензия

MIT License - свободное использование с указанием авторства.

---

## 🤝 Вклад в проект

1. Fork репозитория
2. Создайте feature branch (`git checkout -b feature/amazing-feature`)
3. Commit изменений (`git commit -m 'Add amazing feature'`)
4. Push в branch (`git push origin feature/amazing-feature`)
5. Откройте Pull Request

---

**Разработано с ❤️ для мониторинга качества воздуха**



