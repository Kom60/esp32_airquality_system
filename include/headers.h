#ifndef headers_h
#define headers_h
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Adafruit_HTU21DF.h"
//#include "MS5611.h"
#include <PMserial.h> // Arduino library for PM sensors with serial interface
#include <TFT_eSPI.h>       // Include the graphics library
#include <BH1750.h>
#include "Meteo.h"
#include <scd4x.h>


#include <WiFi.h>              // needed to connect to WiFi
#include <ESPAsyncWebServer.h> // needed to create a simple webserver (make sure tools -> board is set to ESP32, otherwise you will get a "WebServer.h: No such file or directory" error)
#include <WebSocketsServer.h>  // needed for instant communication between client and server through Websockets
#include <ArduinoJson.h>       // needed for JSON encapsulation (send multiple variables with one string)
#include <SPIFFS.h>
#include "esp32/clk.h"

extern SCD4X co2;
extern double co2Value, temperature, humidity;
extern TaskHandle_t measurementTask;


#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)
extern const int formaldehyde_Pin;
extern float formaldehyde;

//extern MS5611 MS5611;
extern BH1750 lightMeter;
extern Adafruit_HTU21DF htu; //0x40 adress
extern Adafruit_BME280 bme; // I2C 0x77 adress

extern unsigned long delayTime;

extern TFT_eSPI tft;  // Create object "tft"

extern Meteo_data AIR_data;

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length);
void sendJson(String l_type, String l_value);
void sendJsonArray(String l_type, float l_array_values[]);
void measurementTaskFunction(void* parameter);

void bme_setup();
void htu_setup();
void BH1750_setup();
//void MS5611_setup();
void SCD40_setup();

void BME_280_printValues();
void SCD40_printValues();
void display_bme();
void display_indoor();
void display_Info();
void PME5003_printValues(SerialPM pms);
void HTU21_prinValues();
//void MS5611_printValues();

#endif