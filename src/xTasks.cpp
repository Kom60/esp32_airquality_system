#include "headers.h"
#include "settings.h"
#include "microphone.h"
#include "ina226.h"
#include "logger.h"

// Переменные
QueueHandle_t samples_queue;
TaskHandle_t INMP441_measurementTask;
TaskHandle_t webSocketTaskHandle = NULL;
TaskHandle_t sendDataTaskHandle = NULL;

// External sensor objects
extern INA226_Sensor ina226;

// Static buffer for block of samples
float samples[SAMPLES_SHORT] __attribute__((aligned(4)));

// Функция проверки валидности числа
bool is_valid_float(float value) {
    return !isnan(value) && !isinf(value) && value < 1e6 && value > -1e6;
}

TaskHandle_t CO2_measurementTask, BME_measurementTask,
    HTU_measurementTask, BH1750_measurementTask, CH2O_measurementTask,
    PMS_measurementTask, MS5611_measurementTask, VEML_measurementTask,
    DISPLAY_measurementTask, INA226_measurementTask;

// Проверка готовности данных от всех датчиков (используем скользящее среднее)
bool are_all_sensors_ready() {
  // Проверяем основные датчики через скользящее среднее
  if (!is_valid_float(meteo_buffer.get_avg_bme_temperature())) return false;
  if (!is_valid_float(meteo_buffer.get_avg_bme_pressure())) return false;
  if (!is_valid_float(meteo_buffer.get_avg_bme_humidity())) return false;

  if (!is_valid_float(meteo_buffer.get_avg_htu_temperature())) return false;
  if (!is_valid_float(meteo_buffer.get_avg_htu_humidity())) return false;

  if (!is_valid_float(meteo_buffer.get_avg_ms5611_pressure())) return false;

  float avg_co2 = meteo_buffer.get_avg_scd4x_co2();
  if (avg_co2 <= 0 || avg_co2 > 5000) return false;

  if (meteo_buffer.get_avg_pms_pm1() > 500) return false;
  if (meteo_buffer.get_avg_pms_pm2_5() > 500) return false;
  if (meteo_buffer.get_avg_pms_pm10() > 500) return false;

  if (!is_valid_float(meteo_buffer.get_avg_bh1750_lighting())) return false;
  if (!is_valid_float(meteo_buffer.get_avg_ch2o_value())) return false;
  if (!is_valid_float(meteo_buffer.get_avg_microphone_noise())) return false;

  return true;
}

// Задача обновления дисплея (раз в 20 секунд)
void DISPLAY_measurementTaskFunction(void *parameter)
{
    const unsigned long DISPLAY_INTERVAL = 20000;  // 20 секунд в миллисекундах
    bool display_initialized = false;

    vTaskDelay(pdMS_TO_TICKS(500));  // Ждём 0.5 сек для первичной инициализации

    LOG_INFO(DISPLAY, "Task started");

    for (;;) {
        display_all_data();
        if (!display_initialized) {
            display_initialized = true;
            LOG_INFO(DISPLAY, "Initial data displayed");
        } else {
            LOG_DEBUG(DISPLAY, "Updated");
        }

        // Ждём следующий цикл (20 секунд)
        vTaskDelay(pdMS_TO_TICKS(DISPLAY_INTERVAL));
    }
}

// Инициализация микрофона и создание задач
void microphone_init(void)
{
    // Create FreeRTOS queue
    samples_queue = xQueueCreate(8, sizeof(sum_queue_t));

    // Create the I2S reader FreeRTOS task
    xTaskCreate(mic_i2s_reader_task, "Mic I2S Reader", I2S_TASK_STACK, NULL, I2S_TASK_PRI, NULL);

    // Create INMP441 measurement task
    xTaskCreatePinnedToCore(INMP441_measurementTaskFunction, "INMP441MeasurementTask", 2048, NULL, 1, &INMP441_measurementTask, 0);

    LOG_INFO(MICROPHONE, "Tasks created");
}

