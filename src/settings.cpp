#include "headers.h"
#include "settings.h"
#include "config.h"
#include <SPIFFS.h>

Preferences preferences;
StationSettings settings;

/**
 * @brief Инициализация системы настроек
 * Теперь использует JSON конфигурацию вместо NVS/Preferences
 */
void settings_init() {
  LOG_INFO(SETTINGS, "Initializing settings (JSON config mode)...");
  
  // Инициализация SPIFFS должна быть выполнена до этого
  if (!SPIFFS.begin(true)) {
    LOG_ERROR(SETTINGS, "Failed to mount SPIFFS, settings may not work");
    return;
  }
  
  // Инициализация JSON конфигурации
  config_init();
  
  // Загрузка настроек из конфигурации
  settings_load();
  
  LOG_INFO(SETTINGS, "Settings initialized from JSON config");
}

/**
 * @brief Загрузка настроек из JSON конфигурации
 */
void settings_load() {
  // Загрузка из глобальной конфигурации
  settings.load_from_config();
  
  LOG_INFO(SETTINGS, "Settings loaded from JSON config");
  LOG_INFO_FMT(SETTINGS, "WiFi SSID: %s", settings.wifi_ssid);
  LOG_INFO_FMT(SETTINGS, "CO2 thresholds: %d/%d ppm", settings.co2_warning, settings.co2_critical);
  LOG_INFO_FMT(SETTINGS, "PM2.5 thresholds: %d/%d µg/m³", settings.pm25_warning, settings.pm25_critical);
}

/**
 * @brief Сохранение настроек в JSON конфигурацию
 */
void settings_save() {
  // Сохранение в глобальную конфигурацию
  settings.save_to_config();
  
  LOG_INFO(SETTINGS, "Settings saved to JSON config");
}

/**
 * @brief Сброс настроек к заводским
 */
void settings_reset() {
  LOG_INFO(SETTINGS, "Resetting settings to defaults...");
  
  // Сброс JSON конфигурации
  config_reset();
  
  // Перезагрузка настроек
  settings_load();
  
  LOG_INFO(SETTINGS, "Settings reset to defaults");
}
