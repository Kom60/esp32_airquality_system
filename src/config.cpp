#include "headers.h"
#include "config.h"
#include "sdcard.h"
#include <SD.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// ============================================================================
// Глобальный объект конфигурации
// ============================================================================
static Config _config;
static bool _config_loaded = false;

// ============================================================================
// Вспомогательные функции
// ============================================================================

static uint32_t generate_config_id() {
  uint64_t chipid = ESP.getEfuseMac();
  return (uint32_t)(chipid >> 24);
}

template<typename T>
static bool in_range(T value, T min, T max) {
  return value >= min && value <= max;
}

// ============================================================================
// JSON сериализация - ArduinoJson v7 совместимость
// ============================================================================

static void wifi_to_json(const WiFiConfig& cfg, JsonObject& root) {
  root["ssid"] = cfg.ssid;
  root["password"] = cfg.password;
  root["reconnect_interval"] = cfg.reconnect_interval;
  root["reconnect_max_attempts"] = cfg.reconnect_max_attempts;
  root["auto_connect"] = cfg.auto_connect;
  root["static_ip"] = cfg.static_ip;
  root["gateway"] = cfg.gateway;
  root["subnet"] = cfg.subnet;
  root["dns1"] = cfg.dns1;
  root["dns2"] = cfg.dns2;
}

static void wifi_from_json(JsonObjectConst root, WiFiConfig& cfg) {
  if (root["ssid"].is<const char*>()) strlcpy(cfg.ssid, root["ssid"], sizeof(cfg.ssid));
  if (root["password"].is<const char*>()) strlcpy(cfg.password, root["password"], sizeof(cfg.password));
  if (root["reconnect_interval"].is<int>()) cfg.reconnect_interval = root["reconnect_interval"];
  if (root["reconnect_max_attempts"].is<int>()) cfg.reconnect_max_attempts = root["reconnect_max_attempts"];
  if (root["auto_connect"].is<bool>()) cfg.auto_connect = root["auto_connect"];
  if (root["static_ip"].is<const char*>()) strlcpy(cfg.static_ip, root["static_ip"], sizeof(cfg.static_ip));
  if (root["gateway"].is<const char*>()) strlcpy(cfg.gateway, root["gateway"], sizeof(cfg.gateway));
  if (root["subnet"].is<const char*>()) strlcpy(cfg.subnet, root["subnet"], sizeof(cfg.subnet));
  if (root["dns1"].is<const char*>()) strlcpy(cfg.dns1, root["dns1"], sizeof(cfg.dns1));
  if (root["dns2"].is<const char*>()) strlcpy(cfg.dns2, root["dns2"], sizeof(cfg.dns2));
}

static void ntp_to_json(const NTPConfig& cfg, JsonObject& root) {
  root["server"] = cfg.server;
  root["gmt_offset_sec"] = cfg.gmt_offset_sec;
  root["daylight_offset_sec"] = cfg.daylight_offset_sec;
  root["auto_sync"] = cfg.auto_sync;
  root["sync_interval"] = cfg.sync_interval;
}

static void ntp_from_json(JsonObjectConst root, NTPConfig& cfg) {
  if (root["server"].is<const char*>()) strlcpy(cfg.server, root["server"], sizeof(cfg.server));
  if (root["gmt_offset_sec"].is<int32_t>()) cfg.gmt_offset_sec = root["gmt_offset_sec"];
  if (root["daylight_offset_sec"].is<int>()) cfg.daylight_offset_sec = root["daylight_offset_sec"];
  if (root["auto_sync"].is<bool>()) cfg.auto_sync = root["auto_sync"];
  if (root["sync_interval"].is<int>()) cfg.sync_interval = root["sync_interval"];
}

static void sensors_to_json(const SensorsConfig& cfg, JsonObject& root) {
  JsonObject intervals = root["intervals"].to<JsonObject>();
  intervals["bme"] = cfg.bme_interval;
  intervals["htu"] = cfg.htu_interval;
  intervals["scd"] = cfg.scd_interval;
  intervals["pms"] = cfg.pms_interval;
  intervals["ms"] = cfg.ms_interval;
  intervals["bh"] = cfg.bh_interval;
  intervals["veml"] = cfg.veml_interval;
  intervals["ch2o"] = cfg.ch2o_interval;
  intervals["mic"] = cfg.mic_interval;
  intervals["ina"] = cfg.ina_interval;

  JsonObject calibration = root["calibration"].to<JsonObject>();
  calibration["temp_bme"] = cfg.temp_offset_bme;
  calibration["temp_htu"] = cfg.temp_offset_htu;
  calibration["temp_scd"] = cfg.temp_offset_scd;
  calibration["hum_bme"] = cfg.hum_offset_bme;
  calibration["hum_htu"] = cfg.hum_offset_htu;
  calibration["hum_scd"] = cfg.hum_offset_scd;
  calibration["press_bme"] = cfg.press_offset_bme;
  calibration["press_ms"] = cfg.press_offset_ms;

  JsonObject enabled = root["enabled"].to<JsonObject>();
  enabled["bme"] = cfg.bme_enabled;
  enabled["htu"] = cfg.htu_enabled;
  enabled["scd"] = cfg.scd_enabled;
  enabled["pms"] = cfg.pms_enabled;
  enabled["ms"] = cfg.ms_enabled;
  enabled["bh"] = cfg.bh_enabled;
  enabled["veml"] = cfg.veml_enabled;
  enabled["ch2o"] = cfg.ch2o_enabled;
  enabled["mic"] = cfg.mic_enabled;
  enabled["ina"] = cfg.ina_enabled;

  JsonObject addresses = root["addresses"].to<JsonObject>();
  addresses["bme"] = cfg.bme_i2c_addr;
  addresses["htu"] = cfg.htu_i2c_addr;
  addresses["scd"] = cfg.scd_i2c_addr;
  addresses["pms_baudrate"] = cfg.pms_baudrate;
  addresses["ms"] = cfg.ms_i2c_addr;
  addresses["bh"] = cfg.bh_i2c_addr;
  addresses["veml"] = cfg.veml_i2c_addr;
  addresses["ch2o"] = cfg.ch2o_i2c_addr;
  addresses["ina"] = cfg.ina_i2c_addr;
}

