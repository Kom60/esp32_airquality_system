This project implements an air quality monitoring system using the ESP32 microcontroller. It integrates various sensors to measure and log environmental data such as PM2.5, PM10, temperature, and humidity. The system features real-time data visualization, remote monitoring capabilities, and data storage on a cloud platform. Developed using C/C++ and the Arduino framework, it demonstrates proficiency in IoT development, sensor integration, and cloud connectivity.

## Поддерживаемые датчики

| Датчик | Модель | Назначение | Единицы измерения |
|--------|--------|-----------|-------------------|
| **BME280** | BME280 | Наружная температура, давление, влажность | °C, гПа (hPa), % |
| **HTU21DF** | HTU21DF | Внутренняя температура, влажность | °C, % |
| **SCD40** | SCD40 (Sensirion) | CO₂, температура, влажность | ppm, °C, % |
| **PMS5003** | PMS5003 | Твёрдые частицы PM1.0, PM2.5, PM10 | мкг/м³ (µg/m³) |
| **MS5611** | MS5611 | Точное барометрическое давление, температура | гПа (hPa), °C |
| **BH1750** | BH1750 | Освещённость | люкс (lux, лк) |
| **VEML6070** | VEML6070 | УФ-излучение | UV index (индекс) |
| **Формальдегид** | WS160725115 | Концентрация формальдегида (CH₂O) | ppm |
| **Микрофон** | INMP441 | Уровень шума | дБ (dB) |

### Диапазоны измерений и точность

| Параметр | Диапазон | Точность |
|----------|----------|----------|
| Температура (BME280/HTU21DF) | -40...+85 °C | ±0.5 °C |
| Температура (SCD40) | -10...+60 °C | ±0.5 °C |
| Влажность (BME280/HTU21DF) | 0...100 % | ±2-3 % |
| Влажность (SCD40) | 0...100 % | ±5 % |
| Давление (BME280/MS5611) | 300...1100 гПа | ±1 гПа |
| CO₂ (SCD40) | 400...5000 ppm | ±40 ppm ±5 % |
| PM1.0/PM2.5/PM10 | 0...500 мкг/м³ | ±10 % |
| Освещённость (BH1750) | 1...65535 лк | ±20 % |
| УФ-индекс (VEML6070) | 0...15 | ±10 % |
| Формальдегид (WS160725115) | 0...0.5 ppm | ±10 % |
| Шум (INMP441) | 30...100 дБ | ±2 дБ |

---

<img width="2523" height="920" alt="image" src="https://github.com/user-attachments/assets/1af0e709-f831-4d62-aefa-845e57ca557a" />
<img width="4369" height="2210" alt="bme_pressure_20260222_213930" src="https://github.com/user-attachments/assets/2b115d8a-ea4a-40bc-9287-2a4c06b9621c" />
<img width="4263" height="2210" alt="bme_temperature_20260222_213511" src="https://github.com/user-attachments/assets/584e9cc7-e25e-4868-91fa-8b1ac0738603" />
<img src="https://github.com/Kom60/esp32_airquality_system/blob/main/222.jpg" />
<img width="1234" height="763" alt="image" src="https://github.com/user-attachments/assets/8e18eee6-f9ba-4119-b5d3-224d0dc8f607" />
<img width="1237" height="852" alt="image" src="https://github.com/user-attachments/assets/8e68e797-f1c4-46e4-a7d1-0b030840f532" />



