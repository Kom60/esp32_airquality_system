#include "Arduino.h"

class Meteo_data
{
  public:
    float Outdoor_temp{};
    float Outdoor_pressure{};
    float Outdoor_Humidity{};
    uint16_t Outdoor_UV{};

    unsigned int Indoor_PM1{};
    unsigned int Indoor_PM2{};
    unsigned int Indoor_PM10{};

    float Indoor_temp{};
    double Indoor_pressure{};
    float Indoor_humidity{};
    unsigned int Radiation{};
    float Lighting{};
    float CO2{};
    float CH2O{};
    float Indoor_noise{};

    Meteo_data();
    void print_values();
    void print_outdoor_values();
    void print_indoor_values();
    //void update_all();
    void update_bme_data(float temperature, float pressure, float humidity);
    void update_pms_data(unsigned int PM01,unsigned int PM025,unsigned int PM10);
    void update_htu_data(float temperature,float humidity);
    void update_MS5611_data(double pressure);
    void update_BH1750_data(float Lighting_val);
    void update_CH2O_data(float CH2O_val);
    void update_CO2_data(float CO2_val);
    void update_UV_data(uint16_t UV_val);
    void update_noise_data(float noise_val);
    
    //void update_indoor_temp(float temperature, float pressure, float humidity);
    /*void update_indoor_temp();
    void update_indoor_press();
    void update_pm();
    void update_lightning();
    void update_co2();
    void update_ch20();
    void update_radiation();*/
};

extern Meteo_data AIR_data;
