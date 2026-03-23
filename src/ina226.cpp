#include "ina226.h"
#include "logger.h"
#include <Wire.h>

INA226_Sensor ina226;

// Регистры INA226
#define INA226_REG_CONFIG       0x00
#define INA226_REG_SHUNT        0x01
#define INA226_REG_BUS          0x02
#define INA226_REG_POWER        0x03
#define INA226_REG_CURRENT      0x04
#define INA226_REG_CALIBRATION  0x05

// Current_LSB для расчёта тока (вычисляется в begin())
static float ina226_currentLSB = 0.0;

// Инициализация INA226
bool INA226_Sensor::begin() {
    // Проверка наличия устройства
    Wire.beginTransmission(INA226_ADDRESS);
    if (Wire.endTransmission() != 0) {
        LOG_ERROR(INA226, "Device not found!");
        return false;
    }

    // Расчёт калибровки
    // Current_LSB = Max_Current / 32768
    ina226_currentLSB = INA226_MAX_CURRENT / 32768.0;
    // Calibration = 0.00512 / (Current_LSB * Shunt)
    calibration = (uint16_t)(0.00512 / (ina226_currentLSB * INA226_SHUNT));

    LOG_DEBUG_FMT(INA226, "Calibration: %d, Current_LSB: %.6f A", calibration, ina226_currentLSB);

    // Запись калибровки
    writeRegister(INA226_REG_CALIBRATION, calibration);

    // Конфигурация:
    // AVG=16 (усреднение 16 отсчётов)
    // VBUS=1.1ms, VSHUNT=1.1ms
    // MODE=7 (Shunt and Bus, Continuous)
    // 0x4127 = 0100 0001 0010 0111
    writeRegister(INA226_REG_CONFIG, 0x4127);

    // Задержка на стабилизацию
    delay(100);

    LOG_INFO(INA226, "Initialized");
    return true;
}

// Чтение данных с INA226
void INA226_Sensor::read() {
    // Чтение напряжения шунта (регистр 0x01)
    int16_t shunt = (int16_t)readRegister(INA226_REG_SHUNT);
    shunt_voltage = shunt * 2.5;  // 2.5 μV per LSB → μV

    // Чтение напряжения шины (регистр 0x02)
    uint16_t bus = readRegister(INA226_REG_BUS);
    bus_voltage = bus * 1.25;  // 1.25 mV per LSB → mV
    bus_voltage = bus_voltage / 1000.0;  // Перевод в V

    // Чтение тока (регистр 0x04)
    int16_t current_raw = (int16_t)readRegister(INA226_REG_CURRENT);
    current = current_raw * ina226_currentLSB;  // Перевод в Amperes

    // Чтение мощности (регистр 0x03)
    uint16_t power_raw = readRegister(INA226_REG_POWER);
    power = power_raw * 25 * ina226_currentLSB;  // Power_LSB = 25 × Current_LSB → Watts

    // Отладка
    LOG_DEBUG_FMT(INA226, "V: %.3fV, I: %.3fA, P: %.3fW (I_raw=%d, P_raw=%d)",
                  bus_voltage, current, power, current_raw, power_raw);
}

// Запись в регистр INA226
bool INA226_Sensor::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(INA226_ADDRESS);
    Wire.write(reg);
    Wire.write(highByte(value));
    Wire.write(lowByte(value));
    return Wire.endTransmission() == 0;
}

// Чтение из регистра INA226
uint16_t INA226_Sensor::readRegister(uint8_t reg) {
    Wire.beginTransmission(INA226_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();
    
    Wire.requestFrom(INA226_ADDRESS, 2);
    if (Wire.available() >= 2) {
        return (Wire.read() << 8) | Wire.read();
    }
    return 0;
}