static void sensors_from_json(JsonObjectConst root, SensorsConfig& cfg) {
  if (root["intervals"].is<JsonObjectConst>()) {
    JsonObjectConst intervals = root["intervals"];
    if (intervals["bme"].is<int>()) cfg.bme_interval = intervals["bme"];
    if (intervals["htu"].is<int>()) cfg.htu_interval = intervals["htu"];
    if (intervals["scd"].is<int>()) cfg.scd_interval = intervals["scd"];
    if (intervals["pms"].is<int>()) cfg.pms_interval = intervals["pms"];
    if (intervals["ms"].is<int>()) cfg.ms_interval = intervals["ms"];
    if (intervals["bh"].is<int>()) cfg.bh_interval = intervals["bh"];
    if (intervals["veml"].is<int>()) cfg.veml_interval = intervals["veml"];
    if (intervals["ch2o"].is<int>()) cfg.ch2o_interval = intervals["ch2o"];
    if (intervals["mic"].is<int>()) cfg.mic_interval = intervals["mic"];
    if (intervals["ina"].is<int>()) cfg.ina_interval = intervals["ina"];
  }

  if (root["calibration"].is<JsonObjectConst>()) {
    JsonObjectConst calibration = root["calibration"];
    if (calibration["temp_bme"].is<float>()) cfg.temp_offset_bme = calibration["temp_bme"];
    if (calibration["temp_htu"].is<float>()) cfg.temp_offset_htu = calibration["temp_htu"];
    if (calibration["temp_scd"].is<float>()) cfg.temp_offset_scd = calibration["temp_scd"];
    if (calibration["hum_bme"].is<int>()) cfg.hum_offset_bme = calibration["hum_bme"];
    if (calibration["hum_htu"].is<int>()) cfg.hum_offset_htu = calibration["hum_htu"];
    if (calibration["hum_scd"].is<int>()) cfg.hum_offset_scd = calibration["hum_scd"];
    if (calibration["press_bme"].is<int>()) cfg.press_offset_bme = calibration["press_bme"];
    if (calibration["press_ms"].is<int>()) cfg.press_offset_ms = calibration["press_ms"];
  }

  if (root["enabled"].is<JsonObjectConst>()) {
    JsonObjectConst enabled = root["enabled"];
    if (enabled["bme"].is<bool>()) cfg.bme_enabled = enabled["bme"];
    if (enabled["htu"].is<bool>()) cfg.htu_enabled = enabled["htu"];
    if (enabled["scd"].is<bool>()) cfg.scd_enabled = enabled["scd"];
    if (enabled["pms"].is<bool>()) cfg.pms_enabled = enabled["pms"];
    if (enabled["ms"].is<bool>()) cfg.ms_enabled = enabled["ms"];
    if (enabled["bh"].is<bool>()) cfg.bh_enabled = enabled["bh"];
    if (enabled["veml"].is<bool>()) cfg.veml_enabled = enabled["veml"];
    if (enabled["ch2o"].is<bool>()) cfg.ch2o_enabled = enabled["ch2o"];
    if (enabled["mic"].is<bool>()) cfg.mic_enabled = enabled["mic"];
    if (enabled["ina"].is<bool>()) cfg.ina_enabled = enabled["ina"];
  }

  if (root["addresses"].is<JsonObjectConst>()) {
    JsonObjectConst addresses = root["addresses"];
    if (addresses["bme"].is<int>()) cfg.bme_i2c_addr = addresses["bme"];
    if (addresses["htu"].is<int>()) cfg.htu_i2c_addr = addresses["htu"];
    if (addresses["scd"].is<int>()) cfg.scd_i2c_addr = addresses["scd"];
    if (addresses["pms_baudrate"].is<int>()) cfg.pms_baudrate = addresses["pms_baudrate"];
    if (addresses["ms"].is<int>()) cfg.ms_i2c_addr = addresses["ms"];
    if (addresses["bh"].is<int>()) cfg.bh_i2c_addr = addresses["bh"];
    if (addresses["veml"].is<int>()) cfg.veml_i2c_addr = addresses["veml"];
    if (addresses["ch2o"].is<int>()) cfg.ch2o_i2c_addr = addresses["ch2o"];
    if (addresses["ina"].is<int>()) cfg.ina_i2c_addr = addresses["ina"];
  }
}

