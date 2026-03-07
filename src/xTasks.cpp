#include "headers.h"

TaskHandle_t CO2_measurementTask, BME_measurementTask,
    HTU_measurementTask, BH1750_measurementTask, CH2O_measurementTask,
    PMS_measurementTask, MS5611_measurementTask, VEML_measurementTask;

void CO2_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(1500));  // Ждём завершения инициализации других датчиков

    Serial.println("[SCD40] === Инициализация SCD40 ===");

    // Инициализация SCD40 с защитой I2C mutex
    if (i2c_mutex != NULL) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);

        Wire.begin();
        Wire.setClock(400000);  // 400 kHz для быстрой инициализации

        // Проверяем наличие датчика на шине
        Wire.beginTransmission(0x62);
        uint8_t i2cErr = Wire.endTransmission();
        if (i2cErr == 0) {
            Serial.println("[SCD40] Датчик найден на адресе 0x62");
        } else {
            Serial.printf("[SCD40] ОШИБКА: Датчик не найден на 0x62 (error=%d)\n", i2cErr);
            Serial.println("[SCD40] Проверьте подключение и питание!");
        }

        uint8_t err = co2.begin();
        if (err != 0) {
            Serial.printf("[SCD40] Ошибка co2.begin(): %s\n", co2.getErrorText(err));
        } else {
            Serial.println("[SCD40] co2.begin() успешно");
        }

        // Принудительно останавливаем измерения (если были запущены из EEPROM)
        Serial.println("[SCD40] Остановка возможных измерений...");
        err = co2.stopPeriodicMeasurement();
        Serial.printf("[SCD40] stopPeriodicMeasurement: %s\n", co2.getErrorText(err));
        
        // Ждём пока датчик остановится (по спецификации 500мс)
        vTaskDelay(pdMS_TO_TICKS(500));

        xSemaphoreGive(i2c_mutex);
    } else {
        Serial.println("[SCD40] ОШИБКА: i2c_mutex не создан!");
    }

    // Ждём ~2 секунды, чтобы BME280 и MS5611 успели сделать первые замеры
    Serial.println("[SCD40] Ожидание данных давления от BME280/MS5611...");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Установка altitude и pressure для компенсации
    // Берём среднее давление из BME280 и MS5611
    float avgPressure = 0;
    if (AIR_data.bme_pressure > 0 && AIR_data.ms5611_pressure > 0) {
        avgPressure = (AIR_data.bme_pressure + AIR_data.ms5611_pressure) / 2.0f;
    } else if (AIR_data.bme_pressure > 0) {
        avgPressure = AIR_data.bme_pressure;
    } else if (AIR_data.ms5611_pressure > 0) {
        avgPressure = AIR_data.ms5611_pressure;
    }

    if (i2c_mutex != NULL) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);

        // Устанавливаем частоту 100kHz для надёжности записи команд
        Wire.setClock(100000);

        if (avgPressure > 0) {
            // Расчёт altitude по барометрической формуле (в метрах)
            float altitude = 44330.0f * (1.0f - pow(avgPressure / 1013.25f, 0.1903f));
            Serial.printf("[SCD40] Расчётное altitude: %.1f м (давление: %.2f гПа)\n", altitude, avgPressure);

            // Установка altitude в SCD40 (в метрах)
            uint16_t altitudeInt = (uint16_t)altitude;
            uint8_t err = co2.setSensorAltitude(altitudeInt);
            if (err == 0) {
                Serial.printf("[SCD40] ✓ Установлено altitude: %d м\n", altitudeInt);
            } else {
                Serial.printf("[SCD40] ✗ Ошибка установки altitude: %s\n", co2.getErrorText(err));
            }
        } else {
            Serial.println("[SCD40] ⚠ Давление не получено (BME280/MS5611 не готовы)");
        }

        // Теперь запускаем периодические измерения
        uint8_t err = co2.startPeriodicMeasurement();
        if (err != 0) {
            Serial.printf("[SCD40] Ошибка startPeriodicMeasurement(): %s\n", co2.getErrorText(err));
        } else {
            Serial.println("[SCD40] startPeriodicMeasurement() успешно");
        }

        xSemaphoreGive(i2c_mutex);
    }

    int errorCount = 0;
    int readAttemptCount = 0;
    int successCount = 0;
    unsigned long startTime = millis();
    unsigned long lastReadTime = 0;
    
    Serial.println("[SCD40] Начало цикла измерений...");
    Serial.println("[SCD40] Первое измерение займёт ~5 секунд");

    while (true)
    {
        unsigned long elapsed = millis() - startTime;
        unsigned long timeSinceLastRead = millis() - lastReadTime;
        
        // Пытаемся читать каждые 5 секунд
        if (timeSinceLastRead >= 5000 || lastReadTime == 0)
        {
            readAttemptCount++;
            Serial.printf("\n[SCD40] === Попытка #%d (elapsed=%lus) ===\n", readAttemptCount, elapsed / 1000);
            
            // Проверяем isDataReady()
            bool ready = false;
            if (i2c_mutex != NULL) {
                if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    ready = co2.isDataReady();
                    Serial.printf("[SCD40] isDataReady() = %d\n", ready ? 1 : 0);
                    xSemaphoreGive(i2c_mutex);
                }
            }
            
            // Читаем данные
            if (i2c_mutex != NULL) {
                if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    
                    Serial.println("[SCD40] Чтение данных...");
                    uint8_t err = co2.readMeasurement(co2Value, temperature, humidity);
                    xSemaphoreGive(i2c_mutex);

                    if (err == 0)
                    {
                        successCount++;
                        Serial.printf("[SCD40] ✓ УСПЕХ! (успешно: %d)\n", successCount);
                        
                        errorCount = 0;
                        lastReadTime = millis();

                        sendJson("scd4x_co2", String(co2Value));
                        sendJson("scd4x_temperature", String(temperature * 100));
                        sendJson("scd4x_humidity", String(humidity * 100));
                        AIR_data.update_scd4x_data(co2Value, temperature, humidity);

                        Serial.printf("CO2: %.0f ppm, Temperature: %.1f °C, Humidity: %.0f %%RH\n", co2Value, temperature, humidity);
                        
                        // После успешного чтения ждём 5 секунд
                        vTaskDelay(pdMS_TO_TICKS(5000));
                    }
                    else
                    {
                        errorCount++;
                        Serial.printf("[SCD40] ✗ Ошибка #%d: %s\n", errorCount, co2.getErrorText(err));

                        // Перезапуск после 3 ошибок
                        if (errorCount >= 3) {
                            Serial.println("[SCD40] Перезапуск измерений...");

                            if (i2c_mutex != NULL) {
                                xSemaphoreTake(i2c_mutex, portMAX_DELAY);
                                
                                Serial.println("[SCD40] stopPeriodicMeasurement()...");
                                co2.stopPeriodicMeasurement();
                                vTaskDelay(pdMS_TO_TICKS(500));
                                
                                Serial.println("[SCD40] startPeriodicMeasurement()...");
                                co2.startPeriodicMeasurement();
                                
                                xSemaphoreGive(i2c_mutex);
                            }
                            
                            errorCount = 0;
                            startTime = millis();
                            lastReadTime = 0;
                            vTaskDelay(pdMS_TO_TICKS(1000));
                        } else {
                            vTaskDelay(pdMS_TO_TICKS(5000));
                        }
                    }
                } else {
                    Serial.println("[SCD40] Таймаут mutex при чтении!");
                    vTaskDelay(pdMS_TO_TICKS(5000));
                }
            }
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void BME_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(500));  // Ждём завершения инициализации всех датчиков
    
    while (true)
    {
        if (i2c_mutex != NULL) {
            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            // BME280 возвращает: температура (°C), давление (Па), влажность (%)
            // Делим на 100 для конвертации в гПа
            AIR_data.update_bme_data(bme.readTemperature(), bme.readPressure() / 100.0F, bme.readHumidity());
            xSemaphoreGive(i2c_mutex);
        }
        sendJson("bme_temperature", String(AIR_data.bme_temperature * 100));
        sendJson("bme_pressure", String(AIR_data.bme_pressure));  // уже в гПа
        sendJson("bme_humidity", String(AIR_data.bme_humidity * 100));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void HTU_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(700));  // Ждём завершения инициализации всех датчиков
    
    float temp = 0, hum = 0;
    
    while (true)
    {
        if (i2c_mutex != NULL) {
            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            temp = htu.readTemperature();
            hum = htu.readHumidity();
            xSemaphoreGive(i2c_mutex);
        }
        AIR_data.update_htu_data(temp, hum);
        sendJson("htu_temperature", String(AIR_data.htu_temperature * 100));
        sendJson("htu_humidity", String(AIR_data.htu_humidity * 100));
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void BH1750_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(900));  // Ждём завершения инициализации всех датчиков
    
    while (true)
    {
        if (i2c_mutex != NULL) {
            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            if (lightMeter.measurementReady())
            {
                float lux = lightMeter.readLightLevel();
                AIR_data.update_bh1750_data(lux);
            }
            xSemaphoreGive(i2c_mutex);
        }
        sendJson("bh1750_lighting", String(AIR_data.bh1750_lighting * 10));
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void CH2O_measurementTaskFunction(void *parameter)
{
    while (true)
    {
        float formaldehyde = analogRead(formaldehyde_Pin) / 496.36;
        AIR_data.update_ch2o_data(formaldehyde);
        sendJson("ch2o_value", String(AIR_data.ch2o_value * 10));
        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}

void PMS_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(6000));
    while (true)
    {
        pms.read();
        AIR_data.update_pms_data(pms.pm01, pms.pm25, pms.pm10);
        sendJson("pms_pm1", String(pms.pm01 * 10));
        sendJson("pms_pm2_5", String(pms.pm25 * 10));
        sendJson("pms_pm10", String(pms.pm10 * 10));
        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}

void MS5611_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(1100));  // Ждём завершения инициализации всех датчиков
    
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        double pressure_pa = 0;  // Давление в Паскалях
        float temperature = 0;
        
        if (i2c_mutex != NULL) {
            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            pressure_pa = ms5611.readPressure();  // Возвращает Па
            temperature = ms5611.readTemperature();
            xSemaphoreGive(i2c_mutex);
        }
        
        double pressure_hpa = pressure_pa / 100.0;  // Конвертируем в гПа

        AIR_data.update_ms5611_data(pressure_hpa, temperature);  // Сохраняем в гПа
        sendJson("ms5611_pressure", String(pressure_hpa));  // гПа
        sendJson("ms5611_temperature", String(temperature * 100));

        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}

void VEML_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(1300));  // Ждём завершения инициализации всех датчиков
    
    while (true)
    {
        if (i2c_mutex != NULL) {
            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            AIR_data.update_veml_data(uv.readUV());
            xSemaphoreGive(i2c_mutex);
        }
        sendJson("veml_uv", String(AIR_data.veml_uv));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}