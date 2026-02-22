#include "Meteo.h"

Meteo_data::Meteo_data()
      :bme_temperature{0}
      ,bme_pressure{0}
      ,bme_humidity{0}
      ,htu_temperature{0}
      ,htu_humidity{0}
      ,scd4x_co2{0}
      ,scd4x_temperature{0}
      ,scd4x_humidity{0}
      ,pms_pm1{0}
      ,pms_pm2_5{0}
      ,pms_pm10{0}
      ,ms5611_pressure{0}
      ,ms5611_temperature{0}
      ,bh1750_lighting{0}
      ,veml_uv{0}
      ,ch2o_value{0}
      ,microphone_noise{0}
{

}


void Meteo_data::print_bme_values()
{
  Serial.println("BME280 SENSOR:");
  Serial.print("T:\t");
  Serial.print(bme_temperature, 2);
  Serial.println(" Celsium");

  Serial.print("P:\t");
  Serial.print(bme_pressure, 2);
  Serial.println(" hPa");

  Serial.print("Hum.:\t");
  Serial.print(bme_humidity, 2);
  Serial.println(" %");
  Serial.println();
}

void Meteo_data::print_htu_values()
{
  Serial.println("HTU21DF SENSOR:");
  Serial.print("T:\t");
  Serial.print(htu_temperature, 2);
  Serial.println(" Celsium");

  Serial.print("Hum.:\t");
  Serial.print(htu_humidity, 2);
  Serial.println(" %");
  Serial.println();
}

void Meteo_data::print_scd4x_values()
{
  Serial.println("SCD4X SENSOR:");
  Serial.print("CO2:\t");
  Serial.print(scd4x_co2, 2);
  Serial.println(" ppm");

  Serial.print("T:\t");
  Serial.print(scd4x_temperature, 2);
  Serial.println(" Celsium");

  Serial.print("Hum.:\t");
  Serial.print(scd4x_humidity, 2);
  Serial.println(" %");
  Serial.println();
}

void Meteo_data::print_pms_values()
{
  Serial.println("PMS SENSOR:");
  Serial.print("PM0.1:\t");
  Serial.print(pms_pm1);
  Serial.println(" [ug/m3]");

  Serial.print("PM2.5:\t");
  Serial.print(pms_pm2_5);
  Serial.println(" [ug/m3]");

  Serial.print("PM10.0:\t");
  Serial.print(pms_pm10);
  Serial.println(" [ug/m3]");
  Serial.println();
}

void Meteo_data::print_ms5611_values()
{
  Serial.println("MS5611 SENSOR:");
  Serial.print("T:\t");
  Serial.print(ms5611_temperature, 2);
  Serial.println(" Celsium");
  
  Serial.print("P:\t");
  Serial.print(ms5611_pressure, 2);
  Serial.println(" hPa");
  Serial.println();
}

void Meteo_data::print_bh1750_values()
{
  Serial.println("BH1750 SENSOR:");
  Serial.print("Lighting:\t");
  Serial.print(bh1750_lighting, 1);
  Serial.println(" lux");
  Serial.println();
}

void Meteo_data::print_veml_values()
{
  Serial.println("VEML6070 SENSOR:");
  Serial.print("UV:\t");
  Serial.print(veml_uv);
  Serial.println(" UV index");
  Serial.println();
}

void Meteo_data::print_ch2o_values()
{
  Serial.println("CH2O SENSOR:");
  Serial.print("CH2O:\t");
  Serial.print(ch2o_value, 1);
  Serial.println(" ppm");
  Serial.println();
}

void Meteo_data::print_microphone_values()
{
  Serial.println("MICROPHONE:");
  Serial.print("Noise:\t");
  Serial.print(microphone_noise, 2);
  Serial.println(" dB");
  Serial.println();
}

void Meteo_data::print_values()
{
  print_bme_values();
  print_htu_values();
  print_scd4x_values();
  print_pms_values();
  print_ms5611_values();
  print_bh1750_values();
  print_veml_values();
  print_ch2o_values();
  print_microphone_values();
}

void Meteo_data::update_bme_data(float temperature, float pressure, float humidity)
{
  bme_temperature = temperature;
  bme_pressure = pressure;
  bme_humidity = humidity;
}

void Meteo_data::update_htu_data(float temperature, float humidity)
{
  htu_temperature = temperature;
  htu_humidity = humidity;
}

void Meteo_data::update_scd4x_data(float co2, float temperature, float humidity)
{
  scd4x_co2 = co2;
  scd4x_temperature = temperature;
  scd4x_humidity = humidity;
}

void Meteo_data::update_pms_data(unsigned int PM01, unsigned int PM025, unsigned int PM10)
{
  pms_pm1 = PM01;
  pms_pm2_5 = PM025;
  pms_pm10 = PM10;
}

void Meteo_data::update_ms5611_data(double pressure, float temperature)  // ОБНОВЛЕННЫЙ МЕТОД
{
  ms5611_pressure = pressure;
  ms5611_temperature = temperature;
}

void Meteo_data::update_bh1750_data(float lighting_val)
{
  bh1750_lighting = lighting_val;
}

void Meteo_data::update_veml_data(uint16_t uv_val)
{
  veml_uv = uv_val;
}

void Meteo_data::update_ch2o_data(float ch2o_val)
{
  ch2o_value = ch2o_val;
}

void Meteo_data::update_microphone_data(float noise_val)
{
  microphone_noise = noise_val;
}