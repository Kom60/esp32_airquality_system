#ifndef headers_h
#define headers_h

#include "display.h"
#include "Meteo.h"
#include "webheaders.h"
#include "tasks.h"
#include <SPIFFS.h>
#include "esp32/clk.h"
#include "sensors.h"


extern unsigned long delayTime;

extern TaskHandle_t CO2_measurementTask,BME_measurementTask,
 HTU_measurementTask,BH1750_measurementTask,CH2O_measurementTask,
  PMS_measurementTask,MS5611_measurementTask,VEML_measurementTask;

#endif