#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <FS.h>

// ============================================================================
// Версия конфигурации
// ============================================================================
#define CONFIG_VERSION "1.0.0"
#define CONFIG_MAX_SIZE 8192  // Максимальный размер JSON (увеличено с 4096)

// ============================================================================
// Структуры конфигурации
// ============================================================================

// WiFi настройки
struct WiFiConfig {
  char ssid[33] = "";
  char password[65] = "";
  int reconnect_interval = 30;        // секунды между попытками подключения
  int reconnect_max_attempts = 0;     // 0 = бесконечно
  bool auto_connect = true;
  char static_ip[16] = "";            // пустая строка = DHCP
  char gateway[16] = "";
  char subnet[16] = "";
  char dns1[16] = "";
  char dns2[16] = "";
};

// NTP настройки
struct NTPConfig {
  char server[64] = "pool.ntp.org";
  int32_t gmt_offset_sec = 3 * 3600;  // Москва GMT+3
  int daylight_offset_sec = 0;
  bool auto_sync = true;
  int sync_interval = 3600;           // секунды между синхронизациями
};

// Настройки датчиков
struct SensorsConfig {
  // Интервалы опроса (секунды)
  int bme_interval = 5;
  int htu_interval = 5;
  int scd_interval = 10;
  int pms_interval = 5;
  int ms_interval = 10;
  int bh_interval = 5;
  int veml_interval = 10;
  int ch2o_interval = 10;
  int mic_interval = 5;
  int ina_interval = 10;

  // Калибровка температуры (°C)
  float temp_offset_bme = 0.0;
  float temp_offset_htu = 0.0;
  float temp_offset_scd = 0.0;

  // Калибровка влажности (%)
  int hum_offset_bme = 0;
  int hum_offset_htu = 0;
  int hum_offset_scd = 0;

  // Калибровка давления (гПа)
  int press_offset_bme = 0;
  int press_offset_ms = 0;

  // Включение/отключение датчиков
  bool bme_enabled = true;
  bool htu_enabled = true;
  bool scd_enabled = true;
  bool pms_enabled = true;
  bool ms_enabled = true;
  bool bh_enabled = true;
  bool veml_enabled = true;
  bool ch2o_enabled = true;
  bool mic_enabled = true;
  bool ina_enabled = true;

  // Адреса I2C (по умолчанию)
  int bme_i2c_addr = 0x76;
  int htu_i2c_addr = 0x40;
  int scd_i2c_addr = 0x62;
  int pms_baudrate = 9600;
  int ms_i2c_addr = 0x76;
  int bh_i2c_addr = 0x23;
  int veml_i2c_addr = 0x38;
  int ch2o_i2c_addr = 0x40;
  int ina_i2c_addr = 0x40;
};

// Пороги предупреждений
struct AlertsConfig {
  // CO2 (ppm)
  int co2_warning = 1000;
  int co2_critical = 1400;

  // PM2.5 (мкг/м³)
  int pm25_warning = 35;
  int pm25_critical = 50;

  // PM10 (мкг/м³)
  int pm10_warning = 50;
  int pm10_critical = 100;

  // Температура (°C)
  float temp_warning_low = 15.0;
  float temp_warning_high = 30.0;
  float temp_critical_low = 5.0;
  float temp_critical_high = 40.0;

  // Влажность (%)
  int hum_warning_low = 30;
  int hum_warning_high = 70;
  int hum_critical_low = 20;
  int hum_critical_high = 80;

  // Давление (гПа)
  int press_warning_low = 980;
  int press_warning_high = 1050;

  // Формальдегид (ppm)
  float ch2o_warning = 0.05;
  float ch2o_critical = 0.10;

  // UV индекс
  int uv_warning = 6;
  int uv_critical = 10;

  // Шум (дБ)
  int noise_warning = 70;
  int noise_critical = 85;

  // Включение предупреждений
  bool co2_alerts_enabled = true;
  bool pm_alerts_enabled = true;
  bool temp_alerts_enabled = true;
  bool hum_alerts_enabled = true;
  bool press_alerts_enabled = false;
  bool ch2o_alerts_enabled = true;
  bool uv_alerts_enabled = false;
  bool noise_alerts_enabled = true;
};

// Настройки дисплея
struct DisplayConfig {
  bool enabled = true;
  int brightness = 80;              // 0-100%
  int brightness_night = 20;        // ночная яркость
  int night_mode_start = 23;        // час начала ночного режима
  int night_mode_end = 7;         // час окончания ночного режима
  int screen_timeout = 300;         // секунды до отключения (0 = никогда)
  int update_interval = 2;          // секунды между обновлениями
  bool auto_rotate = false;
  int rotation = 3;                 // 0-3
};

