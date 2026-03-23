#include "headers.h"

void BME_280_printValues() {
    LOG_DEBUG_FMT(BME, "Temperature = %.2f C, Pressure = %.2f hPa, Altitude = %.0f m, Humidity = %.2f %%",
                  bme.readTemperature(),
                  bme.readPressure() / 100.0F,
                  bme.readAltitude(SEALEVELPRESSURE_HPA),
                  bme.readHumidity());
}

void SCD40_printValues() {
    LOG_DEBUG_FMT(SCD4X, "Co2: %.0f ppm, Temperature: %.1f C, Humidity: %.0f %%",
                  co2Value, temperature, humidity);
}
/*
void display_bme()
{
   tft.setTextSize(1);
    tft.drawString("OUTDOOR AIR:",10,15);
    tft.drawString("Temperature",15,30);
    tft.drawFloat(bme.readTemperature(),1,90,30);
    tft.drawString("Celsium",120,30);
    tft.drawString("Pressure",15,45);
    tft.drawFloat((bme.readPressure() / 100.0F),1,75,45);  // Конвертируем из Па в гПа
    tft.drawString("hPascal",115,45);
    tft.drawString("Humidity",15,60);
    tft.drawFloat(bme.readHumidity(),1,75,60);
    tft.drawString("%",110,60);
}

void display_indoor()
{
  pms.read();
  float lux=0;
  float temp = htu.readTemperature();
  float hum = htu.readHumidity();
  formaldehyde=analogRead(formaldehyde_Pin);
  MS5611.read(); 
  if (lightMeter.measurementReady()) {
    lux = lightMeter.readLightLevel();}
  tft.drawString("INDOOR AIR:",10,75);
  tft.drawString("Temperature",15,90);
  tft.drawFloat(temp,1,90,90);
  tft.drawString("Celsium",120,90);
  tft.drawString("Pressure",15,105);
  tft.drawFloat(MS5611.getPressure(),1,75,105);
  tft.drawString("hPascal",115,105);
  tft.drawString("Humidity",15,120);
  tft.drawFloat(hum,1,75,120);
  tft.drawString("%",110,120);
  tft.drawString("PM1.0",15,135);
  tft.drawFloat(pms.pm01,1,60,135);
  tft.drawString("[ug/m3]",90,135);
  tft.drawString("PM2.5",15,150);
  tft.drawFloat(pms.pm25,1,60,150);
  tft.drawString("[ug/m3]",90,150);
  tft.drawString("PM10",15,165);
  tft.drawFloat(pms.pm10,1,60,165);
  tft.drawString("[ug/m3]",90,165);
  tft.drawString("Radiation",15,180);
  tft.drawString("Lighting",15,195);
  tft.drawFloat(lux,1,75,195);
  tft.drawString("lux",110,195);
  tft.drawString("CO2",15,210);
  tft.drawString("CH2O",15,225);
  tft.drawFloat(formaldehyde/496.36,2,47,225);
  tft.drawString("ppm",75,225);
}
*/

void display_Info()
{
  LOG_DEBUG(SENSORS, "1");
}

void PME5003_printValues(SerialPM pms){
  pms.read();
  if (pms)
  { // successfull read
    // print formatted results
    LOG_DEBUG_FMT(PMS, "PM1.0 %d, PM2.5 %d, PM10 %d [ug/m3]",
                  pms.pm01, pms.pm25, pms.pm10);

    if (pms.has_number_concentration())
      LOG_DEBUG_FMT(PMS, "N0.3 %d, N0.5 %d, N1.0 %d, N2.5 %d, N5.0 %d, N10 %d [#/100cc]",
                    pms.n0p3, pms.n0p5, pms.n1p0, pms.n2p5, pms.n5p0, pms.n10p0);

    if (pms.has_temperature_humidity() || pms.has_formaldehyde())
      LOG_DEBUG_FMT(PMS, "T=%.1f C, RH=%.1f %%, HCHO=%.2f mg/m3",
                    pms.temp, pms.rhum, pms.hcho);
  }
  else
  { // something went wrong
    switch (pms.status)
    {
    case pms.OK: // should never come here
      break;     // included to compile without warnings
    case pms.ERROR_TIMEOUT:
      LOG_ERROR(PMS, PMS_ERROR_TIMEOUT);
      break;
    case pms.ERROR_MSG_UNKNOWN:
      LOG_ERROR(PMS, PMS_ERROR_MSG_UNKNOWN);
      break;
    case pms.ERROR_MSG_HEADER:
      LOG_ERROR(PMS, PMS_ERROR_MSG_HEADER);
      break;
    case pms.ERROR_MSG_BODY:
      LOG_ERROR(PMS, PMS_ERROR_MSG_BODY);
      break;
    case pms.ERROR_MSG_START:
      LOG_ERROR(PMS, PMS_ERROR_MSG_START);
      break;
    case pms.ERROR_MSG_LENGTH:
      LOG_ERROR(PMS, PMS_ERROR_MSG_LENGTH);
      break;
    case pms.ERROR_MSG_CKSUM:
      LOG_ERROR(PMS, PMS_ERROR_MSG_CKSUM);
      break;
    case pms.ERROR_PMS_TYPE:
      LOG_ERROR(PMS, PMS_ERROR_PMS_TYPE);
      break;
    }
  }
}

void HTU21_prinValues(){
    float temp = htu.readTemperature();
    float hum = htu.readHumidity();
    LOG_DEBUG_FMT(HTU, "Temperature=%.1f C, Humidity=%.1f %%", temp, hum);
}

void MS5611_printValues(){
  LOG_DEBUG_FMT(MS5611, "Temperature=%d [0.01 C], Pressure=%d [Pa]",
                ms5611.readTemperature(), ms5611.readPressure());
}