void CO2_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(1500));  // Ждём завершения инициализации других датчиков

    LOG_INFO(SCD4X, "=== Инициализация SCD40 ===");

    // Инициализация SCD40 с защитой I2C mutex
    if (i2c_mutex != NULL) {
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);

        Wire.begin();
        Wire.setClock(400000);  // 400 kHz для быстрой инициализации

        // Проверяем наличие датчика на шине
        Wire.beginTransmission(0x62);
        uint8_t i2cErr = Wire.endTransmission();

        xSemaphoreGive(i2c_mutex);

        // Вывод отладки ЗА пределами mutex
        if (i2cErr == 0) {
            LOG_INFO(SCD4X, "Датчик найден на адресе 0x62");
        } else {
            LOG_ERROR_FMT(SCD4X, "ОШИБКА: Датчик не найден на 0x62 (error=%d)", i2cErr);
            LOG_ERROR(SCD4X, "Проверьте подключение и питание!");
        }

        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        uint8_t err = co2.begin();
        xSemaphoreGive(i2c_mutex);

        // Вывод отладки ЗА пределами mutex
        if (err != 0) {
            LOG_ERROR_FMT(SCD4X, "Ошибка co2.begin(): %s", co2.getErrorText(err));
        } else {
            LOG_INFO(SCD4X, "co2.begin() успешно");
        }

        // Принудительно останавливаем измерения (если были запущены из EEPROM)
        LOG_INFO(SCD4X, "Остановка возможных измерений...");

        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        err = co2.stopPeriodicMeasurement();
        xSemaphoreGive(i2c_mutex);

        LOG_DEBUG_FMT(SCD4X, "stopPeriodicMeasurement: %s", co2.getErrorText(err));

        // Ждём пока датчик остановится (по спецификации 500мс)
        vTaskDelay(pdMS_TO_TICKS(500));
    } else {
        LOG_ERROR(SCD4X, "ОШИБКА: i2c_mutex не создан!");
    }

    // Ждём ~2 секунды, чтобы BME280 и MS5611 успели сделать первые замеры
    LOG_INFO(SCD4X, "Ожидание данных давления от BME280/MS5611...");
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
            xSemaphoreGive(i2c_mutex);

            // Вывод отладки ЗА пределами mutex
            LOG_INFO_FMT(SCD4X, "Расчётное altitude: %.1f м (давление: %.2f гПа)", altitude, avgPressure);

            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            // Установка altitude в SCD40 (в метрах)
            uint16_t altitudeInt = (uint16_t)altitude;
            uint8_t err = co2.setSensorAltitude(altitudeInt);
            xSemaphoreGive(i2c_mutex);

            if (err == 0) {
                LOG_INFO_FMT(SCD4X, "✓ Установлено altitude: %d м", altitudeInt);
            } else {
                LOG_ERROR_FMT(SCD4X, "✗ Ошибка установки altitude: %s", co2.getErrorText(err));
            }
        } else {
            LOG_WARNING(SCD4X, "⚠ Давление не получено (BME280/MS5611 не готовы)");
        }

        // Теперь запускаем периодические измерения
        xSemaphoreTake(i2c_mutex, portMAX_DELAY);
        uint8_t err = co2.startPeriodicMeasurement();
        xSemaphoreGive(i2c_mutex);

        if (err != 0) {
            LOG_ERROR_FMT(SCD4X, "Ошибка startPeriodicMeasurement(): %s", co2.getErrorText(err));
        } else {
            LOG_INFO(SCD4X, "startPeriodicMeasurement() успешно");
        }
    }

    int errorCount = 0;
    int readAttemptCount = 0;
    int successCount = 0;
    unsigned long startTime = millis();
    unsigned long lastReadTime = 0;

    LOG_INFO(SCD4X, "Начало цикла измерений...");
    LOG_INFO(SCD4X, "Первое измерение займёт ~5 секунд");

    while (true)
    {
        unsigned long elapsed = millis() - startTime;
        unsigned long timeSinceLastRead = millis() - lastReadTime;

        // Пытаемся читать каждые 5 секунд
        if (timeSinceLastRead >= 5000 || lastReadTime == 0)
        {
            readAttemptCount++;
            LOG_DEBUG_FMT(SCD4X, "=== Попытка #%d (elapsed=%lus) ===", readAttemptCount, elapsed / 1000);

            // Проверяем isDataReady()
            bool ready = false;
            if (i2c_mutex != NULL) {
                if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    ready = co2.isDataReady();
                    xSemaphoreGive(i2c_mutex);

                    // Вывод отладки ЗА пределами mutex
                    LOG_DEBUG_FMT(SCD4X, "isDataReady() = %d", ready ? 1 : 0);
                }
            }

            // Читаем данные
            if (i2c_mutex != NULL) {
                if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {

                    LOG_DEBUG(SCD4X, "Чтение данных...");
                    uint8_t err = co2.readMeasurement(co2Value, temperature, humidity);
                    xSemaphoreGive(i2c_mutex);

                    if (err == 0)
                    {
                        successCount++;
                        LOG_INFO_FMT(SCD4X, "✓ УСПЕХ! (успешно: %d)", successCount);

                        errorCount = 0;
                        lastReadTime = millis();

                        sendJson("scd4x_co2", String(co2Value));
                        sendJson("scd4x_temperature", String(temperature));  // Температура в °C (без *100)
                        sendJson("scd4x_humidity", String(humidity));        // Влажность в % (без *100)

                        // Применяем калибровку
                        temperature += settings.temp_offset_scd;

                        AIR_data.update_scd4x_data(co2Value, temperature, humidity);
                        meteo_buffer.push();

                        LOG_INFO_FMT(SCD4X, "CO2: %.0f ppm, Temperature: %.1f C, Humidity: %.0f %%RH", co2Value, temperature, humidity);

                        // После успешного чтения ждём 5 секунд
                        vTaskDelay(pdMS_TO_TICKS(5000));
                    }
                    else
                    {
                        errorCount++;
                        LOG_ERROR_FMT(SCD4X, "✗ Ошибка #%d: %s", errorCount, co2.getErrorText(err));

                        // Перезапуск после 3 ошибок
                        if (errorCount >= 3) {
                            LOG_INFO(SCD4X, "Перезапуск измерений...");

                            if (i2c_mutex != NULL) {
                                xSemaphoreTake(i2c_mutex, portMAX_DELAY);

                                LOG_DEBUG(SCD4X, "stopPeriodicMeasurement()...");
                                co2.stopPeriodicMeasurement();
                                vTaskDelay(pdMS_TO_TICKS(500));

                                LOG_DEBUG(SCD4X, "startPeriodicMeasurement()...");
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
                    LOG_ERROR(SCD4X, "Таймаут mutex при чтении!");
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
            // BME280: время измерения 16ms (typ), 1Hz достаточно
            AIR_data.update_bme_data(bme.readTemperature(), bme.readPressure() / 100.0F, bme.readHumidity());
            meteo_buffer.push();
            xSemaphoreGive(i2c_mutex);
        }

        // Применяем калибровку ЗА пределами mutex
        AIR_data.bme_temperature += settings.temp_offset_bme;
        AIR_data.bme_pressure += settings.press_offset_bme;
        AIR_data.bme_humidity += settings.hum_offset_bme;

        sendJson("bme_temperature", String(meteo_buffer.get_avg_bme_temperature()));
        sendJson("bme_pressure", String(meteo_buffer.get_avg_bme_pressure()));
        sendJson("bme_humidity", String(meteo_buffer.get_avg_bme_humidity()));
        vTaskDelay(pdMS_TO_TICKS(1000));  // BME280: 1Hz (оптимально для баланса шум/скорость)
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
            // HTU21DF: время измерения T=14bit/RH=12bit, ~50ms
            temp = htu.readTemperature();
            hum = htu.readHumidity();
            xSemaphoreGive(i2c_mutex);
        }
        AIR_data.update_htu_data(temp, hum);
        meteo_buffer.push();

        // Применяем калибровку ЗА пределами mutex
        AIR_data.htu_temperature += settings.temp_offset_htu;
        AIR_data.htu_humidity += settings.hum_offset_htu;

        sendJson("htu_temperature", String(meteo_buffer.get_avg_htu_temperature()));
        sendJson("htu_humidity", String(meteo_buffer.get_avg_htu_humidity()));
        vTaskDelay(pdMS_TO_TICKS(2000));  // HTU21DF: 0.5Hz (температура/влажность меняются медленно)
    }
}

void BH1750_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(900));  // Ждём завершения инициализации всех датчиков

    while (true)
    {
        if (i2c_mutex != NULL) {
            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            // BH1750: время измерения 120ms (Continuous H-Resolution Mode)
            if (lightMeter.measurementReady())
            {
                float lux = lightMeter.readLightLevel();
                // Проверяем на валидность и реалистичность перед записью
                if (is_valid_float(lux) && lux >= 0 && lux <= 150000) {
                    AIR_data.update_bh1750_data(lux);
                } else {
                    LOG_WARNING_FMT(BH1750, "Некорректное значение: %.1f", lux);
                }
            }
            xSemaphoreGive(i2c_mutex);
        }
        meteo_buffer.push();
        meteo_buffer.update_ema();  // Обновляем EMA для освещённости
        sendJson("bh1750_lighting", String(meteo_buffer.get_avg_bh1750_lighting()));
        vTaskDelay(pdMS_TO_TICKS(500));  // BH1750: 2Hz (освещённость может меняться быстро)
    }
}

