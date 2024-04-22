#include "headers.h"

void BME_280_printValues() {
   
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    
    Serial.println(" °C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();

}

void SCD40_printValues() {
        Serial.print("Co2:");
        Serial.print(co2Value);
        Serial.print("\t");
        Serial.print("Temperature:");
        Serial.print(temperature);
        Serial.print("\t");
        Serial.print("Humidity:");
        Serial.println(humidity);
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
    tft.drawFloat((bme.readPressure() / 100.0F),1,75,45);
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
void display_bme()
{
   tft.setTextSize(1);
    tft.drawString("OUTDOOR AIR:",10,15);
    tft.drawString("Temperature",15,30);
    tft.drawFloat(AIR_data.Outdoor_temp,1,90,30);
    tft.drawString("Celsium",120,30);
    tft.drawString("Pressure",15,45);
    tft.drawFloat(AIR_data.Outdoor_pressure,1,75,45);
    tft.drawString("hPascal",115,45);
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
  tft.drawFloat(AIR_data.Indoor_pressure,1,75,105);
  tft.drawString("hPascal",115,105);
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
void display_Info()
{
  Serial.print("1");
}
void PME5003_printValues(SerialPM pms){
  pms.read();
  if (pms)
  { // successfull read
    // print formatted results
    Serial.printf("PM1.0 %2d, PM2.5 %2d, PM10 %2d [ug/m3]\n",
                  pms.pm01, pms.pm25, pms.pm10);

    if (pms.has_number_concentration())
      Serial.printf("N0.3 %4d, N0.5 %3d, N1.0 %2d, N2.5 %2d, N5.0 %2d, N10 %2d [#/100cc]\n",
                    pms.n0p3, pms.n0p5, pms.n1p0, pms.n2p5, pms.n5p0, pms.n10p0);

    if (pms.has_temperature_humidity() || pms.has_formaldehyde())
      Serial.printf("%5.1f °C, %5.1f %%rh, %5.2f mg/m3 HCHO\n",
                    pms.temp, pms.rhum, pms.hcho);
  }
  else
  { // something went wrong
    switch (pms.status)
    {
    case pms.OK: // should never come here
      break;     // included to compile without warnings
    case pms.ERROR_TIMEOUT:
      Serial.println(F(PMS_ERROR_TIMEOUT));
      break;
    case pms.ERROR_MSG_UNKNOWN:
      Serial.println(F(PMS_ERROR_MSG_UNKNOWN));
      break;
    case pms.ERROR_MSG_HEADER:
      Serial.println(F(PMS_ERROR_MSG_HEADER));
      break;
    case pms.ERROR_MSG_BODY:
      Serial.println(F(PMS_ERROR_MSG_BODY));
      break;
    case pms.ERROR_MSG_START:
      Serial.println(F(PMS_ERROR_MSG_START));
      break;
    case pms.ERROR_MSG_LENGTH:
      Serial.println(F(PMS_ERROR_MSG_LENGTH));
      break;
    case pms.ERROR_MSG_CKSUM:
      Serial.println(F(PMS_ERROR_MSG_CKSUM));
      break;
    case pms.ERROR_PMS_TYPE:
      Serial.println(F(PMS_ERROR_PMS_TYPE));
      break;
    }
  }
}
void HTU21_prinValues(){
    float temp = htu.readTemperature();
    float hum = htu.readHumidity();
    Serial.print("Temperature(°C): "); 
    Serial.print(temp); 
    Serial.print("\t\t");
    Serial.print("Humidity(%): "); 
    Serial.println(hum);
}
/*
void MS5611_printValues(){
  MS5611.read();           //  note no error checking => "optimistic".
  Serial.print("T:\t");
  Serial.print(MS5611.getTemperature(), 2);
  Serial.print("\tP:\t");
  Serial.print(MS5611.getPressure(), 2);
  Serial.println();
}*/

