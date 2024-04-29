#include "headers.h"

TaskHandle_t CO2_measurementTask, BME_measurementTask,
	HTU_measurementTask, BH1750_measurementTask, CH2O_measurementTask,
	PMS_measurementTask, MS5611_measurementTask, VEML_measurementTask,
	INMP441_measurementTask;

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
		AIR_data.update_UV_data(uv.readUV());
		sendJson("outdoor_UV", String(AIR_data.Outdoor_UV));
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
}

void INMP441_measurementTaskFunction(void *parameter)
{
  while (true)
  {
    sum_queue_t q;
    uint32_t Leq_samples = 0;
    double Leq_sum_sqr = 0;
    double Leq_dB = 0;
    Serial.println("!!!!!!!!!!!!!!!!");
    // Read sum of samaples, calculated by 'i2s_reader_task'
    while (xQueueReceive(samples_queue, &q, portMAX_DELAY))
    {
      // Calculate dB values relative to MIC_REF_AMPL and adjust for microphone reference
      double short_RMS = sqrt(double(q.sum_sqr_SPL) / SAMPLES_SHORT);
      double short_SPL_dB = MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(short_RMS / MIC_REF_AMPL);
      // In case of acoustic overload or below noise floor measurement, report infinty Leq value
      if (short_SPL_dB > MIC_OVERLOAD_DB)
      {
        Leq_sum_sqr = INFINITY;
      }
      else if (isnan(short_SPL_dB) || (short_SPL_dB < MIC_NOISE_DB))
      {
        Leq_sum_sqr = -INFINITY;
      }
      // Accumulate Leq sum
      Leq_sum_sqr += q.sum_sqr_weighted;
      Leq_samples += SAMPLES_SHORT;

      // When we gather enough samples, calculate new Leq value
      if (Leq_samples >= SAMPLE_RATE * LEQ_PERIOD)
      {
        double Leq_RMS = sqrt(Leq_sum_sqr / Leq_samples);
        Leq_dB = MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(Leq_RMS / MIC_REF_AMPL);
        Leq_sum_sqr = 0;
        Leq_samples = 0;
      AIR_data.update_noise_data(Leq_dB);
		  sendJson("indoor_noise", String(10.0*AIR_data.Indoor_noise));
        // Serial output, customize (or remove) as needed
        //Serial.printf("%.1f\n", Leq_dB);
      }
      vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
                                     // Debug only
                                     // Serial.printf("%u processing ticks\n", q.proc_ticks);
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
  }
}