static void alerts_to_json(const AlertsConfig& cfg, JsonObject& root) {
  JsonObject thresholds = root["thresholds"].to<JsonObject>();
  
  JsonObject co2 = thresholds["co2"].to<JsonObject>();
  co2["warning"] = cfg.co2_warning;
  co2["critical"] = cfg.co2_critical;
  
  JsonObject pm25 = thresholds["pm25"].to<JsonObject>();
  pm25["warning"] = cfg.pm25_warning;
  pm25["critical"] = cfg.pm25_critical;
  
  JsonObject pm10 = thresholds["pm10"].to<JsonObject>();
  pm10["warning"] = cfg.pm10_warning;
  pm10["critical"] = cfg.pm10_critical;
  
  JsonObject temp = thresholds["temp"].to<JsonObject>();
  temp["warning_low"] = cfg.temp_warning_low;
  temp["warning_high"] = cfg.temp_warning_high;
  temp["critical_low"] = cfg.temp_critical_low;
  temp["critical_high"] = cfg.temp_critical_high;
  
  JsonObject hum = thresholds["hum"].to<JsonObject>();
  hum["warning_low"] = cfg.hum_warning_low;
  hum["warning_high"] = cfg.hum_warning_high;
  hum["critical_low"] = cfg.hum_critical_low;
  hum["critical_high"] = cfg.hum_critical_high;
  
  JsonObject press = thresholds["press"].to<JsonObject>();
  press["warning_low"] = cfg.press_warning_low;
  press["warning_high"] = cfg.press_warning_high;
  
  JsonObject ch2o = thresholds["ch2o"].to<JsonObject>();
  ch2o["warning"] = cfg.ch2o_warning;
  ch2o["critical"] = cfg.ch2o_critical;
  
  JsonObject uv = thresholds["uv"].to<JsonObject>();
  uv["warning"] = cfg.uv_warning;
  uv["critical"] = cfg.uv_critical;
  
  JsonObject noise = thresholds["noise"].to<JsonObject>();
  noise["warning"] = cfg.noise_warning;
  noise["critical"] = cfg.noise_critical;

  JsonObject enabled = root["enabled"].to<JsonObject>();
  enabled["co2"] = cfg.co2_alerts_enabled;
  enabled["pm"] = cfg.pm_alerts_enabled;
  enabled["temp"] = cfg.temp_alerts_enabled;
  enabled["hum"] = cfg.hum_alerts_enabled;
  enabled["press"] = cfg.press_alerts_enabled;
  enabled["ch2o"] = cfg.ch2o_alerts_enabled;
  enabled["uv"] = cfg.uv_alerts_enabled;
  enabled["noise"] = cfg.noise_alerts_enabled;
}

static void alerts_from_json(JsonObjectConst root, AlertsConfig& cfg) {
  if (root["thresholds"].is<JsonObjectConst>()) {
    JsonObjectConst thresholds = root["thresholds"];
    
    if (thresholds["co2"].is<JsonObjectConst>()) {
      JsonObjectConst co2 = thresholds["co2"];
      if (co2["warning"].is<int>()) cfg.co2_warning = co2["warning"];
      if (co2["critical"].is<int>()) cfg.co2_critical = co2["critical"];
    }
    
    if (thresholds["pm25"].is<JsonObjectConst>()) {
      JsonObjectConst pm25 = thresholds["pm25"];
      if (pm25["warning"].is<int>()) cfg.pm25_warning = pm25["warning"];
      if (pm25["critical"].is<int>()) cfg.pm25_critical = pm25["critical"];
    }
    
    if (thresholds["pm10"].is<JsonObjectConst>()) {
      JsonObjectConst pm10 = thresholds["pm10"];
      if (pm10["warning"].is<int>()) cfg.pm10_warning = pm10["warning"];
      if (pm10["critical"].is<int>()) cfg.pm10_critical = pm10["critical"];
    }
    
    if (thresholds["temp"].is<JsonObjectConst>()) {
      JsonObjectConst temp = thresholds["temp"];
      if (temp["warning_low"].is<float>()) cfg.temp_warning_low = temp["warning_low"];
      if (temp["warning_high"].is<float>()) cfg.temp_warning_high = temp["warning_high"];
      if (temp["critical_low"].is<float>()) cfg.temp_critical_low = temp["critical_low"];
      if (temp["critical_high"].is<float>()) cfg.temp_critical_high = temp["critical_high"];
    }
    
    if (thresholds["hum"].is<JsonObjectConst>()) {
      JsonObjectConst hum = thresholds["hum"];
      if (hum["warning_low"].is<int>()) cfg.hum_warning_low = hum["warning_low"];
      if (hum["warning_high"].is<int>()) cfg.hum_warning_high = hum["warning_high"];
      if (hum["critical_low"].is<int>()) cfg.hum_critical_low = hum["critical_low"];
      if (hum["critical_high"].is<int>()) cfg.hum_critical_high = hum["critical_high"];
    }
    
    if (thresholds["press"].is<JsonObjectConst>()) {
      JsonObjectConst press = thresholds["press"];
      if (press["warning_low"].is<int>()) cfg.press_warning_low = press["warning_low"];
      if (press["warning_high"].is<int>()) cfg.press_warning_high = press["warning_high"];
    }
    
    if (thresholds["ch2o"].is<JsonObjectConst>()) {
      JsonObjectConst ch2o = thresholds["ch2o"];
      if (ch2o["warning"].is<float>()) cfg.ch2o_warning = ch2o["warning"];
      if (ch2o["critical"].is<float>()) cfg.ch2o_critical = ch2o["critical"];
    }
    
    if (thresholds["uv"].is<JsonObjectConst>()) {
      JsonObjectConst uv = thresholds["uv"];
      if (uv["warning"].is<int>()) cfg.uv_warning = uv["warning"];
      if (uv["critical"].is<int>()) cfg.uv_critical = uv["critical"];
    }
    
    if (thresholds["noise"].is<JsonObjectConst>()) {
      JsonObjectConst noise = thresholds["noise"];
      if (noise["warning"].is<int>()) cfg.noise_warning = noise["warning"];
      if (noise["critical"].is<int>()) cfg.noise_critical = noise["critical"];
    }
  }

  if (root["enabled"].is<JsonObjectConst>()) {
    JsonObjectConst enabled = root["enabled"];
    if (enabled["co2"].is<bool>()) cfg.co2_alerts_enabled = enabled["co2"];
    if (enabled["pm"].is<bool>()) cfg.pm_alerts_enabled = enabled["pm"];
    if (enabled["temp"].is<bool>()) cfg.temp_alerts_enabled = enabled["temp"];
    if (enabled["hum"].is<bool>()) cfg.hum_alerts_enabled = enabled["hum"];
    if (enabled["press"].is<bool>()) cfg.press_alerts_enabled = enabled["press"];
    if (enabled["ch2o"].is<bool>()) cfg.ch2o_alerts_enabled = enabled["ch2o"];
    if (enabled["uv"].is<bool>()) cfg.uv_alerts_enabled = enabled["uv"];
    if (enabled["noise"].is<bool>()) cfg.noise_alerts_enabled = enabled["noise"];
  }
}

