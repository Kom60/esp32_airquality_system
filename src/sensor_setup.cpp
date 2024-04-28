#include "headers.h"


void bme_setup(){
    
    while(!Serial);    // time to get serial running
    Serial.println(F("BME280 test"));

    unsigned status;
    
    // default settings
    status = bme.begin(0x76);  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }
    
    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();
    
    xTaskCreatePinnedToCore(BME_measurementTaskFunction, "BMEMeasurementTask", 2048, NULL, 1, &BME_measurementTask, 0);
}
void htu_setup()
{
  if (!htu.begin()) {
      Serial.println("Check circuit. HTU21D not found!");
      while (1);
  }
  xTaskCreatePinnedToCore(HTU_measurementTaskFunction, "HTUMeasurementTask", 2048, NULL, 1, &HTU_measurementTask, 0);
}
void BH1750_setup()
{
   if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }
  xTaskCreatePinnedToCore(BH1750_measurementTaskFunction, "BH1750MeasurementTask", 2048, NULL, 1, &BH1750_measurementTask, 0);
}
void MS5611_setup()
{
  ms5611.begin(MS5611_HIGH_RES);
  xTaskCreatePinnedToCore(MS5611_measurementTaskFunction, "MS5611MeasurementTask", 2048, NULL, 1, &MS5611_measurementTask, 0);
}
void SCD40_setup()
{
  //co2.setCalibrationMode(false);
  xTaskCreatePinnedToCore(CO2_measurementTaskFunction, "CO2MeasurementTask", 2048, NULL, 1, &CO2_measurementTask, 0);
}
void PMS_setup(){
  pms.init();
  xTaskCreatePinnedToCore(PMS_measurementTaskFunction, "PMSMeasurementTask", 2048, NULL, 1, &PMS_measurementTask, 0);
}
void CH2O_setup(){
  xTaskCreatePinnedToCore(CH2O_measurementTaskFunction, "CH2OMeasurementTask", 2048, NULL, 1, &CH2O_measurementTask, 0);
}

void VEML_setup(){
  uv.begin(VEML6070_1_T);  // pass in the integration time constant
  xTaskCreatePinnedToCore(VEML_measurementTaskFunction, "VEMLMeasurementTask", 2048, NULL, 1, &VEML_measurementTask, 0);
}