// Настройки SD карты
struct SDConfig {
  bool enabled = true;
  int cs_pin = 5;
  bool auto_log = true;
  int log_interval = 10;            // секунды между записями
  char log_format[16] = "csv";      // csv, json
  bool create_hourly_files = false;
  int max_file_size_mb = 32;        // макс. размер файла перед ротацией
  bool auto_backup = true;
  int backup_interval_hours = 24;
};

// Настройки SPIFFS
struct SPIFFSConfig {
  bool format_on_error = true;
  int max_open_files = 5;
};

// Веб-сервер
struct WebServerConfig {
  bool enabled = true;
  int port = 80;
  bool cors_enabled = true;
  char cors_origin[64] = "*";
  int max_clients = 5;
  bool auth_enabled = false;
  char auth_username[33] = "admin";
  char auth_password[33] = "";      // пустая = без пароля
  int session_timeout = 3600;       // секунды
};

// WebSocket
struct WebSocketConfig {
  bool enabled = true;
  int port = 81;
  int ping_interval = 30;           // секунды между ping
  int max_clients = 10;
  bool broadcast_enabled = true;
  int broadcast_interval = 2;       // секунды между рассылками
};

// MQTT (будущая функция)
struct MQTTConfig {
  bool enabled = false;
  char broker[128] = "";
  int port = 1883;
  char username[64] = "";
  char password[64] = "";
  char client_id[64] = "";
  char topic_prefix[64] = "esp32_airquality";
  int qos = 0;
  bool retain = true;
  int keepalive = 60;
  bool auto_reconnect = true;
};

// OTA обновления
struct OTAConfig {
  bool enabled = true;
  char password[65] = "";           // пустая = без пароля
  int port = 3232;
  bool reboot_on_success = true;
};

// Системные настройки
struct SystemConfig {
  char device_name[33] = "ESP32-AirQuality";
  char device_id[17] = "";          // пустая = MAC адрес
  bool led_enabled = true;
  int led_pin = 2;
  bool led_inverted = false;
  int log_level = 3;                // 0=NONE, 1=ERROR, 2=WARNING, 3=INFO, 4=DEBUG
  bool serial_log_enabled = true;
  int serial_baudrate = 115200;
  int watchdog_timeout = 10;        // секунды (0 = отключен)
  bool factory_reset_pin = false;
  int factory_reset_pin_num = 0;
  int factory_reset_hold_ms = 5000;
};

// Главная структура конфигурации
struct Config {
  char version[16] = CONFIG_VERSION;
  uint32_t config_id = 0;           // Уникальный ID для отслеживания изменений
  
  WiFiConfig wifi;
  NTPConfig ntp;
  SensorsConfig sensors;
  AlertsConfig alerts;
  DisplayConfig display;
  SDConfig sd;
  SPIFFSConfig spiffs;
  WebServerConfig webserver;
  WebSocketConfig websocket;
  MQTTConfig mqtt;
  OTAConfig ota;
  SystemConfig system;
};

// ============================================================================
// Функции
// ============================================================================

/**
 * @brief Инициализация системы конфигурации
 * Загружает конфиг с SD карты или создаёт дефолтный
 */
void config_init();

/**
 * @brief Загрузка конфигурации из файла на SD карте
 * @return true если успешно
 */
bool config_load();

/**
 * @brief Сохранение конфигурации на SD карту
 * @return true если успешно
 */
bool config_save();

/**
 * @brief Сохранение конфигурации в файл с новым именем (для бэкапа)
 * @param filename Имя файла на SD карте
 * @return true если успешно
 */
bool config_save_as(const char* filename);

/**
 * @brief Сброс к заводским настройкам
 */
void config_reset();

/**
 * @brief Валидация конфигурации
 * @return true если конфигурация корректна
 */
bool config_validate();

/**
 * @brief Получить конфигурацию как JSON строку
 * @param buffer Буфер для записи
 * @param size Размер буфера
 * @return длина JSON строки
 */
size_t config_get_json(char* buffer, size_t size);

/**
 * @brief Обновить конфигурацию из JSON строки
 * @param json JSON строка
 * @return true если успешно
 */
bool config_set_json(const char* json);

/**
 * @brief Получить текущий объект конфигурации
 * @return Ссылка на глобальный конфиг
 */
Config& config_get();

/**
 * @brief Проверка наличия файла конфигурации
 * @return true если файл существует
 */
bool config_exists();

/**
 * @brief Удалить файл конфигурации
 * @return true если успешно
 */
bool config_delete();

/**
 * @brief Создать резервную копию конфигурации
 * @return true если успешно
 */
bool config_backup();

/**
 * @brief Восстановить конфигурацию из резервной копии
 * @return true если успешно
 */
bool config_restore();

#endif // CONFIG_H
