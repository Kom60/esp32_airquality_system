#include "display.h"
#include "logger.h"
#include <time.h>

// Пороговые значения для цветовой индикации (как в logic.js)

// Инициализация дисплея и создание задачи обновления
void display_setup() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  // Показываем "Loading..." пока датчики инициализируются
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Loading...", 5, 5);

  // Создаём задачу обновления дисплея (приоритет 1, ядро 1)
  xTaskCreatePinnedToCore(
    DISPLAY_measurementTaskFunction,   // Функция задачи
    "Display Task",        // Имя задачи
    4096,                  // Размер стека
    NULL,                  // Параметры
    1,                     // Приоритет
    &DISPLAY_measurementTask,    // Дескриптор задачи
    1                      // Ядро (1 = APP_CPU)
  );
  LOG_INFO(DISPLAY, "Task created");
}

// Пороговые значения для цветовой индикации (как в logic.js)
struct Thresholds {
  float min;
  float max;
  float warnMin;
  float warnMax;
};

// Цвета для статусов
#define COLOR_OK        TFT_GREEN         // В норме
#define COLOR_WARNING   TFT_YELLOW        // Предупреждение
#define COLOR_CRITICAL  TFT_RED           // Критично

// Определение цвета на основе порогов
uint16_t getValueColor(float value, Thresholds thresh) {
  if (value < thresh.warnMin || value > thresh.warnMax) return COLOR_CRITICAL;
  if (value < thresh.min || value > thresh.max) return COLOR_WARNING;
  return COLOR_OK;
}

// Рисование иконки аккумулятора (без процентов)
void drawBatteryIcon(int16_t x, int16_t y, uint8_t level) {
  // level: 0-100 процентов
  uint16_t color = (level < 20) ? TFT_RED : ((level < 50) ? TFT_YELLOW : TFT_GREEN);

  // Основной корпус батареи
  tft.drawRect(x, y, 36, 16, TFT_WHITE);

  // Положительный вывод справа
  tft.fillRect(x + 36, y + 4, 3, 8, TFT_WHITE);

  // Заполнение уровня
  if (level > 0) {
    uint8_t fill_width = (level * 32) / 100;  // 32px внутри рамки
    tft.fillRect(x + 2, y + 2, fill_width, 12, color);
  }
}

// Отрисовка строки с адаптивным отступом
void drawSensorRow(const char* label, const char* value, const char* unit, int16_t x, int16_t y, uint16_t value_color) {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(label, x, y);

  // Вычисляем ширину метки для адаптивного отступа
  int16_t label_width = tft.textWidth(label);
  int16_t value_x = x + label_width + 8;  // 8px отступ после метки

  tft.setTextColor(value_color, TFT_BLACK);
  tft.drawString(value, value_x, y);

  // Единицы измерения
  int16_t value_width = tft.textWidth(value);
  int16_t unit_x = value_x + value_width + 4;  // 4px отступ после значения

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(unit, unit_x, y);
}

// Отрисовка строки с float значением и адаптивным отступом
void drawSensorRowFloat(const char* label, float value, int decimals, const char* unit, int16_t x, int16_t y, uint16_t value_color) {
  char buf[16];
  dtostrf(value, 1, decimals, buf);
  drawSensorRow(label, buf, unit, x, y, value_color);
}

// Отрисовка строки с заглушкой "---"
void drawSensorRowPlaceholder(const char* label, const char* unit, int16_t x, int16_t y) {
  drawSensorRow(label, "---", unit, x, y, TFT_DARKGREY);
}

// Отрисовка строки с int значением и адаптивным отступом
void drawSensorRowInt(const char* label, int value, const char* unit, int16_t x, int16_t y, uint16_t value_color) {
  char buf[16];
  itoa(value, buf, 10);
  drawSensorRow(label, buf, unit, x, y, value_color);
}