static void display_to_json(const DisplayConfig& cfg, JsonObject& root) {
  root["enabled"] = cfg.enabled;
  root["brightness"] = cfg.brightness;
  root["brightness_night"] = cfg.brightness_night;
  root["night_mode_start"] = cfg.night_mode_start;
  root["night_mode_end"] = cfg.night_mode_end;
  root["screen_timeout"] = cfg.screen_timeout;
  root["update_interval"] = cfg.update_interval;
  root["auto_rotate"] = cfg.auto_rotate;
  root["rotation"] = cfg.rotation;
}

static void display_from_json(JsonObjectConst root, DisplayConfig& cfg) {
  if (root["enabled"].is<bool>()) cfg.enabled = root["enabled"];
  if (root["brightness"].is<int>()) cfg.brightness = root["brightness"];
  if (root["brightness_night"].is<int>()) cfg.brightness_night = root["brightness_night"];
  if (root["night_mode_start"].is<int>()) cfg.night_mode_start = root["night_mode_start"];
  if (root["night_mode_end"].is<int>()) cfg.night_mode_end = root["night_mode_end"];
  if (root["screen_timeout"].is<int>()) cfg.screen_timeout = root["screen_timeout"];
  if (root["update_interval"].is<int>()) cfg.update_interval = root["update_interval"];
  if (root["auto_rotate"].is<bool>()) cfg.auto_rotate = root["auto_rotate"];
  if (root["rotation"].is<int>()) cfg.rotation = root["rotation"];
}

static void sd_to_json(const SDConfig& cfg, JsonObject& root) {
  root["enabled"] = cfg.enabled;
  root["cs_pin"] = cfg.cs_pin;
  root["auto_log"] = cfg.auto_log;
  root["log_interval"] = cfg.log_interval;
  root["log_format"] = cfg.log_format;
  root["create_hourly_files"] = cfg.create_hourly_files;
  root["max_file_size_mb"] = cfg.max_file_size_mb;
  root["auto_backup"] = cfg.auto_backup;
  root["backup_interval_hours"] = cfg.backup_interval_hours;
}

static void sd_from_json(JsonObjectConst root, SDConfig& cfg) {
  if (root["enabled"].is<bool>()) cfg.enabled = root["enabled"];
  if (root["cs_pin"].is<int>()) cfg.cs_pin = root["cs_pin"];
  if (root["auto_log"].is<bool>()) cfg.auto_log = root["auto_log"];
  if (root["log_interval"].is<int>()) cfg.log_interval = root["log_interval"];
  if (root["log_format"].is<const char*>()) strlcpy(cfg.log_format, root["log_format"], sizeof(cfg.log_format));
  if (root["create_hourly_files"].is<bool>()) cfg.create_hourly_files = root["create_hourly_files"];
  if (root["max_file_size_mb"].is<int>()) cfg.max_file_size_mb = root["max_file_size_mb"];
  if (root["auto_backup"].is<bool>()) cfg.auto_backup = root["auto_backup"];
  if (root["backup_interval_hours"].is<int>()) cfg.backup_interval_hours = root["backup_interval_hours"];
}

static void spiffs_to_json(const SPIFFSConfig& cfg, JsonObject& root) {
  root["format_on_error"] = cfg.format_on_error;
  root["max_open_files"] = cfg.max_open_files;
}

static void spiffs_from_json(JsonObjectConst root, SPIFFSConfig& cfg) {
  if (root["format_on_error"].is<bool>()) cfg.format_on_error = root["format_on_error"];
  if (root["max_open_files"].is<int>()) cfg.max_open_files = root["max_open_files"];
}

static void ssl_to_json(const SSLConfig& cfg, JsonObject& root) {
  root["https_enabled"] = cfg.https_enabled;
  root["wss_enabled"] = cfg.wss_enabled;
  root["https_port"] = cfg.https_port;
  root["wss_port"] = cfg.wss_port;
  root["cert_path"] = cfg.cert_path;
  root["key_path"] = cfg.key_path;
  root["use_self_signed"] = cfg.use_self_signed;
  root["common_name"] = cfg.common_name;
  root["cert_validity_days"] = cfg.cert_validity_days;
}

