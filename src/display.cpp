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
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  
  // Outdoor section (BME280)
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("OUTDOOR SENSORS (BME280):", 10, 5);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Temp:", 15, 20);
  tft.drawFloat(AIR_data.bme_temperature, 1, 60, 20);
  tft.drawString("C", 100, 20);
  
  tft.drawString("Press:", 15, 35);
  tft.drawFloat(AIR_data.bme_pressure / 100.0, 1, 60, 35); // Convert to hPa
  tft.drawString("hPa", 100, 35);
  
  tft.drawString("Humidity:", 15, 50);
  tft.drawFloat(AIR_data.bme_humidity, 1, 80, 50);
  tft.drawString("%", 115, 50);
  
  // VEML6070 UV sensor
  tft.drawString("UV:", 15, 65);
  tft.drawNumber(AIR_data.veml_uv, 60, 65);
  tft.drawString("index", 90, 65);
  
  // Indoor section (HTU21DF)
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("INDOOR SENSORS (HTU21DF):", 10, 80);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Temp:", 15, 95);
  tft.drawFloat(AIR_data.htu_temperature, 1, 60, 95);
  tft.drawString("C", 100, 95);
  
  tft.drawString("Humidity:", 15, 110);
  tft.drawFloat(AIR_data.htu_humidity, 1, 80, 110);
  tft.drawString("%", 115, 110);
  
  // MS5611 pressure sensor
  tft.drawString("Press:", 15, 125);
  tft.drawFloat(AIR_data.ms5611_pressure / 100.0, 1, 60, 125); // Convert to hPa
  tft.drawString("hPa", 100, 125);
  
  // Temperature from MS5611 (if available)
  if (AIR_data.ms5611_temperature != 0) {
    tft.drawString("Temp (MS):", 15, 140);
    tft.drawFloat(AIR_data.ms5611_temperature, 1, 80, 140);
    tft.drawString("C", 120, 140);
  }
  
  // Microphone noise
  tft.drawString("Noise:", 15, 155);
  tft.drawFloat(AIR_data.microphone_noise, 1, 60, 155);
  tft.drawString("dB", 90, 155);
  
  // Air quality section (PMS, CO2, CH2O)
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("AIR QUALITY:", 10, 170);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("PM1.0:", 15, 185);
  tft.drawNumber(AIR_data.pms_pm1, 60, 185);
  tft.drawString("ug/m3", 90, 185);
  
  tft.drawString("PM2.5:", 15, 200);
  tft.drawNumber(AIR_data.pms_pm2_5, 60, 200);
  tft.drawString("ug/m3", 90, 200);
  
  tft.drawString("PM10:", 15, 215);
  tft.drawNumber(AIR_data.pms_pm10, 60, 215);
  tft.drawString("ug/m3", 90, 215);
  
  // SCD4X CO2 sensor data
  tft.drawString("CO2:", 15, 230);
  tft.drawFloat(AIR_data.scd4x_co2, 0, 50, 230);
  tft.drawString("ppm", 80, 230);
  
  // Add SCD4X temperature and humidity if available
  if (AIR_data.scd4x_temperature != 0) {
    tft.drawString("Temp (SCD):", 130, 230);
    tft.drawFloat(AIR_data.scd4x_temperature, 1, 190, 230);
    tft.drawString("C", 220, 230);
  }
  
  // Environmental section (Lighting, CH2O)
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString("ENVIRONMENT:", 130, 80);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Lighting:", 135, 95);
  tft.drawFloat(AIR_data.bh1750_lighting, 1, 190, 95);
  tft.drawString("lux", 220, 95);
  
  tft.drawString("CH2O:", 135, 110);
  tft.drawFloat(AIR_data.ch2o_value, 2, 190, 110);
  tft.drawString("ppm", 220, 110);
}