void CH2O_measurementTaskFunction(void *parameter)
{
    while (true)
    {
        // Аналоговый датчик: время отклика ~60 сек, 10 сек достаточно
        float formaldehyde = analogRead(formaldehyde_Pin) / 496.36;
        AIR_data.update_ch2o_data(formaldehyde);
        meteo_buffer.push();
        sendJson("ch2o_value", String(meteo_buffer.get_avg_ch2o_value()));
        vTaskDelay(pdMS_TO_TICKS(10000));  // CH2O: 0.1Hz (медленный электрохимический датчик)
    }
}

void PMS_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(6000));  // PMS: время прогрева и первого измерения
    while (true)
    {
        // PMS5003: время измерения ~1.2 сек, но рекомендуется 2.3 сек интервал
        pms.read();
        AIR_data.update_pms_data(pms.pm01, pms.pm25, pms.pm10);
        meteo_buffer.push();
        sendJson("pms_pm1", String(meteo_buffer.get_avg_pms_pm1()));
        sendJson("pms_pm2_5", String(meteo_buffer.get_avg_pms_pm2_5()));
        sendJson("pms_pm10", String(meteo_buffer.get_avg_pms_pm10()));
        vTaskDelay(pdMS_TO_TICKS(2300));  // PMS5003: мин. 2.3 сек между измерениями
    }
}