static void ssl_from_json(JsonObjectConst root, SSLConfig& cfg) {
  if (root["https_enabled"].is<bool>()) cfg.https_enabled = root["https_enabled"];
  if (root["wss_enabled"].is<bool>()) cfg.wss_enabled = root["wss_enabled"];
  if (root["https_port"].is<int>()) cfg.https_port = root["https_port"];
  if (root["wss_port"].is<int>()) cfg.wss_port = root["wss_port"];
  if (root["cert_path"].is<const char*>()) strlcpy(cfg.cert_path, root["cert_path"], sizeof(cfg.cert_path));
  if (root["key_path"].is<const char*>()) strlcpy(cfg.key_path, root["key_path"], sizeof(cfg.key_path));
  if (root["use_self_signed"].is<bool>()) cfg.use_self_signed = root["use_self_signed"];
  if (root["common_name"].is<const char*>()) strlcpy(cfg.common_name, root["common_name"], sizeof(cfg.common_name));
  if (root["cert_validity_days"].is<int>()) cfg.cert_validity_days = root["cert_validity_days"];
}

static void webserver_to_json(const WebServerConfig& cfg, JsonObject& root) {
  root["enabled"] = cfg.enabled;
  root["port"] = cfg.port;
  root["cors_enabled"] = cfg.cors_enabled;
  root["cors_origin"] = cfg.cors_origin;
  root["max_clients"] = cfg.max_clients;
  root["auth_enabled"] = cfg.auth_enabled;
  root["auth_username"] = cfg.auth_username;
  root["auth_password"] = cfg.auth_password;
  root["session_timeout"] = cfg.session_timeout;
}

static void webserver_from_json(JsonObjectConst root, WebServerConfig& cfg) {
  if (root["enabled"].is<bool>()) cfg.enabled = root["enabled"];
  if (root["port"].is<int>()) cfg.port = root["port"];
  if (root["cors_enabled"].is<bool>()) cfg.cors_enabled = root["cors_enabled"];
  if (root["cors_origin"].is<const char*>()) strlcpy(cfg.cors_origin, root["cors_origin"], sizeof(cfg.cors_origin));
  if (root["max_clients"].is<int>()) cfg.max_clients = root["max_clients"];
  if (root["auth_enabled"].is<bool>()) cfg.auth_enabled = root["auth_enabled"];
  if (root["auth_username"].is<const char*>()) strlcpy(cfg.auth_username, root["auth_username"], sizeof(cfg.auth_username));
  if (root["auth_password"].is<const char*>()) strlcpy(cfg.auth_password, root["auth_password"], sizeof(cfg.auth_password));
  if (root["session_timeout"].is<int>()) cfg.session_timeout = root["session_timeout"];
}

static void websocket_to_json(const WebSocketConfig& cfg, JsonObject& root) {
  root["enabled"] = cfg.enabled;
  root["port"] = cfg.port;
  root["ping_interval"] = cfg.ping_interval;
  root["max_clients"] = cfg.max_clients;
  root["broadcast_enabled"] = cfg.broadcast_enabled;
  root["broadcast_interval"] = cfg.broadcast_interval;
}

static void websocket_from_json(JsonObjectConst root, WebSocketConfig& cfg) {
  if (root["enabled"].is<bool>()) cfg.enabled = root["enabled"];
  if (root["port"].is<int>()) cfg.port = root["port"];
  if (root["ping_interval"].is<int>()) cfg.ping_interval = root["ping_interval"];
  if (root["max_clients"].is<int>()) cfg.max_clients = root["max_clients"];
  if (root["broadcast_enabled"].is<bool>()) cfg.broadcast_enabled = root["broadcast_enabled"];
  if (root["broadcast_interval"].is<int>()) cfg.broadcast_interval = root["broadcast_interval"];
}

static void mqtt_to_json(const MQTTConfig& cfg, JsonObject& root) {
  root["enabled"] = cfg.enabled;
  root["broker"] = cfg.broker;
  root["port"] = cfg.port;
  root["username"] = cfg.username;
  root["password"] = cfg.password;
  root["client_id"] = cfg.client_id;
  root["topic_prefix"] = cfg.topic_prefix;
  root["qos"] = cfg.qos;
  root["retain"] = cfg.retain;
  root["keepalive"] = cfg.keepalive;
  root["auto_reconnect"] = cfg.auto_reconnect;
}

static void mqtt_from_json(JsonObjectConst root, MQTTConfig& cfg) {
  if (root["enabled"].is<bool>()) cfg.enabled = root["enabled"];
  if (root["broker"].is<const char*>()) strlcpy(cfg.broker, root["broker"], sizeof(cfg.broker));
  if (root["port"].is<int>()) cfg.port = root["port"];
  if (root["username"].is<const char*>()) strlcpy(cfg.username, root["username"], sizeof(cfg.username));
  if (root["password"].is<const char*>()) strlcpy(cfg.password, root["password"], sizeof(cfg.password));
  if (root["client_id"].is<const char*>()) strlcpy(cfg.client_id, root["client_id"], sizeof(cfg.client_id));
  if (root["topic_prefix"].is<const char*>()) strlcpy(cfg.topic_prefix, root["topic_prefix"], sizeof(cfg.topic_prefix));
  if (root["qos"].is<int>()) cfg.qos = root["qos"];
  if (root["retain"].is<bool>()) cfg.retain = root["retain"];
  if (root["keepalive"].is<int>()) cfg.keepalive = root["keepalive"];
  if (root["auto_reconnect"].is<bool>()) cfg.auto_reconnect = root["auto_reconnect"];
}

