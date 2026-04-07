#include "Meteo.h"
#include "headers.h"
#include "logger.h"

// Глобальный экземпляр буфера
MeteoBuffer meteo_buffer;

// ==================== Meteo_data ====================

Meteo_data::Meteo_data()
    : bme_temperature{0}
    , bme_pressure{0}
    , bme_humidity{0}
    , htu_temperature{0}
    , htu_humidity{0}
    , scd4x_co2{0}
    , scd4x_temperature{0}
    , scd4x_humidity{0}
    , pms_pm1{0}
    , pms_pm2_5{0}
    , pms_pm10{0}
    , ms5611_pressure{0}
    , ms5611_temperature{0}
    , bh1750_lighting{0}
    , veml_uv{0}
    , ch2o_value{0}
    , microphone_noise{0}
    , ina226_voltage{0}
    , ina226_current{0}
    , ina226_power{0}
{
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

void Meteo_data::update_ms5611_data(double pressure, float temperature)
{
    ms5611_pressure = pressure;
    ms5611_temperature = temperature;
}

void Meteo_data::update_bh1750_data(float lighting_val)
{
    if (is_valid_float(lighting_val) && lighting_val >= 0 && lighting_val <= 150000)
    {
        bh1750_lighting = lighting_val;
    }
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

void Meteo_data::update_ina226_data(float voltage, float current, float power)
{
    ina226_voltage = voltage;
    ina226_current = current;
    ina226_power = power;
}

// ==================== MeteoBuffer ====================

MeteoBuffer::MeteoBuffer()
    : index{0}
    , filled{false}
{
}

// Добавить текущее значение AIR_data в буфер (копирование)
void MeteoBuffer::push()
{
    buffer[index].bme_temperature = AIR_data.bme_temperature;
    buffer[index].bme_pressure = AIR_data.bme_pressure;
    buffer[index].bme_humidity = AIR_data.bme_humidity;

    buffer[index].htu_temperature = AIR_data.htu_temperature;
    buffer[index].htu_humidity = AIR_data.htu_humidity;

    buffer[index].scd4x_co2 = AIR_data.scd4x_co2;
    buffer[index].scd4x_temperature = AIR_data.scd4x_temperature;
    buffer[index].scd4x_humidity = AIR_data.scd4x_humidity;

    buffer[index].pms_pm1 = AIR_data.pms_pm1;
    buffer[index].pms_pm2_5 = AIR_data.pms_pm2_5;
    buffer[index].pms_pm10 = AIR_data.pms_pm10;

    buffer[index].ms5611_pressure = AIR_data.ms5611_pressure;
    buffer[index].ms5611_temperature = AIR_data.ms5611_temperature;

    buffer[index].bh1750_lighting = AIR_data.bh1750_lighting;

    buffer[index].veml_uv = AIR_data.veml_uv;

    buffer[index].ch2o_value = AIR_data.ch2o_value;

    buffer[index].microphone_noise = AIR_data.microphone_noise;

    buffer[index].ina226_voltage = AIR_data.ina226_voltage;
    buffer[index].ina226_current = AIR_data.ina226_current;
    buffer[index].ina226_power = AIR_data.ina226_power;

    // Инкремент индекса
    index++;
    if (index >= METEO_WINDOW_SIZE) {
        index = 0;
        filled = true;
    }
}

// Вспомогательные шаблоны для усреднения
template<typename T>
static T avg_value(T (Meteo_data::*member)(), uint8_t window_size, bool filled, uint8_t index)
{
    T sum = 0;
    int count = filled ? window_size : index;
    if (count == 0) return 0;

    for (int i = 0; i < count; i++) {
        sum += (meteo_buffer.buffer[i].*member)();
    }
    return sum / count;
}

float MeteoBuffer::get_avg_bme_temperature()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].bme_temperature;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_bme_pressure()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].bme_pressure;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_bme_humidity()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].bme_humidity;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_htu_temperature()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].htu_temperature;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_htu_humidity()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].htu_humidity;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_scd4x_co2()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].scd4x_co2;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_scd4x_temperature()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].scd4x_temperature;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_scd4x_humidity()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].scd4x_humidity;
    }
    return sum / count;
}

unsigned int MeteoBuffer::get_avg_pms_pm1()
{
    unsigned int sum = 0;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].pms_pm1;
    }
    return sum / count;
}

unsigned int MeteoBuffer::get_avg_pms_pm2_5()
{
    unsigned int sum = 0;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].pms_pm2_5;
    }
    return sum / count;
}

unsigned int MeteoBuffer::get_avg_pms_pm10()
{
    unsigned int sum = 0;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].pms_pm10;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_ms5611_pressure()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].ms5611_pressure;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_ms5611_temperature()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].ms5611_temperature;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_bh1750_lighting()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].bh1750_lighting;
    }
    return sum / count;
}

uint16_t MeteoBuffer::get_avg_veml_uv()
{
    uint16_t sum = 0;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].veml_uv;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_ch2o_value()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].ch2o_value;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_microphone_noise()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].microphone_noise;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_ina226_voltage()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].ina226_voltage;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_ina226_current()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].ina226_current;
    }
    return sum / count;
}

float MeteoBuffer::get_avg_ina226_power()
{
    float sum = 0.0f;
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;
    for (int i = 0; i < count; i++) {
        sum += buffer[i].ina226_power;
    }
    return sum / count;
}

// ==================== EMA (Exponential Moving Average) ====================

// Обновление EMA (вызывать после push())
void MeteoBuffer::update_ema()
{
    float current_light = AIR_data.bh1750_lighting;
    float current_uv = (float)AIR_data.veml_uv;
    float current_noise = AIR_data.microphone_noise;
    
    if (!ema_initialized) {
        // Инициализация первыми значениями
        ema_bh1750 = current_light;
        ema_veml_uv = current_uv;
        ema_microphone = current_noise;
        ema_initialized = true;
    } else {
        // EMA формула: EMA = α × new + (1 - α) × old
        ema_bh1750 = ema_alpha * current_light + (1.0f - ema_alpha) * ema_bh1750;
        ema_veml_uv = ema_alpha * current_uv + (1.0f - ema_alpha) * ema_veml_uv;
        ema_microphone = ema_alpha * current_noise + (1.0f - ema_alpha) * ema_microphone;
    }
}

// Получить EMA освещённости
float MeteoBuffer::get_ema_bh1750_lighting()
{
    return ema_initialized ? ema_bh1750 : 0.0f;
}

// Получить EMA UV
float MeteoBuffer::get_ema_veml_uv()
{
    return ema_initialized ? ema_veml_uv : 0.0f;
}

// Получить EMA шума
float MeteoBuffer::get_ema_microphone_noise()
{
    return ema_initialized ? ema_microphone : 0.0f;
}
