#ifndef headers_h
#define headers_h

#include "logger.h"
#include "config.h"
#include "settings.h"
#include "display.h"
#include "Meteo.h"
#include "webheaders.h"
#include "tasks.h"
#include "network.h"
#include "system_init.h"
#include "sdcard.h"
#include <SD.h>
#include "esp32/clk.h"
#include "sensors.h"
#include <HTTPClient.h>
#include <time.h>

extern SemaphoreHandle_t i2c_mutex;
extern QueueHandle_t samples_queue;
extern unsigned long delayTime;

extern TaskHandle_t CO2_measurementTask,BME_measurementTask,
 HTU_measurementTask,BH1750_measurementTask,CH2O_measurementTask,
  PMS_measurementTask,MS5611_measurementTask,VEML_measurementTask,
  DISPLAY_measurementTask, INMP441_measurementTask, INA226_measurementTask;

// Функция проверки валидности числа
bool is_valid_float(float value);

#endif