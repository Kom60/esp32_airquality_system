#include "display.h"

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

void show_init_animation()
{
  uint32_t count = 400;
  while(count)
  {
    uint16_t fg_color = random(0x10000);
    uint16_t bg_color = TFT_BLACK;       // This is the background colour used for smoothing (anti-aliasing)

    uint16_t x = random(tft.width());  // Position of centre of arc
    uint16_t y = random(tft.height());

    uint8_t radius       = random(20, tft.width()/4); // Outer arc radius
    uint8_t thickness    = random(1, radius / 4);     // Thickness
    uint8_t inner_radius = radius - thickness;        // Calculate inner radius (can be 0 for circle segment)

    // 0 degrees is at 6 o'clock position
    // Arcs are drawn clockwise from start_angle to end_angle
    uint16_t start_angle = 0; // Start angle must be in range 0 to 360
    uint16_t end_angle   = 360; // End angle must be in range 0 to 360

    bool arc_end = random(2);           // true = round ends, false = square ends (arc_end parameter can be omitted, ends will then be square)

    tft.drawSmoothArc(x, y, radius, inner_radius, start_angle, end_angle, fg_color, bg_color, arc_end);
    count--;
  }
}

void display_all_data()
{
  // Clear the screen with a nice background
  tft.fillScreen(TFT_BLACK);

  int16_t screen_width = tft.width();   // 320

  // Позиции по X для разных секций
  const int16_t section1_label_x = 5;   // Метки (T:, P:, H: и т.д.)
  const int16_t section1_value_x = 45;  // Значения
  const int16_t section1_unit_x = 110;  // Единицы измерения

  const int16_t section2_label_x = 170;   // Метки правой колонки
  const int16_t section2_value_x = 210;   // Значения правой колонки
  const int16_t section2_unit_x = 280;    // Единицы правой колонки

  // Высота строки для TextSize 2 = 16px + 4px отступ = 20px
  const int16_t row_height = 20;
  int16_t y = 5;  // Стартовая позиция Y
  int16_t y_right = 5;  // Позиция Y для правой колонки

  // ==================== ЗАГОЛОВОК ====================
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("AIR QUALITY MONITOR", section1_label_x, y);
  y += row_height + 10;

  // ==================== OUTDOOR SENSORS (BME280) ====================
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("OUTDOOR (BME280)", section1_label_x, y);
  y += row_height + 5;

  // Температура BME280 (норма: 18-26, предупреждение: 15-30)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("T:", section1_label_x, y);
  Thresholds temp_thresh = {-10, 40, -20, 50};
  tft.setTextColor(getValueColor(AIR_data.bme_temperature, temp_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.bme_temperature, 1, section1_value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("C", section1_unit_x, y);
  y += row_height;

  // Давление BME280 (норма: 980-1040, предупреждение: 960-1060)
  tft.drawString("P:", section1_label_x, y);
  Thresholds press_thresh = {980, 1040, 960, 1060};
  tft.setTextColor(getValueColor(AIR_data.bme_pressure, press_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.bme_pressure, 0, section1_value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("hPa", section1_unit_x, y);
  y += row_height;

  // Влажность BME280 (норма: 20-90, предупреждение: 10-95)
  tft.drawString("H:", section1_label_x, y);
  Thresholds hum_thresh = {20, 90, 10, 95};
  tft.setTextColor(getValueColor(AIR_data.bme_humidity, hum_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.bme_humidity, 0, section1_value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("%", section1_unit_x, y);
  y += row_height;

  // UV индекс (норма: 0-5, предупреждение: 0-8)
  tft.drawString("UV:", section1_label_x, y);
  Thresholds uv_thresh = {0, 5, 0, 8};
  tft.setTextColor(getValueColor(AIR_data.veml_uv, uv_thresh), TFT_BLACK);
  tft.drawNumber(AIR_data.veml_uv, section1_value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("idx", section1_unit_x, y);
  y += row_height + 10;

  // ==================== INDOOR SENSORS (HTU21DF) ====================
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("INDOOR (HTU21DF)", section1_label_x, y);
  y += row_height + 5;

  // Температура HTU (норма: 18-26, предупреждение: 15-30)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("T:", section1_label_x, y);
  tft.setTextColor(getValueColor(AIR_data.htu_temperature, temp_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.htu_temperature, 1, section1_value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("C", section1_unit_x, y);
  y += row_height;

  // Влажность HTU (норма: 30-60, предупреждение: 20-80)
  tft.drawString("H:", section1_label_x, y);
  Thresholds hum_indoor_thresh = {30, 60, 20, 80};
  tft.setTextColor(getValueColor(AIR_data.htu_humidity, hum_indoor_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.htu_humidity, 0, section1_value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("%", section1_unit_x, y);
  y += row_height + 10;

  // ==================== MS5611 ====================
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("MS5611", section1_label_x, y);
  y += row_height + 5;

  // Давление MS5611
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("P:", section1_label_x, y);
  tft.setTextColor(getValueColor(AIR_data.ms5611_pressure, press_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.ms5611_pressure, 0, section1_value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("hPa", section1_unit_x, y);
  y += row_height;

  // Температура MS5611
  if (AIR_data.ms5611_temperature != 0) {
    tft.drawString("T:", section1_label_x, y);
    tft.setTextColor(getValueColor(AIR_data.ms5611_temperature, temp_thresh), TFT_BLACK);
    tft.drawFloat(AIR_data.ms5611_temperature, 1, section1_value_x, y);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("C", section1_unit_x, y);
    y += row_height;
  }
  y += 5;

  // ==================== MICROPHONE ====================
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("NOISE", section1_label_x, y);
  y += row_height + 5;

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("dB:", section1_label_x, y);
  Thresholds noise_thresh = {30, 55, 20, 70};
  tft.setTextColor(getValueColor(AIR_data.microphone_noise, noise_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.microphone_noise, 1, section1_value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("dB", section1_unit_x, y);
  y += row_height + 10;

  // ==================== AIR QUALITY (ПРАВАЯ КОЛОНКА) ====================
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("AIR QUALITY", section2_label_x, y_right);
  y_right += row_height + 5;

  // PM1.0 (норма: 0-35, предупреждение: 0-50)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("PM1.0:", section2_label_x, y_right);
  Thresholds pm1_thresh = {0, 35, 0, 50};
  tft.setTextColor(getValueColor(AIR_data.pms_pm1, pm1_thresh), TFT_BLACK);
  tft.drawNumber(AIR_data.pms_pm1, section2_value_x, y_right);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ug/m3", section2_unit_x, y_right + 4);
  tft.setTextSize(2);
  y_right += row_height;

  // PM2.5 (норма: 0-25, предупреждение: 0-50)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("PM2.5:", section2_label_x, y_right);
  Thresholds pm25_thresh = {0, 25, 0, 50};
  tft.setTextColor(getValueColor(AIR_data.pms_pm2_5, pm25_thresh), TFT_BLACK);
  tft.drawNumber(AIR_data.pms_pm2_5, section2_value_x, y_right);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ug/m3", section2_unit_x, y_right + 4);
  tft.setTextSize(2);
  y_right += row_height;

  // PM10 (норма: 0-50, предупреждение: 0-150)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("PM10:", section2_label_x, y_right);
  Thresholds pm10_thresh = {0, 50, 0, 150};
  tft.setTextColor(getValueColor(AIR_data.pms_pm10, pm10_thresh), TFT_BLACK);
  tft.drawNumber(AIR_data.pms_pm10, section2_value_x, y_right);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ug/m3", section2_unit_x, y_right + 4);
  tft.setTextSize(2);
  y_right += row_height;

  // CO2 (норма: 400-1000, предупреждение: 400-1400)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("CO2:", section2_label_x, y_right);
  Thresholds co2_thresh = {400, 1000, 400, 1400};
  tft.setTextColor(getValueColor(AIR_data.scd4x_co2, co2_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.scd4x_co2, 0, section2_value_x, y_right);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ppm", section2_unit_x, y_right + 4);
  tft.setTextSize(2);
  y_right += row_height;

  // CH2O формальдегид (норма: 0-0.08, предупреждение: 0-0.1)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("CH2O:", section2_label_x, y_right);
  Thresholds ch2o_thresh = {0, 0.08, 0, 0.1};
  tft.setTextColor(getValueColor(AIR_data.ch2o_value, ch2o_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.ch2o_value, 3, section2_value_x, y_right);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ppm", section2_unit_x, y_right + 4);
  tft.setTextSize(2);
  y_right += row_height + 5;

  // SCD4X температура и влажность
  if (AIR_data.scd4x_temperature != 0) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("T(SCD):", section2_label_x, y_right);
    tft.setTextColor(getValueColor(AIR_data.scd4x_temperature, temp_thresh), TFT_BLACK);
    tft.drawFloat(AIR_data.scd4x_temperature, 1, section2_value_x, y_right);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("C", section2_unit_x, y_right);
    y_right += row_height;

    tft.drawString("H(SCD):", section2_label_x, y_right);
    tft.setTextColor(getValueColor(AIR_data.scd4x_humidity, hum_indoor_thresh), TFT_BLACK);
    tft.drawFloat(AIR_data.scd4x_humidity, 0, section2_value_x, y_right);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("%", section2_unit_x, y_right);
    y_right += row_height + 5;
  }

  // ==================== ENVIRONMENT (ПРАВАЯ КОЛОНКА) ====================
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("ENVIRONMENT", section2_label_x, y_right);
  y_right += row_height + 5;

  // Освещённость (норма: 300-1000, предупреждение: 100-2000)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Light:", section2_label_x, y_right);
  Thresholds light_thresh = {300, 1000, 100, 2000};
  tft.setTextColor(getValueColor(AIR_data.bh1750_lighting, light_thresh), TFT_BLACK);
  tft.drawFloat(AIR_data.bh1750_lighting, 0, section2_value_x, y_right);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("lux", section2_unit_x, y_right + 4);
  tft.setTextSize(2);
  y_right += row_height + 10;

  // ==================== FOOTER (WiFi + RSSI) ====================
  int16_t footer_y = max(y, y_right) + 5;
  
  // Разделительная линия
  tft.drawLine(0, footer_y, screen_width, footer_y, TFT_DARKGREY);
  footer_y += 5;

  // WiFi IP
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawString("WiFi:", section1_label_x, footer_y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(WiFi.localIP().toString().c_str(), section1_label_x + 40, footer_y);

  // RSSI справа
  int16_t rssi = WiFi.RSSI();
  uint16_t rssi_color = (rssi < -80) ? TFT_RED : ((rssi < -60) ? TFT_YELLOW : TFT_GREEN);
  tft.setTextColor(rssi_color, TFT_BLACK);
  tft.drawString("RSSI:", section2_label_x, footer_y);
  tft.drawNumber(rssi, section2_label_x + 45, footer_y);
  tft.drawString("dBm", section2_label_x + 75, footer_y);
}