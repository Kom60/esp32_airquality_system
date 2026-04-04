#ifndef settings_h
#define settings_h

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// ============================================================================
// Обратная совместимость: структура StationSettings
// Теперь использует данные из JSON конфигурации
// ============================================================================

// Глобальный объект настроек (для обратной совместимости)
// Теперь это wrapper вокруг config_get()
struct StationSettings {
  // Интервал обновления
  int update_interval = 10;  // секунды

  // WiFi настройки
  char wifi_ssid[33] = "";
  char wifi_password[65] = "";

  // Калибровка температуры (°C)
  float temp_offset_bme = 0.0;
  float temp_offset_htu = 0.0;
  float temp_offset_scd = 0.0;

  // Калибровка влажности (%)
  int hum_offset_bme = 0;
  int hum_offset_htu = 0;

  // Калибровка давления (гПа)
  int press_offset_bme = 0;
  int press_offset_ms = 0;

  // Пороги CO₂ (ppm)
  int co2_warning = 1000;
  int co2_critical = 1400;

  // Пороги PM2.5 (мкг/м³)
  int pm25_warning = 35;
  int pm25_critical = 50;

  // Ночной режим дисплея
  int night_mode_start = 23;
  int night_mode_end = 7;

  /**
   * @brief Обновить значения из JSON конфигурации
   */
  void load_from_config() {
    Config& cfg = config_get();
    
    update_interval = 10;  // можно добавить в Config
    
    strlcpy(wifi_ssid, cfg.wifi.ssid, sizeof(wifi_ssid));
    strlcpy(wifi_password, cfg.wifi.password, sizeof(wifi_password));
    
    temp_offset_bme = cfg.sensors.temp_offset_bme;
    temp_offset_htu = cfg.sensors.temp_offset_htu;
    temp_offset_scd = cfg.sensors.temp_offset_scd;
    
    hum_offset_bme = cfg.sensors.hum_offset_bme;
    hum_offset_htu = cfg.sensors.hum_offset_htu;
    
    press_offset_bme = cfg.sensors.press_offset_bme;
    press_offset_ms = cfg.sensors.press_offset_ms;
    
    co2_warning = cfg.alerts.co2_warning;
    co2_critical = cfg.alerts.co2_critical;
    
    pm25_warning = cfg.alerts.pm25_warning;
    pm25_critical = cfg.alerts.pm25_critical;
    
    night_mode_start = cfg.display.night_mode_start;
    night_mode_end = cfg.display.night_mode_end;
  }

  /**
   * @brief Сохранить значения в JSON конфигурацию
   */
  void save_to_config() {
    Config& cfg = config_get();
    
    strlcpy(cfg.wifi.ssid, wifi_ssid, sizeof(cfg.wifi.ssid));
    strlcpy(cfg.wifi.password, wifi_password, sizeof(cfg.wifi.password));
    
    cfg.sensors.temp_offset_bme = temp_offset_bme;
    cfg.sensors.temp_offset_htu = temp_offset_htu;
    cfg.sensors.temp_offset_scd = temp_offset_scd;
    
    cfg.sensors.hum_offset_bme = hum_offset_bme;
    cfg.sensors.hum_offset_htu = hum_offset_htu;
    
    cfg.sensors.press_offset_bme = press_offset_bme;
    cfg.sensors.press_offset_ms = press_offset_ms;
    
    cfg.alerts.co2_warning = co2_warning;
    cfg.alerts.co2_critical = co2_critical;
    
    cfg.alerts.pm25_warning = pm25_warning;
    cfg.alerts.pm25_critical = pm25_critical;
    
    cfg.display.night_mode_start = night_mode_start;
    cfg.display.night_mode_end = night_mode_end;
    
    config_save();
  }
};

// Глобальный объект настроек
extern StationSettings settings;

// Функции для работы с настройками (для обратной совместимости)
void settings_init();
void settings_load();
void settings_save();
void settings_reset();

// Новые функции для работы с JSON конфигурацией
// (доступны через config.h)

#endif
