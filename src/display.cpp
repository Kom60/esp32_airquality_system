#include "display.h"

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

  // Позиции по X
  const int16_t label_x = 5;      // Позиция меток (T:, P:, H: и т.д.)
  const int16_t value_x = 55;     // Позиция значений
  const int16_t unit_x = 130;     // Позиция единиц измерения

  // Высота строки для TextSize 2 = 16px + 4px отступ = 20px
  const int16_t row_height = 20;
  int16_t y = 5;  // Стартовая позиция Y

  // ==================== OUTDOOR SENSORS (BME280) ====================
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("OUTDOOR (BME280)", label_x, y);
  y += row_height + 5;  // +5 доп. отступ после заголовка

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Температура
  tft.drawString("T:", label_x, y);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawFloat(AIR_data.bme_temperature, 1, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("C", unit_x, y);
  y += row_height;

  // Давление
  tft.drawString("P:", label_x, y);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.drawFloat(AIR_data.bme_pressure, 0, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("hPa", unit_x, y);
  y += row_height;

  // Влажность
  tft.drawString("H:", label_x, y);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawFloat(AIR_data.bme_humidity, 0, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("%", unit_x, y);
  y += row_height;

  // UV индекс
  tft.drawString("UV:", label_x, y);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawNumber(AIR_data.veml_uv, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("index", unit_x, y);
  y += row_height + 5;  // Доп. отступ после секции

  // ==================== INDOOR SENSORS (HTU21DF) ====================
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("INDOOR (HTU21DF)", label_x, y);
  y += row_height + 5;

  // Температура
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("T:", label_x, y);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawFloat(AIR_data.htu_temperature, 1, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("C", unit_x, y);
  y += row_height;

  // Влажность
  tft.drawString("H:", label_x, y);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawFloat(AIR_data.htu_humidity, 0, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("%", unit_x, y);
  y += row_height + 5;

  // ==================== MS5611 ====================
  // Давление MS5611
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("P(MS):", label_x, y);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.drawFloat(AIR_data.ms5611_pressure, 0, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("hPa", unit_x, y);
  y += row_height;

  // Температура MS5611 (если доступна)
  if (AIR_data.ms5611_temperature != 0) {
    tft.drawString("T(MS):", label_x, y);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawFloat(AIR_data.ms5611_temperature, 1, value_x, y);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("C", unit_x, y);
    y += row_height;
  }

  // ==================== MICROPHONE ====================
  tft.drawString("Noise:", label_x, y);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawFloat(AIR_data.microphone_noise, 1, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("dB", unit_x, y);
  y += row_height + 10;  // Больше отступ перед важной секцией

  // ==================== AIR QUALITY ====================
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("AIR QUALITY", label_x, y);
  y += row_height + 5;

  // PM1.0
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("PM1.0:", label_x, y);
  tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
  tft.drawNumber(AIR_data.pms_pm1, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ug/m3", unit_x, y + 4);  // +4 для центровки мелкого шрифта
  tft.setTextSize(2);
  y += row_height;

  // PM2.5
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("PM2.5:", label_x, y);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawNumber(AIR_data.pms_pm2_5, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ug/m3", unit_x, y + 4);
  tft.setTextSize(2);
  y += row_height;

  // PM10
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("PM10:", label_x, y);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawNumber(AIR_data.pms_pm10, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ug/m3", unit_x, y + 4);
  tft.setTextSize(2);
  y += row_height;

  // CO2
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("CO2:", label_x, y);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawFloat(AIR_data.scd4x_co2, 0, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ppm", unit_x, y + 4);
  tft.setTextSize(2);
  y += row_height + 5;

  // SCD4X температура и влажность (если доступны)
  if (AIR_data.scd4x_temperature != 0) {
    // Температура SCD4X
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("T(SCD):", label_x, y);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawFloat(AIR_data.scd4x_temperature, 1, value_x, y);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("C", unit_x, y);
    y += row_height;

    // Влажность SCD4X
    tft.drawString("H(SCD):", label_x, y);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawFloat(AIR_data.scd4x_humidity, 0, value_x, y);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("%", unit_x, y);
    y += row_height + 5;
  }

  // ==================== ENVIRONMENT ====================
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("ENVIRONMENT", label_x, y);
  y += row_height + 5;

  // Освещённость
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Light:", label_x, y);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawFloat(AIR_data.bh1750_lighting, 0, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("lux", unit_x, y + 4);
  tft.setTextSize(2);
  y += row_height;

  // CH2O формальдегид
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("CH2O:", label_x, y);
  tft.setTextColor(TFT_PINK, TFT_BLACK);
  tft.drawFloat(AIR_data.ch2o_value, 3, value_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ppm", unit_x, y + 4);
  tft.setTextSize(2);
  y += row_height + 10;

  // ==================== FOOTER (WiFi + RSSI) ====================
  // Разделительная линия
  tft.drawLine(0, y, screen_width, y, TFT_DARKGREY);
  y += 5;

  // WiFi IP
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawString("WiFi IP:", label_x, y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(WiFi.localIP().toString().c_str(), label_x + 55, y);

  // RSSI справа
  int16_t rssi = WiFi.RSSI();
  uint16_t rssi_color = (rssi < -80) ? TFT_RED : ((rssi < -60) ? TFT_YELLOW : TFT_GREEN);
  tft.setTextColor(rssi_color, TFT_BLACK);
  tft.drawString("RSSI: ", 200, y);
  tft.drawNumber(rssi, 245, y);
  tft.drawString(" dBm", 270, y);
}