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
    Serial.println("Using WiFi settings from NVS: " + String(settings.wifi_ssid));
    WiFi.begin(settings.wifi_ssid, settings.wifi_password);
  }
  else
  {
    // Используем настройки по умолчанию из secrets.h
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  Serial.println("Establishing connection to WiFi with SSID: " + String(WiFi.SSID()));

  // Ожидание подключения
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());

  // Инициализация времени через NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("[NTP] Waiting for time sync...");
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 5000)) {
    Serial.println("[NTP] Time synchronized successfully");
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.print("[NTP] Current time: ");
    Serial.println(timeStringBuff);
  } else {
    Serial.println("[NTP] Failed to get time from NTP server");
  }
}
