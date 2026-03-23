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

void Meteo_data::print_bme_values()
{
    LOG_DEBUG_FMT(BME, "BME280 SENSOR: T=%.2f C, P=%.2f hPa, H=%.2f %%",
                  bme_temperature, bme_pressure, bme_humidity);
}

void Meteo_data::print_htu_values()
{
    LOG_DEBUG_FMT(HTU, "HTU21DF SENSOR: T=%.2f C, H=%.2f %%",
                  htu_temperature, htu_humidity);
}

void Meteo_data::print_scd4x_values()
{
    LOG_DEBUG_FMT(SCD4X, "SCD4X SENSOR: CO2=%.0f ppm, T=%.2f C, H=%.0f %%",
                  scd4x_co2, scd4x_temperature, scd4x_humidity);
}

void Meteo_data::print_pms_values()
{
    LOG_DEBUG_FMT(PMS, "PMS SENSOR: PM1=%d, PM2.5=%d, PM10=%d [ug/m3]",
                  pms_pm1, pms_pm2_5, pms_pm10);
}

void Meteo_data::print_ms5611_values()
{
    LOG_DEBUG_FMT(MS5611, "MS5611 SENSOR: T=%.2f C, P=%.2f hPa",
                  ms5611_temperature, ms5611_pressure);
}

void Meteo_data::print_bh1750_values()
{
    LOG_DEBUG_FMT(BH1750, "BH1750 SENSOR: Lighting=%.1f lux",
                  bh1750_lighting);
}

void Meteo_data::print_veml_values()
{
    LOG_DEBUG_FMT(VEML, "VEML6070 SENSOR: UV=%d UV index",
                  veml_uv);
}

void Meteo_data::print_ch2o_values()
{
    LOG_DEBUG_FMT(CH2O, "CH2O SENSOR: CH2O=%.1f ppm",
                  ch2o_value);
}

void Meteo_data::print_microphone_values()
{
    LOG_DEBUG_FMT(MICROPHONE, "MICROPHONE: Noise=%.2f dB",
                  microphone_noise);
}

void Meteo_data::print_ina226_values()
{
    LOG_DEBUG_FMT(INA226, "INA226 SENSOR: V=%.3f V, I=%.3f A, P=%.3f W",
                  ina226_voltage, ina226_current, ina226_power);
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
    print_ina226_values();
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

// Вспомогательная функция для проверки валидности значения влажности (0-100%)
static bool is_valid_humidity(float h)
{
    return !isnan(h) && h >= 0.0f && h <= 100.0f;
}

// Простое среднее арифметическое всех валидных датчиков
float Meteo_data::get_average_humidity()
{
    float sum = 0.0f;
    int count = 0;

    if (is_valid_humidity(bme_humidity)) {
        sum += bme_humidity;
        count++;
    }
    if (is_valid_humidity(htu_humidity)) {
        sum += htu_humidity;
        count++;
    }
    if (is_valid_humidity(scd4x_humidity)) {
        sum += scd4x_humidity;
        count++;
    }

    return (count > 0) ? sum / count : 0.0f;
}

// Взвешенное среднее (SCD40 наиболее точный, BME280 средний, HTU21DF базовый)
float Meteo_data::get_weighted_humidity()
{
    float weighted_sum = 0.0f;
    float weight_sum = 0.0f;

    if (is_valid_humidity(bme_humidity)) {
        weighted_sum += bme_humidity * 0.3f;
        weight_sum += 0.3f;
    }

    if (is_valid_humidity(htu_humidity)) {
        weighted_sum += htu_humidity * 0.2f;
        weight_sum += 0.2f;
    }

    if (is_valid_humidity(scd4x_humidity)) {
        weighted_sum += scd4x_humidity * 0.5f;
        weight_sum += 0.5f;
    }

    return (weight_sum > 0.0f) ? weighted_sum / weight_sum : 0.0f;
}

// Медианный фильтр + отбрасывание выбросов
float Meteo_data::get_filtered_humidity()
{
    float values[3];
    int count = 0;

    if (is_valid_humidity(bme_humidity)) {
        values[count++] = bme_humidity;
    }
    if (is_valid_humidity(htu_humidity)) {
        values[count++] = htu_humidity;
    }
    if (is_valid_humidity(scd4x_humidity)) {
        values[count++] = scd4x_humidity;
    }

    if (count == 0) return 0.0f;
    if (count == 1) return values[0];

    // Сортировка для медианы
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (values[i] > values[j]) {
                float tmp = values[i];
                values[i] = values[j];
                values[j] = tmp;
            }
        }
    }

    if (count == 3) {
        return values[1]; // медиана
    } else {
        return (values[0] + values[1]) / 2.0f;
    }
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

// Комбинированная влажность: скользящее среднее по каждому датчику + взвешивание
float MeteoBuffer::get_weighted_humidity()
{
    int count = filled ? METEO_WINDOW_SIZE : index;
    if (count == 0) return 0.0f;

    // Усредняем каждый датчик отдельно
    float bme_sum = 0.0f, htu_sum = 0.0f, scd_sum = 0.0f;
    int bme_count = 0, htu_count = 0, scd_count = 0;

    for (int i = 0; i < count; i++) {
        if (is_valid_humidity(buffer[i].bme_humidity)) {
            bme_sum += buffer[i].bme_humidity;
            bme_count++;
        }
        if (is_valid_humidity(buffer[i].htu_humidity)) {
            htu_sum += buffer[i].htu_humidity;
            htu_count++;
        }
        if (is_valid_humidity(buffer[i].scd4x_humidity)) {
            scd_sum += buffer[i].scd4x_humidity;
            scd_count++;
        }
    }

    float bme_avg = (bme_count > 0) ? bme_sum / bme_count : 0.0f;
    float htu_avg = (htu_count > 0) ? htu_sum / htu_count : 0.0f;
    float scd_avg = (scd_count > 0) ? scd_sum / scd_count : 0.0f;

    // Взвешенное среднее (SCD40=0.5, BME280=0.3, HTU21DF=0.2)
    float weighted_sum = 0.0f;
    float weight_sum = 0.0f;

    if (bme_count > 0) {
        weighted_sum += bme_avg * 0.3f;
        weight_sum += 0.3f;
    }
    if (htu_count > 0) {
        weighted_sum += htu_avg * 0.2f;
        weight_sum += 0.2f;
    }
    if (scd_count > 0) {
        weighted_sum += scd_avg * 0.5f;
        weight_sum += 0.5f;
    }

    return (weight_sum > 0.0f) ? weighted_sum / weight_sum : 0.0f;
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