void MS5611_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(1100));  // Ждём завершения инициализации всех датчиков

    while (true)
    {
        // MS5611: время измерения 0.5ms (Ultra High Resolution), 1Hz достаточно
        double pressure_pa = 0;  // Давление в Паскалях
        float temperature = 0;

        if (i2c_mutex != NULL) {
            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            pressure_pa = ms5611.readPressure();  // Возвращает Па
            temperature = ms5611.readTemperature();
            xSemaphoreGive(i2c_mutex);
        }

        double pressure_hpa = pressure_pa / 100.0;  // Конвертируем в гПа

        // Применяем калибровку ЗА пределами mutex
        pressure_hpa += settings.press_offset_ms;
        temperature += settings.temp_offset_bme;  // Используем тот же offset что и для BME

        AIR_data.update_ms5611_data(pressure_hpa, temperature);  // Сохраняем в гПа
        meteo_buffer.push();
        sendJson("ms5611_pressure", String(meteo_buffer.get_avg_ms5611_pressure()));
        sendJson("ms5611_temperature", String(meteo_buffer.get_avg_ms5611_temperature()));

        vTaskDelay(pdMS_TO_TICKS(1000));  // MS5611: 1Hz (давление меняется медленно)
    }
}

void VEML_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(1300));  // Ждём завершения инициализации всех датчиков

    while (true)
    {
        if (i2c_mutex != NULL) {
            xSemaphoreTake(i2c_mutex, portMAX_DELAY);
            // VEML6070: время измерения 56ms (typical)
            AIR_data.update_veml_data(uv.readUV());
            xSemaphoreGive(i2c_mutex);
        }
        meteo_buffer.push();
        meteo_buffer.update_ema();  // Обновляем EMA для UV
        sendJson("veml_uv", String(meteo_buffer.get_avg_veml_uv()));
        vTaskDelay(pdMS_TO_TICKS(1000));  // VEML6070: 1Hz (UV индекс меняется медленно)
    }
}

// Задача обработки WebSocket (для links2004/WebSockets требуется периодический вызов loop())
void webSocketTaskFunction(void *parameter)
{
    LOG_INFO_FMT(WEBSOCKET, "Task started on core %d", xPortGetCoreID());

    for (;;) {
        webSocket.loop();  // Обработка подключений и событий WebSocket
        vTaskDelay(pdMS_TO_TICKS(10));  // Небольшая задержка для стабильности
    }
}

// Переменные для задачи отправки данных
static unsigned long send_data_previousMillis = 0;
static unsigned long send_data_loop_start_time = 0;
static unsigned long send_data_loop_total_time = 0;
static unsigned long send_data_loop_count = 0;
static float send_data_cpu_load_percent = 0.0;

