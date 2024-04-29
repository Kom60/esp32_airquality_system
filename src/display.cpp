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
    //i=false;
    count--;
    }
}

void display_bme()
{
   tft.setTextSize(1);
    tft.drawString("OUTDOOR AIR:",10,15);
    tft.drawString("Temperature",15,30);
    tft.drawFloat(AIR_data.Outdoor_temp,1,90,30);
    tft.drawString("Celsium",120,30);
    tft.drawString("Pressure",15,45);
    tft.drawFloat(AIR_data.Outdoor_pressure,1,75,45);
    tft.drawString("GPascal",115,45);
    tft.drawString("Humidity",15,60);
    tft.drawFloat(AIR_data.Outdoor_Humidity,1,75,60);
    tft.drawString("%",110,60);
}
void display_indoor()
{
  tft.drawString("INDOOR AIR:",10,75);
  tft.drawString("Temperature",15,90);
  tft.drawFloat(AIR_data.Indoor_temp,1,90,90);
  tft.drawString("Celsium",120,90);
  tft.drawString("Pressure",15,105);
  tft.drawFloat(AIR_data.Indoor_pressure/100.0,1,75,105);
  tft.drawString("GPascal",115,105);
  tft.drawString("Humidity",15,120);
  tft.drawFloat(AIR_data.Indoor_humidity,1,75,120);
  tft.drawString("%",110,120);
  tft.drawString("PM1.0",15,135);
  tft.drawFloat(AIR_data.Indoor_PM1,1,60,135);
  tft.drawString("[ug/m3]",90,135);
  tft.drawString("PM2.5",15,150);
  tft.drawFloat(AIR_data.Indoor_PM2,1,60,150);
  tft.drawString("[ug/m3]",90,150);
  tft.drawString("PM10",15,165);
  tft.drawFloat(AIR_data.Indoor_PM10,1,60,165);
  tft.drawString("[ug/m3]",90,165);
  tft.drawString("Radiation",15,180);
  tft.drawString("Lighting",15,195);
  tft.drawFloat(AIR_data.Lighting,1,75,195);
  tft.drawString("lux",110,195);
  tft.drawString("CO2",15,210);
  tft.drawString("CH2O",15,225);
  tft.drawFloat(AIR_data.CH2O,2,47,225);
  tft.drawString("ppm",75,225);
}