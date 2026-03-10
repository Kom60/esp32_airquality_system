#include "settings.h"

Preferences preferences;
StationSettings settings;

void settings_init() {
  preferences.begin("meteostation", false);
  settings_load();
}

void settings_load() {
  // Интервал обновления
  settings.update_interval = preferences.getInt("update_interval", 10);
  
  // WiFi настройки
  preferences.getString("wifi_ssid", settings.wifi_ssid, 33);
  preferences.getString("wifi_password", settings.wifi_password, 65);
  
  // Калибровка температуры
  settings.temp_offset_bme = preferences.getFloat("temp_offset_bme", 0.0);
  settings.temp_offset_htu = preferences.getFloat("temp_offset_htu", 0.0);
  settings.temp_offset_scd = preferences.getFloat("temp_offset_scd", 0.0);
  
  // Калибровка влажности
  settings.hum_offset_bme = preferences.getInt("hum_offset_bme", 0);
  settings.hum_offset_htu = preferences.getInt("hum_offset_htu", 0);
  
  // Калибровка давления
  settings.press_offset_bme = preferences.getInt("press_offset_bme", 0);
  settings.press_offset_ms = preferences.getInt("press_offset_ms", 0);
  
  // Пороги CO2
  settings.co2_warning = preferences.getInt("co2_warning", 1000);
  settings.co2_critical = preferences.getInt("co2_critical", 1400);
  
  // Пороги PM2.5
  settings.pm25_warning = preferences.getInt("pm25_warning", 35);
  settings.pm25_critical = preferences.getInt("pm25_critical", 50);
  
  // Ночной режим
  settings.night_mode_start = preferences.getInt("night_mode_start", 23);
  settings.night_mode_end = preferences.getInt("night_mode_end", 7);
  
  Serial.println("Settings loaded from NVS");
}

void settings_save() {
  // Интервал обновления
  preferences.putInt("update_interval", settings.update_interval);
  
  // WiFi настройки
  preferences.putString("wifi_ssid", settings.wifi_ssid);
  preferences.putString("wifi_password", settings.wifi_password);
  
  // Калибровка температуры
  preferences.putFloat("temp_offset_bme", settings.temp_offset_bme);
  preferences.putFloat("temp_offset_htu", settings.temp_offset_htu);
  preferences.putFloat("temp_offset_scd", settings.temp_offset_scd);
  
  // Калибровка влажности
  preferences.putInt("hum_offset_bme", settings.hum_offset_bme);
  preferences.putInt("hum_offset_htu", settings.hum_offset_htu);
  
  // Калибровка давления
  preferences.putInt("press_offset_bme", settings.press_offset_bme);
  preferences.putInt("press_offset_ms", settings.press_offset_ms);
  
  // Пороги CO2
  preferences.putInt("co2_warning", settings.co2_warning);
  preferences.putInt("co2_critical", settings.co2_critical);
  
  // Пороги PM2.5
  preferences.putInt("pm25_warning", settings.pm25_warning);
  preferences.putInt("pm25_critical", settings.pm25_critical);
  
  // Ночной режим
  preferences.putInt("night_mode_start", settings.night_mode_start);
  preferences.putInt("night_mode_end", settings.night_mode_end);
  
  preferences.end();
  Serial.println("Settings saved to NVS");
}

void settings_reset() {
  preferences.clear();
  settings_load();
  Serial.println("Settings reset to defaults");
}