// Задача периодической отправки данных датчиков в WebSocket и на ПК
void sendDataTaskFunction(void *parameter)
{
    LOG_INFO_FMT(SEND_DATA, "Task started on core %d", xPortGetCoreID());
    
    for (;;) {
        unsigned long now = millis();
        
        // Получаем текущий интервал из настроек
        int current_interval = settings.update_interval * 1000;
        
        if ((unsigned long)(now - send_data_previousMillis) > (unsigned long)current_interval) {
            send_data_previousMillis = now;
            
            // Отправка данных датчиков в WebSocket (скользящее среднее)
            // BME280 sensor data
            sendJson("bme_temperature", String(meteo_buffer.get_avg_bme_temperature()));
            sendJson("bme_pressure", String(meteo_buffer.get_avg_bme_pressure()));
            sendJson("bme_humidity", String(meteo_buffer.get_avg_bme_humidity()));

            // HTU21DF sensor data
            sendJson("htu_temperature", String(meteo_buffer.get_avg_htu_temperature()));
            sendJson("htu_humidity", String(meteo_buffer.get_avg_htu_humidity()));

            // SCD4X sensor data
            sendJson("scd4x_co2", String((int)meteo_buffer.get_avg_scd4x_co2()));
            sendJson("scd4x_temperature", String(meteo_buffer.get_avg_scd4x_temperature()));
            sendJson("scd4x_humidity", String(meteo_buffer.get_avg_scd4x_humidity()));

            // PMS sensor data
            sendJson("pms_pm1", String(meteo_buffer.get_avg_pms_pm1()));
            sendJson("pms_pm2_5", String(meteo_buffer.get_avg_pms_pm2_5()));
            sendJson("pms_pm10", String(meteo_buffer.get_avg_pms_pm10()));

            // MS5611 sensor data
            sendJson("ms5611_pressure", String(meteo_buffer.get_avg_ms5611_pressure()));
            sendJson("ms5611_temperature", String(meteo_buffer.get_avg_ms5611_temperature()));

            // BH1750 sensor data
            sendJson("bh1750_lighting", String(meteo_buffer.get_avg_bh1750_lighting()));

            // VEML6070 sensor data
            sendJson("veml_uv", String(meteo_buffer.get_avg_veml_uv()));

            // CH2O sensor data
            sendJson("ch2o_value", String(meteo_buffer.get_avg_ch2o_value()));

            // Microphone data (EMA)
            sendJson("microphone_noise", String(meteo_buffer.get_ema_microphone_noise()));

            // INA226 sensor data
            sendJson("ina226_voltage", String(meteo_buffer.get_avg_ina226_voltage()));
            sendJson("ina226_current", String(meteo_buffer.get_avg_ina226_current()));
            sendJson("ina226_power", String(meteo_buffer.get_avg_ina226_power()));

            // Отправка данных о системе
            sendJson("esp32_cpu_freq", String(esp_clk_cpu_freq()));
            sendJson("esp32_cpu_temp", String(temperatureRead()));
            sendJson("esp32_free_heap", String(ESP.getFreeHeap()));
            
            // Расчёт загрузки CPU на основе времени выполнения loop
            send_data_cpu_load_percent = (float)send_data_loop_total_time / (send_data_loop_count * current_interval * 1000) * 100.0;
            if (send_data_cpu_load_percent > 100)
                send_data_cpu_load_percent = 100;
            sendJson("esp32_cpu_load", String(send_data_cpu_load_percent));
            
            // Сброс счётчиков каждые 100 циклов
            if (send_data_loop_count >= 100) {
                send_data_loop_total_time = 0;
                send_data_loop_count = 0;
            }
            
            sendJson("wifi_rssi", String(WiFi.RSSI()));
            
            // Отправка данных на ПК
            send_data_to_pc();
        }
        
        // Измеряем время выполнения цикла (для расчёта CPU load в main loop)
        unsigned long loop_time = micros() - send_data_loop_start_time;
        send_data_loop_total_time += loop_time;
        send_data_loop_count++;
        send_data_loop_start_time = micros();

        vTaskDelay(pdMS_TO_TICKS(10));  // Небольшая задержка для стабильности
    }
}

// Задача измерения INA226 (ток, напряжение, мощность)
void INA226_measurementTaskFunction(void *parameter)
{
    vTaskDelay(pdMS_TO_TICKS(500));  // Ждём инициализации I2C

    LOG_INFO(INA226, "Task started");

    if (!ina226.begin()) {
        LOG_ERROR(INA226, "Initialization failed!");
        vTaskDelete(NULL);
        return;
    }

    for (;;) {
        ina226.read();

        AIR_data.update_ina226_data(
            ina226.bus_voltage,
            ina226.current,
            ina226.power
        );

        LOG_DEBUG_FMT(INA226, "V: %.3fV, I: %.3fA, P: %.3fW",
                      ina226.bus_voltage, ina226.current, ina226.power);

        vTaskDelay(pdMS_TO_TICKS(1000));  // Обновление раз в секунду
    }
}

