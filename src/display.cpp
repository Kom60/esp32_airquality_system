#include "display.h"
#include <time.h>

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

  // Температура
  Thresholds temp_thresh = {-10, 40, -20, 50};
  drawSensorRowFloat("Temperature:", AIR_data.bme_temperature, 1, "C", label_x, y, 
                     getValueColor(AIR_data.bme_temperature, temp_thresh));
  y += row_height;

  // Давление
  Thresholds press_thresh = {980, 1040, 960, 1060};
  drawSensorRowFloat("Pressure:", AIR_data.bme_pressure, 0, "GPa", label_x, y,
                     getValueColor(AIR_data.bme_pressure, press_thresh));
  y += row_height;

  // Влажность
  Thresholds hum_thresh = {20, 90, 10, 95};
  drawSensorRowFloat("Humidity:", AIR_data.bme_humidity, 0, "%", label_x, y,
                     getValueColor(AIR_data.bme_humidity, hum_thresh));
  y += row_height + 4;

  // ==================== INDOOR (HTU) ====================
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("INDOOR (HTU):", label_x, y);
  y += row_height + 2;

  // Температура
  drawSensorRowFloat("Temperature:", AIR_data.htu_temperature, 1, "C", label_x, y,
                     getValueColor(AIR_data.htu_temperature, temp_thresh));
  y += row_height;

  // Влажность
  Thresholds hum_indoor_thresh = {30, 60, 20, 80};
  drawSensorRowFloat("Humidity:", AIR_data.htu_humidity, 0, "%", label_x, y,
                     getValueColor(AIR_data.htu_humidity, hum_indoor_thresh));
  y += row_height + 4;

  // ==================== MS5611 ====================
  // Давление
  drawSensorRowFloat("Pressure(MS):", AIR_data.ms5611_pressure, 0, "GPa", label_x, y,
                     getValueColor(AIR_data.ms5611_pressure, press_thresh));
  y += row_height;
  
  // Температура (если доступна)
  if (AIR_data.ms5611_temperature != 0) {
    drawSensorRowFloat("Temperature(MS):", AIR_data.ms5611_temperature, 1, "C", label_x, y,
                       getValueColor(AIR_data.ms5611_temperature, temp_thresh));
    y += row_height;
  }

  // ==================== SCD4X ====================
  // CO2
  Thresholds co2_thresh = {400, 1000, 400, 1400};
  drawSensorRowFloat("CO2:", AIR_data.scd4x_co2, 0, "ppm", label_x, y,
                     getValueColor(AIR_data.scd4x_co2, co2_thresh));
  y += row_height;

  // Температура SCD4X
  drawSensorRowFloat("Temperature(SCD):", AIR_data.scd4x_temperature, 1, "C", label_x, y,
                     getValueColor(AIR_data.scd4x_temperature, temp_thresh));
  y += row_height;

  // Влажность SCD4X
  drawSensorRowFloat("Humidity(SCD):", AIR_data.scd4x_humidity, 0, "%", label_x, y,
                     getValueColor(AIR_data.scd4x_humidity, hum_indoor_thresh));
  y += row_height + 4;

  // ==================== AIR QUALITY ====================
  // PM1.0
  Thresholds pm1_thresh = {0, 35, 0, 50};
  drawSensorRowInt("PM1.0:", (int)AIR_data.pms_pm1, "ug/m3", label_x, y,
                   getValueColor(AIR_data.pms_pm1, pm1_thresh));
  y += row_height;

  // PM2.5
  Thresholds pm25_thresh = {0, 25, 0, 50};
  drawSensorRowInt("PM2.5:", (int)AIR_data.pms_pm2_5, "ug/m3", label_x, y,
                   getValueColor(AIR_data.pms_pm2_5, pm25_thresh));
  y += row_height;

  // PM10
  Thresholds pm10_thresh = {0, 50, 0, 150};
  drawSensorRowInt("PM10:", (int)AIR_data.pms_pm10, "ug/m3", label_x, y,
                   getValueColor(AIR_data.pms_pm10, pm10_thresh));
  y += row_height;

  // CH2O
  Thresholds ch2o_thresh = {0, 0.08, 0, 0.1};
  drawSensorRowFloat("CH2O:", AIR_data.ch2o_value, 3, "ppm", label_x, y,
                     getValueColor(AIR_data.ch2o_value, ch2o_thresh));
  y += row_height + 4;

  // ==================== ENVIRONMENT ====================
  // Освещённость
  Thresholds light_thresh = {300, 1000, 100, 2000};
  drawSensorRowFloat("Lighting:", AIR_data.bh1750_lighting, 0, "lux", label_x, y,
                     getValueColor(AIR_data.bh1750_lighting, light_thresh));
  y += row_height;
  
  // UV индекс
  Thresholds uv_thresh = {0, 5, 0, 8};
  drawSensorRowInt("UV:", (int)AIR_data.veml_uv, "idx", label_x, y,
                   getValueColor(AIR_data.veml_uv, uv_thresh));
  y += row_height;
  
  // Шум
  Thresholds noise_thresh = {30, 55, 20, 70};
  drawSensorRowFloat("Noise:", AIR_data.microphone_noise, 0, "dB", label_x, y,
                     getValueColor(AIR_data.microphone_noise, noise_thresh));
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
}