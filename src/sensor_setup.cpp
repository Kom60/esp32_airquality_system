#include "headers.h"

extern SemaphoreHandle_t i2c_mutex;

// Инициализация I2C шины и создание mutex
void sensors_setup()
{
  // Создаём mutex для защиты I2C шины
  i2c_mutex = xSemaphoreCreateMutex();

  // Инициализация I2C
  Wire.begin();
  Wire.setClock(100000); // 100 kHz для стабильности всех датчиков
  Serial.println("[I2C] Mutex создан, частота 100 kHz");

  // Группа 1: Быстрые I2C датчики (HTU, MS5611, BME)
  htu_setup();
  vTaskDelay(pdMS_TO_TICKS(50));

  MS5611_setup();
  vTaskDelay(pdMS_TO_TICKS(50));

  bme_setup();
  vTaskDelay(pdMS_TO_TICKS(50));

  // Группа 2: Остальные I2C датчики (BH1750, VEML)
  BH1750_setup();
  vTaskDelay(pdMS_TO_TICKS(50));

  VEML_setup();
  vTaskDelay(pdMS_TO_TICKS(50));

  // Группа 3: UART и Analog датчики (не требуют I2C задержек)
  PMS_setup();
  CH2O_setup();
  vTaskDelay(pdMS_TO_TICKS(50));

  // SCD40 требует особой последовательности инициализации
  SCD40_setup();

  Serial.println("[SENSORS] Все датчики инициализированы");
}

void bme_setup(){
    while(!Serial);    // time to get serial running
    Serial.println(F("BME280 test"));

    unsigned status;

    // Захватываем mutex перед инициализацией
    if (i2c_mutex != NULL) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        status = bme.begin(0x76);
        xSemaphoreGive(i2c_mutex);
    } else {
        status = bme.begin(0x76);
    }

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
    bool htu_found = false;
    
    // Захватываем mutex перед инициализацией
    if (i2c_mutex != NULL) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        htu_found = htu.begin();
        xSemaphoreGive(i2c_mutex);
    } else {
        htu_found = htu.begin();
    }

    if (!htu_found) {
        Serial.println("Check circuit. HTU21D not found!");
        while (1);
    }

    Serial.println("[HTU21D] Connected");
    xTaskCreatePinnedToCore(HTU_measurementTaskFunction, "HTUMeasurementTask", 2048, NULL, 1, &HTU_measurementTask, 0);
}

void BH1750_setup()
{
    bool bh1750_ok = false;
    
    // Захватываем mutex перед инициализацией
    if (i2c_mutex != NULL) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        bh1750_ok = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
        xSemaphoreGive(i2c_mutex);
    } else {
        bh1750_ok = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    }

    if (bh1750_ok) {
        Serial.println(F("BH1750 Advanced begin"));
    } else {
        Serial.println(F("Error initialising BH1750"));
    }

    xTaskCreatePinnedToCore(BH1750_measurementTaskFunction, "BH1750MeasurementTask", 2048, NULL, 1, &BH1750_measurementTask, 0);
}

void MS5611_setup()
{
    // Захватываем mutex перед инициализацией
    if (i2c_mutex != NULL) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        ms5611.begin(MS5611_HIGH_RES);
        xSemaphoreGive(i2c_mutex);
    } else {
        ms5611.begin(MS5611_HIGH_RES);
    }

    Serial.println("[MS5611] Connected");
    xTaskCreatePinnedToCore(MS5611_measurementTaskFunction, "MS5611MeasurementTask", 2048, NULL, 1, &MS5611_measurementTask, 0);
}

void SCD40_setup()
{
    //co2.setCalibrationMode(false);
    xTaskCreatePinnedToCore(CO2_measurementTaskFunction, "CO2MeasurementTask", 4000, NULL, 1, &CO2_measurementTask, 0);
}

void PMS_setup(){
    pms.init();
    xTaskCreatePinnedToCore(PMS_measurementTaskFunction, "PMSMeasurementTask", 2048, NULL, 1, &PMS_measurementTask, 0);
}

void CH2O_setup(){
    xTaskCreatePinnedToCore(CH2O_measurementTaskFunction, "CH2OMeasurementTask", 2048, NULL, 1, &CH2O_measurementTask, 0);
}

void VEML_setup(){
    // Захватываем mutex перед инициализацией
    if (i2c_mutex != NULL) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        uv.begin(VEML6070_1_T);  // pass in the integration time constant
        xSemaphoreGive(i2c_mutex);
    } else {
        uv.begin(VEML6070_1_T);  // pass in the integration time constant
    }
    
    Serial.println("[VEML6070] Connected");
    xTaskCreatePinnedToCore(VEML_measurementTaskFunction, "VEMLMeasurementTask", 2048, NULL, 1, &VEML_measurementTask, 0);
}