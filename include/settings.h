#ifndef settings_h
#define settings_h

#include <Arduino.h>
#include <Preferences.h>

// Структура настроек
struct StationSettings {
  // Интервал обновления
  int update_interval = 90;  // секунды
  
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
};

// Глобальный объект настроек
extern StationSettings settings;

// Функции для работы с настройками
void settings_init();
void settings_load();
void settings_save();
void settings_reset();

#endif