static void ota_to_json(const OTAConfig& cfg, JsonObject& root) {
  root["enabled"] = cfg.enabled;
  root["password"] = cfg.password;
  root["port"] = cfg.port;
  root["reboot_on_success"] = cfg.reboot_on_success;
}

static void ota_from_json(JsonObjectConst root, OTAConfig& cfg) {
  if (root["enabled"].is<bool>()) cfg.enabled = root["enabled"];
  if (root["password"].is<const char*>()) strlcpy(cfg.password, root["password"], sizeof(cfg.password));
  if (root["port"].is<int>()) cfg.port = root["port"];
  if (root["reboot_on_success"].is<bool>()) cfg.reboot_on_success = root["reboot_on_success"];
}

static void system_to_json(const SystemConfig& cfg, JsonObject& root) {
  root["device_name"] = cfg.device_name;
  root["device_id"] = cfg.device_id;
  root["led_enabled"] = cfg.led_enabled;
  root["led_pin"] = cfg.led_pin;
  root["led_inverted"] = cfg.led_inverted;
  root["log_level"] = cfg.log_level;
  root["serial_log_enabled"] = cfg.serial_log_enabled;
  root["serial_baudrate"] = cfg.serial_baudrate;
  root["watchdog_timeout"] = cfg.watchdog_timeout;
  root["factory_reset_pin"] = cfg.factory_reset_pin;
  root["factory_reset_pin_num"] = cfg.factory_reset_pin_num;
  root["factory_reset_hold_ms"] = cfg.factory_reset_hold_ms;
}

static void system_from_json(JsonObjectConst root, SystemConfig& cfg) {
  if (root["device_name"].is<const char*>()) strlcpy(cfg.device_name, root["device_name"], sizeof(cfg.device_name));
  if (root["device_id"].is<const char*>()) strlcpy(cfg.device_id, root["device_id"], sizeof(cfg.device_id));
  if (root["led_enabled"].is<bool>()) cfg.led_enabled = root["led_enabled"];
  if (root["led_pin"].is<int>()) cfg.led_pin = root["led_pin"];
  if (root["led_inverted"].is<bool>()) cfg.led_inverted = root["led_inverted"];
  if (root["log_level"].is<int>()) cfg.log_level = root["log_level"];
  if (root["serial_log_enabled"].is<bool>()) cfg.serial_log_enabled = root["serial_log_enabled"];
  if (root["serial_baudrate"].is<int>()) cfg.serial_baudrate = root["serial_baudrate"];
  if (root["watchdog_timeout"].is<int>()) cfg.watchdog_timeout = root["watchdog_timeout"];
  if (root["factory_reset_pin"].is<bool>()) cfg.factory_reset_pin = root["factory_reset_pin"];
  if (root["factory_reset_pin_num"].is<int>()) cfg.factory_reset_pin_num = root["factory_reset_pin_num"];
  if (root["factory_reset_hold_ms"].is<int>()) cfg.factory_reset_hold_ms = root["factory_reset_hold_ms"];
}

// ============================================================================
// Основные функции
// ============================================================================

void config_init() {
  LOG_INFO(SETTINGS, "Initializing configuration system...");
  _config.config_id = generate_config_id();

  if (!config_load()) {
    LOG_WARNING(SETTINGS, "Failed to load config from SD card");
    LOG_WARNING(SETTINGS, "Please copy config.json to SD card at /config/config.json");
    LOG_WARNING(SETTINGS, "Using default configuration (will NOT save to SD)");
    // НЕ вызываем config_reset() - это предотвращает перезапись файла!
  } else {
    LOG_INFO(SETTINGS, "Configuration system initialized");
  }

  _config_loaded = true;
}

bool config_load() {
  if (!sdcard_is_ready()) {
    LOG_ERROR(SETTINGS, "SD card not ready");
    return false;
  }

  LOG_INFO_FMT(SETTINGS, "Opening config file from SD card: %s", SD_CONFIG_FILE);
  
  File file = SD.open(SD_CONFIG_FILE, FILE_READ);
  if (!file || file.size() == 0) {
    LOG_WARNING(SETTINGS, "Config file not found or empty on SD card");
    if (file) file.close();
    return false;
  }

  size_t size = file.size();
  LOG_INFO_FMT(SETTINGS, "Config file size: %d bytes (from SD card)", size);
  
  if (size > CONFIG_MAX_SIZE) {
    LOG_ERROR_FMT(SETTINGS, "Config file too large: %d bytes", size);
    file.close();
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size + 1]);
  file.readBytes(buf.get(), size);
  buf[size] = '\0';
  file.close();

  bool result = config_set_json(buf.get());
  if (result) {
    LOG_INFO(SETTINGS, "✓ Configuration successfully loaded from SD card");
    LOG_INFO_FMT(SETTINGS, "  → WiFi SSID: %s", _config.wifi.ssid);
    LOG_INFO_FMT(SETTINGS,  "  → NTP Server: %s", _config.ntp.server);
    LOG_INFO_FMT(SETTINGS, "  → Device Name: %s", _config.system.device_name);
  } else {
    LOG_ERROR(SETTINGS, "Failed to parse config JSON from SD card");
  }

  return result;
}

