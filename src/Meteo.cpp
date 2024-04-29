#include "Meteo.h"

Meteo_data::Meteo_data()
      :Indoor_temp{0}
      ,Indoor_humidity{0}
      ,Indoor_pressure{0}
      ,Radiation{0}
      ,Lighting{0}
      ,Indoor_PM1{0}
      ,Indoor_PM2{0}
      ,Indoor_PM10{0}
      ,CH2O{0}
      //Outdoor
      ,Outdoor_temp{0}
      ,Outdoor_pressure{0}
      ,Outdoor_Humidity{0}
{

}


    void Meteo_data::print_outdoor_values()
    {
      Serial.println("OUTDOOR AIR:");
      Serial.print("T:\t");
      Serial.print(Outdoor_temp, 2);
      Serial.println(" Celsium");

      Serial.print("P:\t");
      Serial.print(Outdoor_pressure, 2);
      Serial.println(" GPa");

      Serial.print("Hum.:\t");
      Serial.print(Outdoor_Humidity, 2);
      Serial.println(" %");
      Serial.println();
    }

    void Meteo_data::print_indoor_values()
    {
      Serial.println("INDOOR AIR:");
      Serial.print("T:\t");
      Serial.print(Indoor_temp, 2);
      Serial.println(" Celsium");

      Serial.print("P:\t");
      Serial.print(Indoor_pressure, 2);
      Serial.println(" GPa");

      Serial.print("Hum.:\t");
      Serial.print(Indoor_humidity, 2);
      Serial.println(" %");

      Serial.print("PM0.1:\t");
      Serial.print(Indoor_PM1);
      Serial.println(" [ug/m3]");

      Serial.print("PM2.5:\t");
      Serial.print(Indoor_PM2);
      Serial.println(" [ug/m3]");

      Serial.print("PM10.0:\t");
      Serial.print(Indoor_PM10);
      Serial.println(" [ug/m3]");

      Serial.print("Lighting:\t");
      Serial.print(Lighting, 1);
      Serial.println(" lux");

      Serial.print("CO2:\t");
      Serial.print(CO2, 2);
      Serial.println(" ppm");

      Serial.print("CH2O:\t");
      Serial.print(CH2O, 1);
      Serial.println(" ppm");

      Serial.print("Radiation:\t");
      Serial.print(Radiation);
      Serial.println(" uR");
      Serial.println();
    }

    void Meteo_data::print_values()
    {
      print_outdoor_values();
      print_indoor_values();
    }
/*
    void update_all()
    {

    }
*/
    void Meteo_data::update_bme_data(float temperature, float pressure, float humidity)
    {
      Outdoor_temp=temperature;
      Outdoor_pressure=pressure;
      Outdoor_Humidity=humidity;
    }

    /*void update_indoor_temp();
    void update_indoor_press();
    void update_pm();
    void update_lightning();
    void update_co2();
    void update_ch20();
    void update_radiation();*/

    void  Meteo_data::update_pms_data(unsigned int PM01,unsigned int PM025,unsigned int PM10)
    {
      Indoor_PM1=PM01;
      Indoor_PM2=PM025;
      Indoor_PM10=PM10;
    }

    void  Meteo_data::update_htu_data(float temperature,float humidity)
    {
      Indoor_temp=temperature;
      Indoor_humidity=humidity;
    }

    void  Meteo_data::update_MS5611_data(double pressure)
    {
        Indoor_pressure=pressure;
    }

    void  Meteo_data::update_BH1750_data(float Lighting_val)
    {
        Lighting=Lighting_val;
    }

    void Meteo_data::update_CH2O_data(float CH2O_val)
    {
        CH2O=CH2O_val;
    }

    void Meteo_data::update_CO2_data(float CO2_val)
    {
        CO2=CO2_val;
    }

    void Meteo_data::update_UV_data(uint16_t UV_val)
    {
        Outdoor_UV=UV_val;
    }

    void Meteo_data::update_noise_data(float noise_val)
    {
      Indoor_noise=noise_val;
    }
/*
Meteo_data::Meteo_data()
    {
      pms.read();
      MS5611.read(); 
      //Indoor
      Indoor_temp = htu.readTemperature();
      Indoor_humidity = htu.readHumidity();
      Indoor_pressure=MS5611.getPressure();
      Radiation=12;
      if (lightMeter.measurementReady()) 
      {
        Lighting = lightMeter.readLightLevel();
      }
      Indoor_PM1=pms.pm01;
      Indoor_PM2=pms.pm25;
      Indoor_PM10=pms.pm10;
      //Outdoor
      Outdoor_temp=bme.readTemperature();
      Outdoor_pressure=bme.readPressure();
      Outdoor_Humidity=bme.readHumidity();
      CH2O=analogRead(formaldehyde_Pin)/496.36;
    }


    void Meteo_data::print_values()
    {
      Serial.println("OUTDOOR AIR:");
      Serial.print("T:\t");
      Serial.print(Outdoor_temp, 2);
      Serial.println(" C");

      Serial.print("P:\t");
      Serial.print(Outdoor_pressure, 2);
      Serial.println(" GPa");

      Serial.print("Hum.:\t");
      Serial.print(Outdoor_Humidity, 2);
      Serial.println(" %");
      Serial.println();
    }
*/
