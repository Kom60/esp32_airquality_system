#include "headers.h"
#include "network.h"
#include "settings.h"
#include "secrets.h"

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
}
