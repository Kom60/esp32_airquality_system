#include "headers.h"
#include "network.h"
#include "settings.h"
#include "secrets.h"
#include <time.h>

// NTP серверы и настройки времени
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3 * 3600;  // Москва GMT+3
const int   daylightOffset_sec = 0;    // Летнее время не используется

// Инициализация WiFi с использованием настроек из NVS или defaults
void wifi_setup()
{
  // Инициализация настроек из NVS
  settings_init();
  // Применяем WiFi настройки из NVS если они есть
  if (strlen(settings.wifi_ssid) > 0)
  {
    LOG_INFO(WIFI, "Using WiFi settings from NVS: " + String(settings.wifi_ssid));
    WiFi.begin(settings.wifi_ssid, settings.wifi_password);
  }
  else
  {
    // Используем настройки по умолчанию из secrets.h
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  LOG_INFO_FMT(WIFI, "Establishing connection to WiFi with SSID: %s", WiFi.SSID());

  // Ожидание подключения
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    LOG_D(WIFI, ".");
  }
  LOG_INFO_FMT(WIFI, "Connected to network with IP address: %s", WiFi.localIP().toString().c_str());

  // Инициализация времени через NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  LOG_INFO(NTP, "Waiting for time sync...");

  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 5000)) {
    LOG_INFO(NTP, "Time synchronized successfully");
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    LOG_INFO_FMT(NTP, "Current time: %s", timeStringBuff);
  } else {
    LOG_ERROR(NTP, "Failed to get time from NTP server");
  }
}
