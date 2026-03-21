#ifndef ina226_h
#define ina226_h

#include <Arduino.h>
#include <Wire.h>

// INA226 I2C адрес (зависит от пинов A0, A1)
// A0=VCC, A1=GND → 0x41 (изменено для избежания конфликта с HTU31)
// A0=GND, A1=GND → 0x40
// A0=GND, A1=VCC → 0x44
// A0=VCC, A1=VCC → 0x45
#define INA226_ADDRESS 0x41

// Шунт в Омах (обычно 0.002 или 0.01)
#define INA226_SHUNT 0.1

// Максимальный ожидаемый ток (A)
#define INA226_MAX_CURRENT 1.0

class INA226_Sensor {
public:
    bool begin();
    void read();
    
    float bus_voltage;    // Напряжение шины (V)
    float shunt_voltage;  // Напряжение шунта (mV)
    float current;        // Ток (A)
    float power;          // Мощность (W)
    
private:
    uint16_t calibration;
    bool writeRegister(uint8_t reg, uint16_t value);
    uint16_t readRegister(uint8_t reg);
};

extern INA226_Sensor ina226;

#endif
