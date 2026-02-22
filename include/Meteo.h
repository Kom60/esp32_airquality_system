#include "Arduino.h"

/*
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
    //void update_indoor_temp();
    //void update_indoor_press();
    //void update_pm();
    //void update_lightning();
    //void update_co2();
    //void update_ch20();
    //void update_radiation();
};
*/
class Meteo_data
{
public:
    // BME280 sensor data
    float bme_temperature{};
    float bme_pressure{};
    float bme_humidity{};
    
    // HTU21DF sensor data
    float htu_temperature{};
    float htu_humidity{};
    
    // SCD4X sensor data
    float scd4x_co2{};
    float scd4x_temperature{};
    float scd4x_humidity{};
    
    // PMS sensor data
    unsigned int pms_pm1{};
    unsigned int pms_pm2_5{};
    unsigned int pms_pm10{};
    
    // MS5611 sensor data
    double ms5611_pressure{};
    float ms5611_temperature{};

    // BH1750 sensor data
    float bh1750_lighting{};
    
    // VEML6070 sensor data
    uint16_t veml_uv{};
    
    // CH2O (formaldehyde) sensor data
    float ch2o_value{};
    
    // Microphone noise data
    float microphone_noise{};
    
    // Constructor
    Meteo_data();
    
    // Print methods
    void print_values();
    void print_bme_values();
    void print_htu_values();
    void print_scd4x_values();
    void print_pms_values();
    void print_ms5611_values();
    void print_bh1750_values();
    void print_veml_values();
    void print_ch2o_values();
    void print_microphone_values();
    
    // Update methods
    void update_bme_data(float temperature, float pressure, float humidity);
    void update_htu_data(float temperature, float humidity);
    void update_scd4x_data(float co2, float temperature, float humidity);
    void update_pms_data(unsigned int PM01, unsigned int PM025, unsigned int PM10);
    void update_ms5611_data(double pressure, float temperature);
    void update_bh1750_data(float lighting_val);
    void update_veml_data(uint16_t uv_val);
    void update_ch2o_data(float ch2o_val);
    void update_microphone_data(float noise_val);
};

extern Meteo_data AIR_data;
