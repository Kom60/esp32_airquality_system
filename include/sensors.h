#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Adafruit_HTU21DF.h"
#include "Adafruit_VEML6070.h"
#include <scd4x.h>
#include <BH1750.h>
#include <MS5611.h>
#include <PMserial.h> // Arduino library for PM sensors with serial interface

//#include "sos-iir-filter.h"

extern SCD4X co2;
extern double co2Value, temperature, humidity;

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)
extern const int formaldehyde_Pin;
extern float formaldehyde;

extern MS5611 ms5611;
extern SerialPM pms;
extern BH1750 lightMeter;
extern Adafruit_HTU21DF htu; // 0x40 adress
extern Adafruit_BME280 bme;  // I2C 0x77 adress
extern Adafruit_VEML6070 uv;

void bme_setup();
void htu_setup();
void BH1750_setup();
void MS5611_setup();
void SCD40_setup();
void PMS_setup();
void CH2O_setup();
void VEML_setup();

void BME_280_printValues();
void SCD40_printValues();
void display_bme();
void display_indoor();
void display_Info();
void PME5003_printValues(SerialPM pms);
void HTU21_prinValues();
void MS5611_printValues();
