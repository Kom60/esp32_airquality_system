#include "headers.h"

TaskHandle_t CO2_measurementTask, BME_measurementTask,
	HTU_measurementTask, BH1750_measurementTask, CH2O_measurementTask,
	PMS_measurementTask, MS5611_measurementTask, VEML_measurementTask;

void CO2_measurementTaskFunction(void *parameter)
{
	co2.begin();
	co2.startPeriodicMeasurement();

	while (true)
	{
		if (co2.isDataReady())
		{
			if (co2.readMeasurement(co2Value, temperature, humidity) == 0)
			{
				sendJson("indoor_CO2", String(co2Value));
				AIR_data.update_CO2_data(co2Value);

				// Serial.printf("CO2: %.0f ppm, Temperature: %.1f °C, Humidity: %.0f %%RH\n", co2Value, temperature, humidity);
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
		sendJson("outdoor_temp", String(AIR_data.Outdoor_temp * 100));
		sendJson("outdoor_press", String(AIR_data.Outdoor_pressure * 100));
		sendJson("outdoor_humidity", String(AIR_data.Outdoor_Humidity * 100));
		vTaskDelay(pdMS_TO_TICKS(1000)); // New data available after approximately 5 seconds
	}
	vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
}
void HTU_measurementTaskFunction(void *parameter)
{
	float temp;
	float hum;
	while (true)
	{
		float temp = htu.readTemperature();
		float hum = htu.readHumidity();
		AIR_data.update_htu_data(temp, hum);
		sendJson("indoor_temp", String(AIR_data.Indoor_temp * 100));
		sendJson("indoor_humidity", String(AIR_data.Indoor_humidity * 100));
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
	vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
}
void BH1750_measurementTaskFunction(void *parameter)
{
	float lux = 0;
	while (true)
	{
		if (lightMeter.measurementReady())
		{
			lux = lightMeter.readLightLevel();
		}
		AIR_data.update_BH1750_data(lux);
		sendJson("indoor_light", String(AIR_data.Lighting * 10));
		vTaskDelay(pdMS_TO_TICKS(500)); // New data available after approximately 5 seconds
	}
	vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
}
void CH2O_measurementTaskFunction(void *parameter)
{
	float formaldehyde = 0.0;
	formaldehyde = analogRead(formaldehyde_Pin);
	while (true)
	{
		AIR_data.update_CH2O_data(formaldehyde / 496.36);
		sendJson("indoor_CH2O", String(AIR_data.CH2O * 10));
		vTaskDelay(pdMS_TO_TICKS(6000));
	}
	vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
}

void PMS_measurementTaskFunction(void *parameter)
{
	vTaskDelay(pdMS_TO_TICKS(6000));
	while (true)
	{
		// vTaskDelay(pdMS_TO_TICKS(6000));
		pms.read();
		AIR_data.update_pms_data(pms.pm01, pms.pm25, pms.pm10);
		sendJson("indoor_1_0", String(pms.pm01 * 10));
		sendJson("indoor_pm2_5", String(pms.pm25 * 10));
		sendJson("indoor_pm10", String(pms.pm10 * 10));
		vTaskDelay(pdMS_TO_TICKS(6000));
	}
	vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
}

void MS5611_measurementTaskFunction(void *parameter)
{
	vTaskDelay(pdMS_TO_TICKS(100));
	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(500));
		AIR_data.update_MS5611_data(ms5611.readPressure());
		sendJson("indoor_press", String(AIR_data.Indoor_pressure));
		vTaskDelay(pdMS_TO_TICKS(6000));
	}
	vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
}

void VEML_measurementTaskFunction(void *parameter)
{
	vTaskDelay(pdMS_TO_TICKS(100));
	while (true)
	{
		sendJson("indoor_noise", String(uv.readUV()));
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
}

