#include "headers.h"
#include "microphone.h"
#include "webheaders.h"
#include "secrets.h"

SCD4X co2;
double co2Value = 0, temperature = 0, humidity = 0;

// Mutex для защиты I2C шины
SemaphoreHandle_t i2c_mutex = NULL;

const int formaldehyde_Pin = 34;


SerialPM pms(PMSx003, 17,16); // PMSx003, UART
Adafruit_HTU21DF htu = Adafruit_HTU21DF(); //0x40 adress
MS5611 ms5611;
BH1750 lightMeter(0x23);
Adafruit_BME280 bme; // I2C 0x77 adress
Adafruit_VEML6070 uv = Adafruit_VEML6070();


TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

Meteo_data AIR_data = Meteo_data();


unsigned long delayTime;

// Configure IP addresses of the local access point
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 2);
IPAddress subnet(255, 255, 255, 0);

// We want to periodically send values to the clients, so we need to define an "interval" and remember the last time we sent data to the client (with "previousMillis")
int interval = 10000;              // будет обновлено из settings.update_interval
unsigned long previousMillis = 0; // we use the "millis()" command for time reference and this will output an unsigned long

// Обработчики API настроек
void handleGetSettings(AsyncWebServerRequest *request);
void handleSaveSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
void handleResetSettings(AsyncWebServerRequest *request);
void handleReboot(AsyncWebServerRequest *request);