void display_all_data()
{
  // Clear the screen with a nice background
  tft.fillScreen(TFT_BLACK);

  int16_t screen_width = tft.width();   // 320

  // Позиции по X
  const int16_t label_x = 5;      // Позиция меток

  // Высота строки для TextSize 2 = 16px + 2px отступ = 18px
  const int16_t row_height = 18;
  int16_t y = 25;  // Стартовая позиция Y (после статус-бара)

  // ==================== STATUS BAR ====================
  // Разделительная линия снизу
  tft.drawLine(0, 22, screen_width, 22, TFT_DARKGREY);

  // Время слева (ЧЧ:ММ) - заглушка
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("12:00", 5, 2);

  // Аккумулятор справа (без процентов, заглушка 75%)
  drawBatteryIcon(screen_width - 45, 3, 75);

  // ==================== OUTDOOR (BME) ====================
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("OUTDOOR (BME):", label_x, y);
  y += row_height + 2;

  // Температура (скользящее среднее)
  Thresholds temp_thresh = {-10, 40, -20, 50};
  float avg_bme_temp = meteo_buffer.get_avg_bme_temperature();
  if (is_valid_float(avg_bme_temp)) {
    drawSensorRowFloat("Temperature:", avg_bme_temp, 1, "C", label_x, y,
                       getValueColor(avg_bme_temp, temp_thresh));
  } else {
    drawSensorRowPlaceholder("Temperature:", "C", label_x, y);
  }
  y += row_height;

  // Давление (скользящее среднее)
  Thresholds press_thresh = {980, 1040, 960, 1060};
  float avg_bme_press = meteo_buffer.get_avg_bme_pressure();
  if (is_valid_float(avg_bme_press)) {
    drawSensorRowFloat("Pressure:", avg_bme_press, 0, "hPa", label_x, y,
                       getValueColor(avg_bme_press, press_thresh));
  } else {
    drawSensorRowPlaceholder("Pressure:", "hPa", label_x, y);
  }
  y += row_height;

  // Влажность (скользящее среднее)
  Thresholds hum_thresh = {20, 90, 10, 95};
  float avg_bme_hum = meteo_buffer.get_avg_bme_humidity();
  if (is_valid_float(avg_bme_hum)) {
    drawSensorRowFloat("Humidity:", avg_bme_hum, 0, "%", label_x, y,
                       getValueColor(avg_bme_hum, hum_thresh));
  } else {
    drawSensorRowPlaceholder("Humidity:", "%", label_x, y);
  }
  y += row_height + 4;

  // ==================== INDOOR (HTU) ====================
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("INDOOR (HTU):", label_x, y);
  y += row_height + 2;

  // Температура (скользящее среднее)
  float avg_htu_temp = meteo_buffer.get_avg_htu_temperature();
  if (is_valid_float(avg_htu_temp)) {
    drawSensorRowFloat("Temperature:", avg_htu_temp, 1, "C", label_x, y,
                       getValueColor(avg_htu_temp, temp_thresh));
  } else {
    drawSensorRowPlaceholder("Temperature:", "C", label_x, y);
  }
  y += row_height;

  // Влажность (скользящее среднее)
  Thresholds hum_indoor_thresh = {30, 60, 20, 80};
  float avg_htu_hum = meteo_buffer.get_avg_htu_humidity();
  if (is_valid_float(avg_htu_hum)) {
    drawSensorRowFloat("Humidity:", avg_htu_hum, 0, "%", label_x, y,
                       getValueColor(avg_htu_hum, hum_indoor_thresh));
  } else {
    drawSensorRowPlaceholder("Humidity:", "%", label_x, y);
  }
  y += row_height + 4;

  // ==================== MS5611 ====================
  // Давление (скользящее среднее)
  float avg_ms_press = meteo_buffer.get_avg_ms5611_pressure();
  if (is_valid_float(avg_ms_press)) {
    drawSensorRowFloat("Pressure(MS):", avg_ms_press, 0, "hPa", label_x, y,
                       getValueColor(avg_ms_press, press_thresh));
  } else {
    drawSensorRowPlaceholder("Pressure(MS):", "hPa", label_x, y);
  }
  y += row_height;

  // Температура (скользящее среднее)
  float avg_ms_temp = meteo_buffer.get_avg_ms5611_temperature();
  if (is_valid_float(avg_ms_temp) && avg_ms_temp != 0) {
    drawSensorRowFloat("Temperature(MS):", avg_ms_temp, 1, "C", label_x, y,
                       getValueColor(avg_ms_temp, temp_thresh));
    y += row_height;
  }

  // ==================== SCD4X ====================
  // CO2 (скользящее среднее)
  Thresholds co2_thresh = {400, 1000, 400, 1400};
  float avg_scd_co2 = meteo_buffer.get_avg_scd4x_co2();
  if (is_valid_float(avg_scd_co2) && avg_scd_co2 > 0 && avg_scd_co2 <= 5000) {
    drawSensorRowFloat("CO2:", avg_scd_co2, 0, "ppm", label_x, y,
                       getValueColor(avg_scd_co2, co2_thresh));
  } else {
    drawSensorRowPlaceholder("CO2:", "ppm", label_x, y);
  }
  y += row_height;

  // Температура SCD4X (скользящее среднее)
  float avg_scd_temp = meteo_buffer.get_avg_scd4x_temperature();
  if (is_valid_float(avg_scd_temp)) {
    drawSensorRowFloat("Temperature(SCD):", avg_scd_temp, 1, "C", label_x, y,
                       getValueColor(avg_scd_temp, temp_thresh));
  } else {
    drawSensorRowPlaceholder("Temperature(SCD):", "C", label_x, y);
  }
  y += row_height;

  // Влажность SCD4X (скользящее среднее)
  float avg_scd_hum = meteo_buffer.get_avg_scd4x_humidity();
  if (is_valid_float(avg_scd_hum)) {
    drawSensorRowFloat("Humidity(SCD):", avg_scd_hum, 0, "%", label_x, y,
                       getValueColor(avg_scd_hum, hum_indoor_thresh));
  } else {
    drawSensorRowPlaceholder("Humidity(SCD):", "%", label_x, y);
  }
  y += row_height + 4;

  // ==================== AIR QUALITY ====================
  // PM1.0 (скользящее среднее)
  Thresholds pm1_thresh = {0, 35, 0, 50};
  unsigned int avg_pm1 = meteo_buffer.get_avg_pms_pm1();
  if (avg_pm1 >= 0 && avg_pm1 <= 500) {
    drawSensorRowInt("PM1.0:", (int)avg_pm1, "ug/m3", label_x, y,
                     getValueColor((float)avg_pm1, pm1_thresh));
  } else {
    drawSensorRowPlaceholder("PM1.0:", "ug/m3", label_x, y);
  }
  y += row_height;

  // PM2.5 (скользящее среднее)
  Thresholds pm25_thresh = {0, 25, 0, 50};
  unsigned int avg_pm25 = meteo_buffer.get_avg_pms_pm2_5();
  if (avg_pm25 >= 0 && avg_pm25 <= 500) {
    drawSensorRowInt("PM2.5:", (int)avg_pm25, "ug/m3", label_x, y,
                     getValueColor((float)avg_pm25, pm25_thresh));
  } else {
    drawSensorRowPlaceholder("PM2.5:", "ug/m3", label_x, y);
  }
  y += row_height;

  // PM10 (скользящее среднее)
  Thresholds pm10_thresh = {0, 50, 0, 150};
  unsigned int avg_pm10 = meteo_buffer.get_avg_pms_pm10();
  if (avg_pm10 >= 0 && avg_pm10 <= 500) {
    drawSensorRowInt("PM10:", (int)avg_pm10, "ug/m3", label_x, y,
                     getValueColor((float)avg_pm10, pm10_thresh));
  } else {
    drawSensorRowPlaceholder("PM10:", "ug/m3", label_x, y);
  }
  y += row_height;

  // CH2O (скользящее среднее)
  Thresholds ch2o_thresh = {0, 0.08, 0, 0.1};
  float avg_ch2o = meteo_buffer.get_avg_ch2o_value();
  if (is_valid_float(avg_ch2o)) {
    drawSensorRowFloat("CH2O:", avg_ch2o, 3, "mg/m3", label_x, y,
                       getValueColor(avg_ch2o, ch2o_thresh));
  } else {
    drawSensorRowPlaceholder("CH2O:", "mg/m3", label_x, y);
  }
  y += row_height + 4;

  // ==================== ENVIRONMENT ====================
  // Освещённость (EMA - экспоненциальное скользящее среднее, alpha=0.3)
  Thresholds light_thresh = {300, 1000, 100, 2000};
  float ema_light = meteo_buffer.get_ema_bh1750_lighting();
  if (is_valid_float(ema_light)) {
    drawSensorRowFloat("Lighting:", ema_light, 0, "lux", label_x, y,
                       getValueColor(ema_light, light_thresh));
  } else {
    drawSensorRowPlaceholder("Lighting:", "lux", label_x, y);
  }
  y += row_height;

  // UV индекс (EMA - экспоненциальное скользящее среднее, alpha=0.3)
  Thresholds uv_thresh = {0, 5, 0, 8};
  float ema_uv = meteo_buffer.get_ema_veml_uv();
  if (ema_uv >= 0) {
    drawSensorRowInt("UV:", (int)ema_uv, "idx", label_x, y,
                     getValueColor(ema_uv, uv_thresh));
  } else {
    drawSensorRowPlaceholder("UV:", "idx", label_x, y);
  }
  y += row_height;

  // Шум (EMA - экспоненциальное скользящее среднее, alpha=0.3)
  Thresholds noise_thresh = {30, 55, 20, 70};
  float ema_noise = meteo_buffer.get_ema_microphone_noise();
  if (is_valid_float(ema_noise)) {
    drawSensorRowFloat("Noise:", ema_noise, 0, "dB", label_x, y,
                       getValueColor(ema_noise, noise_thresh));
  } else {
    drawSensorRowPlaceholder("Noise:", "dB", label_x, y);
  }
  y += row_height + 8;

  // ==================== SYSTEM INFO ====================
  // Разделительная линия
  tft.drawLine(0, y, screen_width, y, TFT_DARKGREY);
  y += 5;

  tft.setTextSize(1);

  // WiFi IP
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("WiFi IP:", label_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(WiFi.localIP().toString().c_str(), label_x + 55, y);
  y += 12;

  // RSSI
  int16_t rssi = WiFi.RSSI();
  uint16_t rssi_color = (rssi < -80) ? TFT_RED : ((rssi < -60) ? TFT_YELLOW : TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("RSSI:", label_x, y);
  tft.setTextColor(rssi_color, TFT_BLACK);
  char rssi_str[16];
  sprintf(rssi_str, "%d dBm", rssi);
  tft.drawString(rssi_str, label_x + 40, y);
  y += 12;

  // CPU температура
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("CPU Temp:", label_x, y);
  tft.setTextColor(getValueColor(temperatureRead(), temp_thresh), TFT_BLACK);
  char cpu_temp_str[16];
  sprintf(cpu_temp_str, "%.1f C", temperatureRead());
  tft.drawString(cpu_temp_str, label_x + 70, y);
  y += 12;

  // Free heap
  uint32_t free_heap = ESP.getFreeHeap();
  uint16_t heap_color = (free_heap < 100000) ? TFT_RED : ((free_heap < 200000) ? TFT_YELLOW : TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Free Heap:", label_x, y);
  tft.setTextColor(heap_color, TFT_BLACK);
  char heap_str[16];
  sprintf(heap_str, "%d KB", (int)(free_heap / 1024));
  tft.drawString(heap_str, label_x + 70, y);
  y += 12;

  // ==================== INA226 POWER ====================
  // Напряжение (скользящее среднее)
  float avg_voltage = meteo_buffer.get_avg_ina226_voltage();
  if (is_valid_float(avg_voltage)) {
    uint16_t voltage_color = (avg_voltage < 2.8 || avg_voltage > 3.8) ? TFT_RED :
                             (avg_voltage < 3.0 || avg_voltage > 3.6) ? TFT_YELLOW : TFT_GREEN;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("INA226 V:", label_x, y);
    tft.setTextColor(voltage_color, TFT_BLACK);
    char voltage_str[16];
    sprintf(voltage_str, "%.3f V", avg_voltage);
    tft.drawString(voltage_str, label_x + 65, y);
  } else {
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("INA226 V: ---", label_x, y);
  }
  y += 12;

  // Ток (скользящее среднее)
  float avg_current = meteo_buffer.get_avg_ina226_current();
  if (is_valid_float(avg_current)) {
    uint16_t current_color = (avg_current > 2.0) ? TFT_RED : ((avg_current > 1.5) ? TFT_YELLOW : TFT_GREEN);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("INA226 A:", label_x, y);
    tft.setTextColor(current_color, TFT_BLACK);
    char current_str[16];
    sprintf(current_str, "%.3f A", avg_current);
    tft.drawString(current_str, label_x + 65, y);
  } else {
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("INA226 A: ---", label_x, y);
  }
  y += 12;

  // Мощность (скользящее среднее)
  float avg_power = meteo_buffer.get_avg_ina226_power();
  if (is_valid_float(avg_power)) {
    uint16_t power_color = (avg_power > 5.0) ? TFT_RED : ((avg_power > 3.0) ? TFT_YELLOW : TFT_GREEN);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("INA226 W:", label_x, y);
    tft.setTextColor(power_color, TFT_BLACK);
    char power_str[16];
    sprintf(power_str, "%.3f W", avg_power);
    tft.drawString(power_str, label_x + 65, y);
  } else {
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("INA226 W: ---", label_x, y);
  }
}