bool config_save() {
  return config_save_as(SD_CONFIG_FILE);
}

bool config_save_as(const char* filename) {
  if (!sdcard_is_ready()) {
    LOG_ERROR(SETTINGS, "SD card not ready");
    return false;
  }

  JsonDocument doc;

  doc["version"] = _config.version;
  doc["config_id"] = _config.config_id;

  // ArduinoJson v7: создаём временные переменные для JsonObject
  JsonObject wifi_obj = doc["wifi"].to<JsonObject>();
  wifi_to_json(_config.wifi, wifi_obj);

  JsonObject ntp_obj = doc["ntp"].to<JsonObject>();
  ntp_to_json(_config.ntp, ntp_obj);

  JsonObject sensors_obj = doc["sensors"].to<JsonObject>();
  sensors_to_json(_config.sensors, sensors_obj);

  JsonObject alerts_obj = doc["alerts"].to<JsonObject>();
  alerts_to_json(_config.alerts, alerts_obj);

  JsonObject display_obj = doc["display"].to<JsonObject>();
  display_to_json(_config.display, display_obj);

  JsonObject sd_obj = doc["sd"].to<JsonObject>();
  sd_to_json(_config.sd, sd_obj);

  JsonObject spiffs_obj = doc["spiffs"].to<JsonObject>();
  spiffs_to_json(_config.spiffs, spiffs_obj);

  JsonObject ssl_obj = doc["ssl"].to<JsonObject>();
  ssl_to_json(_config.ssl, ssl_obj);

  JsonObject webserver_obj = doc["webserver"].to<JsonObject>();
  webserver_to_json(_config.webserver, webserver_obj);

  JsonObject websocket_obj = doc["websocket"].to<JsonObject>();
  websocket_to_json(_config.websocket, websocket_obj);

  JsonObject mqtt_obj = doc["mqtt"].to<JsonObject>();
  mqtt_to_json(_config.mqtt, mqtt_obj);

  JsonObject ota_obj = doc["ota"].to<JsonObject>();
  ota_to_json(_config.ota, ota_obj);

  JsonObject system_obj = doc["system"].to<JsonObject>();
  system_to_json(_config.system, system_obj);

  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    LOG_ERROR_FMT(SETTINGS, "Failed to open %s for writing", filename);
    return false;
  }

  serializeJson(doc, file);
  file.flush();
  file.close();

  LOG_INFO_FMT(SETTINGS, "Configuration saved to %s", filename);
  return true;
}

void config_reset() {
  LOG_INFO(SETTINGS, "Resetting configuration to defaults (in memory only)");
  _config = Config();
  _config.config_id = generate_config_id();
  // НЕ сохраняем автоматически - пользователь должен явно вызвать config_save()
  LOG_WARNING(SETTINGS, "Config reset in memory only - file on SD card NOT changed");
}

bool config_validate() {
  bool valid = true;
  
  if (_config.wifi.reconnect_interval < 5) {
    LOG_WARNING(SETTINGS, "WiFi reconnect_interval too low, setting to 5");
    _config.wifi.reconnect_interval = 5;
    valid = false;
  }
  
  if (!in_range(_config.sensors.bme_interval, 1, 3600)) {
    LOG_WARNING(SETTINGS, "BME interval out of range");
    _config.sensors.bme_interval = 5;
    valid = false;
  }
  
  if (_config.alerts.co2_warning >= _config.alerts.co2_critical) {
    LOG_WARNING(SETTINGS, "CO2 warning threshold invalid");
    _config.alerts.co2_warning = 1000;
    _config.alerts.co2_critical = 1400;
    valid = false;
  }
  
  if (!in_range(_config.display.brightness, 0, 100)) {
    LOG_WARNING(SETTINGS, "Display brightness out of range");
    _config.display.brightness = 80;
    valid = false;
  }
  
  if (!in_range(_config.display.night_mode_start, 0, 23) ||
      !in_range(_config.display.night_mode_end, 0, 23)) {
    LOG_WARNING(SETTINGS, "Night mode hours invalid");
    _config.display.night_mode_start = 23;
    _config.display.night_mode_end = 7;
    valid = false;
  }
  
  if (_config.alerts.temp_warning_low >= _config.alerts.temp_warning_high) {
    LOG_WARNING(SETTINGS, "Temperature thresholds invalid");
    _config.alerts.temp_warning_low = 15.0;
    _config.alerts.temp_warning_high = 30.0;
    valid = false;
  }
  
  if (!in_range(_config.alerts.hum_warning_low, 0, 100) ||
      !in_range(_config.alerts.hum_warning_high, 0, 100)) {
    LOG_WARNING(SETTINGS, "Humidity thresholds out of range");
    _config.alerts.hum_warning_low = 30;
    _config.alerts.hum_warning_high = 70;
    valid = false;
  }
  
  if (valid) {
    LOG_DEBUG(SETTINGS, "Configuration validation passed");
  } else {
    LOG_WARNING(SETTINGS, "Configuration validation completed with corrections");
  }
  
  return valid;
}

