#include "headers.h"

TaskHandle_t CO2_measurementTask, BME_measurementTask,
    HTU_measurementTask, BH1750_measurementTask, CH2O_measurementTask,
    PMS_measurementTask, MS5611_measurementTask, VEML_measurementTask;

void CO2_measurementTaskFunction(void *parameter)
{
    Wire.begin();
    co2.begin();
    co2.startPeriodicMeasurement();
    while (true)
    {
        if (co2.isDataReady())
        {
            if (co2.readMeasurement(co2Value, temperature, humidity) == 0)
            {
                sendJson("scd4x_co2", String(co2Value));
                sendJson("scd4x_temperature", String(temperature * 100));
                sendJson("scd4x_humidity", String(humidity * 100));
                AIR_data.update_scd4x_data(co2Value, temperature, humidity);

                Serial.printf("CO2: %.0f ppm, Temperature: %.1f °C, Humidity: %.0f %%RH\n", co2Value, temperature, humidity);
                vTaskDelay(pdMS_TO_TICKS(4750)); // New data available after approximately 5 seconds
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
    }
}

void BME_measurementTaskFunction(void *parameter)
{
    while (true)
    {
        AIR_data.update_bme_data(bme.readTemperature(), bme.readPressure() / 100.0F, bme.readHumidity());
        sendJson("bme_temperature", String(AIR_data.bme_temperature * 100));
        sendJson("bme_pressure", String(AIR_data.bme_pressure * 100));
        sendJson("bme_humidity", String(AIR_data.bme_humidity * 100));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void HTU_measurementTaskFunction(void *parameter)
{
    while (true)
    {
        float temp = htu.readTemperature();
        float hum = htu.readHumidity();
        AIR_data.update_htu_data(temp, hum);
        sendJson("htu_temperature", String(AIR_data.htu_temperature * 100));
        sendJson("htu_humidity", String(AIR_data.htu_humidity * 100));
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void BH1750_measurementTaskFunction(void *parameter)
{
    while (true)
    {
        if (lightMeter.measurementReady())
        {
            float lux = lightMeter.readLightLevel();
            AIR_data.update_bh1750_data(lux);
            sendJson("bh1750_lighting", String(AIR_data.bh1750_lighting * 10));
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void CH2O_measurementTaskFunction(void *parameter)
{
    while (true)
    {
        float formaldehyde = analogRead(formaldehyde_Pin) / 496.36;
        AIR_data.update_ch2o_data(formaldehyde);
        sendJson("ch2o_value", String(AIR_data.ch2o_value * 10));
        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}

void PMS_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(6000));
    while (true)
    {
        pms.read();
        AIR_data.update_pms_data(pms.pm01, pms.pm25, pms.pm10);
        sendJson("pms_pm1", String(pms.pm01 * 10));
        sendJson("pms_pm2_5", String(pms.pm25 * 10));
        sendJson("pms_pm10", String(pms.pm10 * 10));
        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}

void MS5611_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        double pressure = ms5611.readPressure();
        float temperature = ms5611.readTemperature();

        AIR_data.update_ms5611_data(pressure, temperature);
        sendJson("ms5611_pressure", String(pressure / 100.0));  // Па -> гПа
        sendJson("ms5611_temperature", String(temperature * 100));

        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}

void VEML_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    while (true)
    {
        AIR_data.update_veml_data(uv.readUV());
        sendJson("veml_uv", String(AIR_data.veml_uv));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}