size_t config_get_json(char* buffer, size_t size) {
  JsonDocument doc;
  
  doc["version"] = _config.version;
  doc["config_id"] = _config.config_id;
  
  // ArduinoJson v7: создаём временные переменные для JsonObject
  JsonObject wifi_obj = doc["wifi"].to<JsonObject>();
  wifi_to_json(_config.wifi, wifi_obj);
  
  JsonObject ntp_obj = doc["ntp"].to<JsonObject>();
  ntp_to_json(_config.ntp, ntp_obj);
  
  JsonObject sensors_obj = doc["sensors"].to<JsonObject>();
  sensors_to_json(_config.sensors, sensors_obj);
  
  JsonObject alerts_obj = doc["alerts"].to<JsonObject>();
  alerts_to_json(_config.alerts, alerts_obj);
  
  JsonObject display_obj = doc["display"].to<JsonObject>();
  display_to_json(_config.display, display_obj);
  
  JsonObject sd_obj = doc["sd"].to<JsonObject>();
  sd_to_json(_config.sd, sd_obj);
  
  JsonObject spiffs_obj = doc["spiffs"].to<JsonObject>();
  spiffs_to_json(_config.spiffs, spiffs_obj);
  
  JsonObject webserver_obj = doc["webserver"].to<JsonObject>();
  webserver_to_json(_config.webserver, webserver_obj);
  
  JsonObject websocket_obj = doc["websocket"].to<JsonObject>();
  websocket_to_json(_config.websocket, websocket_obj);
  
  JsonObject mqtt_obj = doc["mqtt"].to<JsonObject>();
  mqtt_to_json(_config.mqtt, mqtt_obj);
  
  JsonObject ota_obj = doc["ota"].to<JsonObject>();
  ota_to_json(_config.ota, ota_obj);
  
  JsonObject system_obj = doc["system"].to<JsonObject>();
  system_to_json(_config.system, system_obj);
  
  return serializeJson(doc, buffer, size);
}

bool config_set_json(const char* json) {
  JsonDocument doc;
  
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    LOG_ERROR_FMT(SETTINGS, "JSON parse error: %s", error.c_str());
    return false;
  }
  
  if (doc["version"].is<const char*>()) strlcpy(_config.version, doc["version"], sizeof(_config.version));
  if (doc["config_id"].is<uint32_t>()) _config.config_id = doc["config_id"];
  
  if (doc["wifi"].is<JsonObjectConst>()) wifi_from_json(doc["wifi"], _config.wifi);
  if (doc["ntp"].is<JsonObjectConst>()) ntp_from_json(doc["ntp"], _config.ntp);
  if (doc["sensors"].is<JsonObjectConst>()) sensors_from_json(doc["sensors"], _config.sensors);
  if (doc["alerts"].is<JsonObjectConst>()) alerts_from_json(doc["alerts"], _config.alerts);
  if (doc["display"].is<JsonObjectConst>()) display_from_json(doc["display"], _config.display);
  if (doc["sd"].is<JsonObjectConst>()) sd_from_json(doc["sd"], _config.sd);
  if (doc["spiffs"].is<JsonObjectConst>()) spiffs_from_json(doc["spiffs"], _config.spiffs);
  if (doc["ssl"].is<JsonObjectConst>()) ssl_from_json(doc["ssl"], _config.ssl);
  if (doc["webserver"].is<JsonObjectConst>()) webserver_from_json(doc["webserver"], _config.webserver);
  if (doc["websocket"].is<JsonObjectConst>()) websocket_from_json(doc["websocket"], _config.websocket);
  if (doc["mqtt"].is<JsonObjectConst>()) mqtt_from_json(doc["mqtt"], _config.mqtt);
  if (doc["ota"].is<JsonObjectConst>()) ota_from_json(doc["ota"], _config.ota);
  if (doc["system"].is<JsonObjectConst>()) system_from_json(doc["system"], _config.system);
  
  config_validate();
  
  return true;
}

Config& config_get() {
  return _config;
}

bool config_exists() {
  if (!sdcard_is_ready()) {
    return false;
  }
  return sdcard_file_exists(SD_CONFIG_FILE);
}

bool config_delete() {
  if (!sdcard_is_ready()) {
    LOG_ERROR(SETTINGS, "SD card not ready");
    return false;
  }

  if (SD.remove(SD_CONFIG_FILE)) {
    LOG_INFO(SETTINGS, "Config file deleted");
    return true;
  }

  LOG_ERROR(SETTINGS, "Failed to delete config file");
  return false;
}

bool config_backup() {
  if (!sdcard_is_ready()) {
    LOG_ERROR(SETTINGS, "SD card not ready");
    return false;
  }

  if (config_save_as(SD_CONFIG_BACKUP)) {
    LOG_INFO(SETTINGS, "Configuration backup created");
    return true;
  }

  return false;
}

bool config_restore() {
  if (!sdcard_is_ready()) {
    LOG_ERROR(SETTINGS, "SD card not ready");
    return false;
  }

  if (!sdcard_file_exists(SD_CONFIG_BACKUP)) {
    LOG_WARNING(SETTINGS, "Backup file not found");
    return false;
  }

  File file = SD.open(SD_CONFIG_BACKUP, FILE_READ);
  if (!file) {
    LOG_ERROR(SETTINGS, "Failed to open backup file");
    return false;
  }

  size_t size = file.size();
  if (size > CONFIG_MAX_SIZE) {
    LOG_ERROR_FMT(SETTINGS, "Backup file too large: %d bytes", size);
    file.close();
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size + 1]);
  file.readBytes(buf.get(), size);
  buf[size] = '\0';
  file.close();
  
  bool result = config_set_json(buf.get());
  if (result) {
    config_save();
    LOG_INFO(SETTINGS, "Configuration restored from backup");
  }
  
  return result;
}
