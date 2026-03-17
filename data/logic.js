let counter=1;
let Socket;

// === Хранение данных для расчётных показателей ===
const sensorData = {
  bme_temperature: null,
  bme_humidity: null,
  bme_pressure: null,
  htu_temperature: null,
  htu_humidity: null,
  scd4x_co2: null,
  scd4x_temperature: null,
  scd4x_humidity: null,
  pms_pm1: null,
  pms_pm2_5: null,
  pms_pm10: null,
  bh1750_lighting: null,
  veml_uv: null,
  ch2o_value: null,
  microphone_noise: null,
  pressure_history: []  // Для барометрической тенденции
};

// Пороговые значения для цветовой индикации
const SENSOR_THRESHOLDS = {
  'bme_temperature':      { min: -10, max: 40, warnMin: -20, warnMax: 50 },
  'bme_pressure':         { min: 980, max: 1040, warnMin: 960, warnMax: 1060 },
  'bme_humidity':         { min: 20, max: 90, warnMin: 10, warnMax: 95 },
  'htu_temperature':      { min: 18, max: 26, warnMin: 15, warnMax: 30 },
  'htu_humidity':         { min: 30, max: 60, warnMin: 20, warnMax: 80 },
  'ms5611_pressure':      { min: 980, max: 1040, warnMin: 960, warnMax: 1060 },  // гПа
  'ms5611_temperature':   { min: 18, max: 26, warnMin: 15, warnMax: 30 },
  'scd4x_co2':            { min: 400, max: 1000, warnMin: 400, warnMax: 1400 },
  'scd4x_temperature':    { min: 18, max: 26, warnMin: 15, warnMax: 30 },
  'scd4x_humidity':       { min: 30, max: 60, warnMin: 20, warnMax: 80 },
  'pms_pm1':              { min: 0, max: 35, warnMin: 0, warnMax: 50 },
  'pms_pm2_5':            { min: 0, max: 25, warnMin: 0, warnMax: 50 },
  'pms_pm10':             { min: 0, max: 50, warnMin: 0, warnMax: 150 },
  'bh1750_lighting':      { min: 300, max: 1000, warnMin: 100, warnMax: 2000 },
  'veml_uv':              { min: 0, max: 5, warnMin: 0, warnMax: 8 },
  'ch2o_value':           { min: 0, max: 0.08, warnMin: 0, warnMax: 0.1 },
  'microphone_noise':     { min: 30, max: 55, warnMin: 20, warnMax: 70 }
};

// Определение статуса по значению
function getStatus(value, thresholds) {
  if (value === null || value === undefined || isNaN(value)) return null;
  if (value < thresholds.warnMin || value > thresholds.warnMax) return 'critical';
  if (value < thresholds.min || value > thresholds.max) return 'warning';
  return 'ok';
}

// ============================================================
// === РАСЧЁТНЫЕ ПОКАЗАТЕЛИ ====================================
// ============================================================

// 1. Точка росы (Dew Point) - формула Магнуса
function calculateDewPoint(temp, humidity) {
  if (temp === null || humidity === null || isNaN(temp) || isNaN(humidity)) return null;
  const a = 17.27;
  const b = 237.7;
  const alpha = ((a * temp) / (b + temp)) + Math.log(humidity / 100.0);
  return (b * alpha) / (a - alpha);
}

// 2. Индекс жары (Heat Index) - упрощённая формула для °C
function calculateHeatIndex(temp, humidity) {
  if (temp === null || humidity === null || isNaN(temp) || isNaN(humidity)) return null;
  // Работает при temp > 27°C и humidity > 40%
  if (temp < 27 || humidity < 40) return temp;
  
  const HI = -8.784695 + 1.61139411*temp + 2.338549*humidity 
           + 0.14611605*temp*humidity - 0.012308094*temp*temp 
           - 0.016424828*humidity*humidity + 0.002211732*temp*temp*humidity 
           + 0.00072546*temp*humidity*humidity - 0.000003582*temp*temp*humidity*humidity;
  return HI;
}

// 3. Индекс качества воздуха (IAQ)
function calculateIAQ(co2, pm25, pm10, ch2o) {
  let aqi = 0;
  
  // CO₂ вклад (0-200 баллов)
  if (co2 !== null && !isNaN(co2)) {
    if (co2 <= 800) aqi += Math.round((co2 / 800) * 50);
    else if (co2 <= 1400) aqi += Math.round(50 + ((co2 - 800) / 600) * 100);
    else aqi += Math.round(150 + Math.min((co2 - 1400) / 60, 50));
  }
  
  // PM2.5 вклад (0-200 баллов) - по ВОЗ
  if (pm25 !== null && !isNaN(pm25)) {
    if (pm25 <= 15) aqi += Math.round((pm25 / 15) * 50);
    else if (pm25 <= 50) aqi += Math.round(50 + ((pm25 - 15) / 35) * 100);
    else aqi += Math.round(150 + Math.min((pm25 - 50) / 50, 50));
  }
  
  // PM10 вклад (0-100 баллов)
  if (pm10 !== null && !isNaN(pm10)) {
    if (pm10 <= 50) aqi += Math.round((pm10 / 50) * 50);
    else aqi += Math.round(50 + Math.min((pm10 - 50) / 100, 50));
  }
  
  // Формальдегид вклад (0-100 баллов) - мг/м³
  // 0-0.08: хорошо, 0.08-0.1: умеренно, 0.1-0.2: вредно чувствительным, 0.2-0.5: вредно, >0.5: очень вредно
  if (ch2o !== null && !isNaN(ch2o)) {
    if (ch2o <= 0.08) aqi += Math.round((ch2o / 0.08) * 50);
    else if (ch2o <= 0.1) aqi += Math.round(50 + ((ch2o - 0.08) / 0.02) * 50);
    else if (ch2o <= 0.2) aqi += Math.round(100 + ((ch2o - 0.1) / 0.1) * 50);
    else if (ch2o <= 0.5) aqi += Math.round(150 + ((ch2o - 0.2) / 0.3) * 50);
    else aqi += Math.round(200 + Math.min((ch2o - 0.5) / 0.5, 50));
  }
  
  return Math.min(aqi, 500);  // Максимум 500
}

function getIAQDescription(iaq) {
  if (iaq <= 50) return { text: 'Отлично', class: 'iaq-good' };
  if (iaq <= 100) return { text: 'Нормально', class: 'iaq-moderate' };
  if (iaq <= 150) return { text: 'Посредственно', class: 'iaq-unhealthy-sensitive' };
  if (iaq <= 200) return { text: 'Плохо', class: 'iaq-unhealthy' };
  if (iaq <= 300) return { text: 'Очень плохо', class: 'iaq-very-unhealthy' };
  return { text: 'Опасно', class: 'iaq-hazardous' };
}

// 4. Барометрическая тенденция
function calculatePressureTrend(currentPressure) {
  if (currentPressure === null || isNaN(currentPressure)) return null;

  // Добавляем текущее значение с временной меткой
  sensorData.pressure_history.push({
    pressure: currentPressure,
    time: Date.now()
  });

  // Храним только последние 3 часа
  const threeHoursAgo = Date.now() - (3 * 60 * 60 * 1000);
  sensorData.pressure_history = sensorData.pressure_history.filter(
    item => item.time > threeHoursAgo
  );

  // Нужно минимум 2 значения для сравнения
  if (sensorData.pressure_history.length < 2) return null;

  const firstPressure = sensorData.pressure_history[0].pressure;
  const trend = currentPressure - firstPressure;

  if (trend > 2) return { value: trend, text: 'Растёт', desc: 'К улучшению погоды' };
  if (trend < -2) return { value: trend, text: 'Падает', desc: 'К ухудшению погоды' };
  return { value: trend, text: 'Стабильно', desc: 'Без изменений' };
}

// 5. УФ-индекс с рекомендациями
function getUVRecommendation(uvIndex) {
  if (uvIndex === null || isNaN(uvIndex)) return null;

  if (uvIndex <= 2) {
    return { text: 'Защита не требуется', class: 'uv-low' };
  } else if (uvIndex <= 5) {
    return { text: 'Надеть очки', class: 'uv-moderate' };
  } else if (uvIndex <= 7) {
    return { text: 'Использовать SPF 30+', class: 'uv-high' };
  } else if (uvIndex <= 10) {
    return { text: 'Избегать солнца 11-16ч', class: 'uv-very-high' };
  } else {
    return { text: 'Опасно! В помещении', class: 'uv-extreme' };
  }
}

// 6. Индекс комфортности помещения
function calculateComfortIndex(temp, humidity, co2) {
  if (temp === null || humidity === null || co2 === null) return null;
  if (isNaN(temp) || isNaN(humidity) || isNaN(co2)) return null;

  let score = 100;

  // Температура (оптимум 20-23°C)
  if (temp < 18 || temp > 26) score -= 30;
  else if (temp < 20 || temp > 24) score -= 15;

  // Влажность (оптимум 40-60%)
  if (humidity < 30 || humidity > 70) score -= 30;
  else if (humidity < 40 || humidity > 60) score -= 15;

  // CO2 (оптимум <800 ppm)
  if (co2 > 1400) score -= 30;
  else if (co2 > 1000) score -= 15;
  else if (co2 > 800) score -= 5;

  return Math.max(0, Math.min(100, score));
}

function getComfortDescription(score) {
  if (score >= 80) return { text: 'Комфортно', class: 'comfort-good' };
  if (score >= 50) return { text: 'Нормально', class: 'comfort-moderate' };
  return { text: 'Некомфортно', class: 'comfort-poor' };
}

// 7. Рекомендации по проветриванию
function getVentilationRecommendation() {
  const co2 = sensorData.scd4x_co2;
  const indoorTemp = sensorData.htu_temperature || sensorData.scd4x_temperature;
  const outdoorTemp = sensorData.bme_temperature;
  const uv = sensorData.veml_uv;
  const hour = new Date().getHours();

  if (co2 === null || indoorTemp === null) {
    return { text: 'Нет данных', class: 'vent-wait' };
  }

  // Проверка на высокий CO2
  if (co2 > 1000) {
    if (outdoorTemp !== null && outdoorTemp < indoorTemp) {
      if (uv !== null && uv < 6) {
        return { text: 'Проветрить!', class: 'vent-recommend' };
      }
    }
    return { text: 'Высокий CO2', class: 'vent-warning' };
  }

  // Проверка на высокий УФ
  if (uv !== null && uv > 7) {
    return { text: 'Закрыть шторы', class: 'vent-uv' };
  }

  // Энергосбережение
  if (co2 < 600 && outdoorTemp !== null && outdoorTemp > indoorTemp) {
    return { text: 'Закрыть окна', class: 'vent-save' };
  }

  return { text: 'Всё в норме', class: 'vent-ok' };
}

// 9. Абсолютная влажность (г/м³)
function calculateAbsoluteHumidity(temp, humidity) {
  if (temp === null || humidity === null || isNaN(temp) || isNaN(humidity)) return null;
  
  // Максимальная плотность водяного пара при данной температуре
  const maxVaporDensity = 17.3 * Math.pow(1.06, temp);
  return (humidity / 100.0) * maxVaporDensity;
}

// 10. Рекомендации по освещению
function getLightingRecommendation(lux) {
  if (lux === null || isNaN(lux)) {
    return { text: 'Нет данных', class: 'light-wait' };
  }

  const hour = new Date().getHours();
  const isDaytime = hour >= 8 && hour <= 18;

  if (isDaytime) {
    if (lux < 300) {
      return { text: 'Недостаточно света', class: 'light-low' };
    } else if (lux > 1000) {
      return { text: 'Хорошее освещение', class: 'light-good' };
    }
    return { text: 'Нормально', class: 'light-moderate' };
  } else {
    if (lux < 50) {
      return { text: 'Вечернее освещение', class: 'light-evening' };
    }
    return { text: 'Яркий свет', class: 'light-bright' };
  }
}

// 11. Дефицит точки росы (разница между температурой и точкой росы)
function calculateDewPointDeficit(temp, dewPoint) {
  if (temp === null || dewPoint === null || isNaN(temp) || isNaN(dewPoint)) return null;
  return temp - dewPoint;
}

function getDewPointDeficitDescription(deficit) {
  if (deficit === null || isNaN(deficit)) return null;
  if (deficit > 15) return { text: 'Очень сухо', class: 'deficit-very-dry' };
  if (deficit > 10) return { text: 'Сухо', class: 'deficit-dry' };
  if (deficit > 5) return { text: 'Норма', class: 'deficit-normal' };
  return { text: 'Влажно', class: 'deficit-humid' };
}

// 12. Энтальпия воздуха (кДж/кг) - формула для влажного воздуха
function calculateEnthalpy(temp, humidity) {
  if (temp === null || humidity === null || isNaN(temp) || isNaN(humidity)) return null;
  // Упрощённая формула: h = 1.006*t + w*(2501 + 1.86*t)
  // где w - влажность в кг воды/кг сухого воздуха
  const p = 101325; // атмосферное давление, Па
  const ws = 0.622 * (610.78 * Math.exp(17.2694 * temp / (temp + 238.3))) / (p - 610.78 * Math.exp(17.2694 * temp / (temp + 238.3)));
  const w = (humidity / 100) * ws;
  return 1.006 * temp + w * (2501 + 1.86 * temp);
}

function getEnthalpyDescription(enthalpy) {
  if (enthalpy === null || isNaN(enthalpy)) return null;
  if (enthalpy < 25) return { text: 'Холодный', class: 'enthalpy-cold' };
  if (enthalpy < 40) return { text: 'Прохладный', class: 'enthalpy-cool' };
  if (enthalpy < 55) return { text: 'Комфортный', class: 'enthalpy-comfort' };
  if (enthalpy < 70) return { text: 'Тёплый', class: 'enthalpy-warm' };
  return { text: 'Горячий', class: 'enthalpy-hot' };
}

// 13. Риск плесени (на основе влажности и точки росы)
function calculateMoldRisk(humidity, dewPoint, temp) {
  if (humidity === null || dewPoint === null || temp === null) return null;
  if (isNaN(humidity) || isNaN(dewPoint) || isNaN(temp)) return null;
  
  let risk = 0;
  // Высокая влажность (>70%) — основной фактор
  if (humidity > 80) risk += 40;
  else if (humidity > 70) risk += 25;
  else if (humidity > 60) risk += 10;
  
  // Точка росы >15°C — риск конденсата
  if (dewPoint > 18) risk += 35;
  else if (dewPoint > 15) risk += 20;
  else if (dewPoint > 12) risk += 10;
  
  // Маленькая разница между температурой и точкой росы
  const deficit = temp - dewPoint;
  if (deficit < 3) risk += 25;
  else if (deficit < 5) risk += 15;
  
  return Math.min(100, risk);
}

function getMoldRiskDescription(risk) {
  if (risk === null || isNaN(risk)) return null;
  if (risk < 20) return { text: 'Низкий', class: 'mold-low' };
  if (risk < 40) return { text: 'Умеренный', class: 'mold-moderate' };
  if (risk < 60) return { text: 'Высокий', class: 'mold-high' };
  return { text: 'Опасный', class: 'mold-danger' };
}

// 14. Риск статического электричества (на основе влажности)
function calculateStaticElectricityRisk(humidity) {
  if (humidity === null || isNaN(humidity)) return null;
  if (humidity < 20) return { text: 'Высокий', class: 'static-high' };
  if (humidity < 30) return { text: 'Средний', class: 'static-medium' };
  if (humidity < 40) return { text: 'Низкий', class: 'static-low' };
  return { text: 'Нет', class: 'static-none' };
}

// 15. Ощущаемая температура (Feels Like) - комбинация Heat Index и Wind Chill
function calculateFeelsLike(temp, humidity) {
  if (temp === null || humidity === null || isNaN(temp) || isNaN(humidity)) return null;
  
  // Для высоких температур используем индекс жары
  if (temp >= 27) {
    return calculateHeatIndex(temp, humidity);
  }
  
  // Для низких температур — упрощённый wind chill
  if (temp <= 10) {
    // Wind chill формула для °C
    const wc = 13.12 + 0.6215 * temp - 11.37 * Math.pow(0.5, 0.16) + 0.3965 * temp * Math.pow(0.5, 0.16);
    return wc;
  }
  
  // Для умеренных температур — просто температура
  return temp;
}

// 16. Время до проветривания (прогноз достижения CO2 = 1000 ppm)
function calculateVentilationTime(co2) {
  if (co2 === null || isNaN(co2)) return null;
  if (co2 >= 1000) return 0; // Уже пора
  
  // Сохраняем историю CO2 для расчёта тренда
  if (!sensorData.co2_history) sensorData.co2_history = [];
  sensorData.co2_history.push({ value: co2, time: Date.now() });
  
  // Храним последние 30 минут
  const thirtyMinAgo = Date.now() - 30 * 60 * 1000;
  sensorData.co2_history = sensorData.co2_history.filter(item => item.time > thirtyMinAgo);
  
  // Если есть история, считаем скорость роста
  if (sensorData.co2_history.length >= 2) {
    const first = sensorData.co2_history[0];
    const last = sensorData.co2_history[sensorData.co2_history.length - 1];
    const timeDiffHours = (last.time - first.time) / (1000 * 60 * 60);
    if (timeDiffHours > 0) {
      const co2RatePerHour = (last.value - first.value) / timeDiffHours;
      if (co2RatePerHour > 0) {
        const minutesTo1000 = ((1000 - co2) / co2RatePerHour) * 60;
        return Math.max(0, Math.round(minutesTo1000));
      }
    }
  }
  
  // Если нет тренда, предполагаем среднюю скорость роста 50 ppm/час
  const defaultRate = 50; // ppm/час
  return Math.round(((1000 - co2) / defaultRate) * 60);
}

function getVentilationTimeDescription(minutes) {
  if (minutes === null) return null;
  if (minutes <= 0) return { text: 'Сейчас!', class: 'vent-now' };
  if (minutes < 15) return { text: 'Скоро', class: 'vent-soon' };
  if (minutes < 30) return { text: '15-30', class: 'vent-moderate' };
  return { text: '>30', class: 'vent-later' };
}

// 17. Качество сна (на основе температуры, влажности и CO2)
function calculateSleepQuality(temp, humidity, co2) {
  if (temp === null || humidity === null || co2 === null) return null;
  if (isNaN(temp) || isNaN(humidity) || isNaN(co2)) return null;
  
  let score = 100;
  
  // Температура (оптимум для сна: 18-20°C)
  if (temp >= 18 && temp <= 20) score += 0;
  else if (temp >= 16 && temp <= 22) score -= 10;
  else if (temp >= 14 && temp <= 24) score -= 25;
  else score -= 40;
  
  // Влажность (оптимум: 40-60%)
  if (humidity >= 40 && humidity <= 60) score += 0;
  else if (humidity >= 30 && humidity <= 70) score -= 10;
  else if (humidity >= 20 && humidity <= 80) score -= 25;
  else score -= 40;
  
  // CO2 (оптимум: <800 ppm)
  if (co2 < 800) score += 0;
  else if (co2 < 1000) score -= 15;
  else if (co2 < 1400) score -= 30;
  else score -= 45;
  
  return Math.max(0, Math.min(100, score));
}

function getSleepQualityDescription(score) {
  if (score === null || isNaN(score)) return null;
  if (score >= 85) return { text: 'Отлично', class: 'sleep-excellent' };
  if (score >= 70) return { text: 'Хорошо', class: 'sleep-good' };
  if (score >= 50) return { text: 'Нормально', class: 'sleep-fair' };
  return { text: 'Плохо', class: 'sleep-poor' };
}

// 18. AQI по PM2.5 (по шкале ВОЗ)
function calculatePM25AQI(pm25) {
  if (pm25 === null || isNaN(pm25)) return null;
  
  // Шкала AQI ВОЗ для PM2.5 (мкг/м³)
  if (pm25 <= 15) return Math.round((pm25 / 15) * 50);
  if (pm25 <= 25) return Math.round(50 + ((pm25 - 15) / 10) * 50);
  if (pm25 <= 50) return Math.round(100 + ((pm25 - 25) / 25) * 50);
  if (pm25 <= 100) return Math.round(150 + ((pm25 - 50) / 50) * 50);
  if (pm25 <= 200) return Math.round(200 + ((pm25 - 100) / 100) * 100);
  return Math.min(500, Math.round(300 + ((pm25 - 200) / 100) * 200));
}

function getPM25AQIDescription(aqi) {
  if (aqi === null || isNaN(aqi)) return null;
  if (aqi <= 50) return { text: 'Отлично', class: 'aqi-good' };
  if (aqi <= 100) return { text: 'Нормально', class: 'aqi-moderate' };
  if (aqi <= 150) return { text: 'Посредственно', class: 'aqi-unhealthy-sensitive' };
  if (aqi <= 200) return { text: 'Плохо', class: 'aqi-unhealthy' };
  if (aqi <= 300) return { text: 'Очень плохо', class: 'aqi-very-unhealthy' };
  return { text: 'Опасно', class: 'aqi-hazardous' };
}

// ============================================================
// === НОВЫЕ РАСЧЁТНЫЕ ПОКАЗАТЕЛИ (10) =========================
// ============================================================

// 19. Точка замерзания (температура образования инея)
function calculateFreezingPoint(temp, dewPoint) {
  if (temp === null || dewPoint === null || isNaN(temp) || isNaN(dewPoint)) return null;
  // Упрощённая формула: 0.5 * (Температура + Точка росы)
  return 0.5 * (temp + dewPoint);
}

function getFreezingPointDescription(freezePoint) {
  if (freezePoint === null || isNaN(freezePoint)) return null;
  if (freezePoint <= -5) return { text: 'Риск инея', class: 'freeze-danger' };
  if (freezePoint <= 0) return { text: 'Около нуля', class: 'freeze-warning' };
  return { text: 'Без риска', class: 'freeze-safe' };
}

// 20. Индекс загрязнения (комплексный индекс частиц)
function calculatePollutionIndex(pm25, pm10) {
  if (pm25 === null || pm10 === null || isNaN(pm25) || isNaN(pm10)) return null;
  // PM2.5 * 0.7 + PM10 * 0.3 (PM2.5 более вредные)
  return pm25 * 0.7 + pm10 * 0.3;
}

function getPollutionIndexDescription(index) {
  if (index === null || isNaN(index)) return null;
  if (index <= 25) return { text: 'Чисто', class: 'pollution-excellent' };
  if (index <= 50) return { text: 'Нормально', class: 'pollution-good' };
  if (index <= 75) return { text: 'Загрязнено', class: 'pollution-moderate' };
  if (index <= 100) return { text: 'Плохо', class: 'pollution-unhealthy' };
  return { text: 'Опасно', class: 'pollution-hazardous' };
}

// 21. Время безопасного пребывания (прогноз достижения CO2 = 1400 ppm)
function calculateSafeExposureTime(co2) {
  if (co2 === null || isNaN(co2)) return null;
  if (co2 >= 1400) return 0; // Уже опасно

  // Сохраняем историю CO2 для расчёта тренда
  if (!sensorData.co2_history) sensorData.co2_history = [];
  sensorData.co2_history.push({ value: co2, time: Date.now() });

  // Храним последние 30 минут
  const thirtyMinAgo = Date.now() - 30 * 60 * 1000;
  sensorData.co2_history = sensorData.co2_history.filter(item => item.time > thirtyMinAgo);

  // Если есть история, считаем скорость роста
  if (sensorData.co2_history.length >= 2) {
    const first = sensorData.co2_history[0];
    const last = sensorData.co2_history[sensorData.co2_history.length - 1];
    const timeDiffMinutes = (last.time - first.time) / (1000 * 60);
    if (timeDiffMinutes > 0) {
      const co2RatePerMin = (last.value - first.value) / timeDiffMinutes;
      if (co2RatePerMin > 0) {
        const minutesTo1400 = ((1400 - co2) / co2RatePerMin);
        return Math.max(0, Math.round(minutesTo1400));
      }
    }
  }

  // Если нет тренда, предполагаем среднюю скорость роста 50 ppm/час
  const defaultRate = 50 / 60; // ppm/мин
  return Math.round(((1400 - co2) / defaultRate));
}

function getSafeExposureTimeDescription(minutes) {
  if (minutes === null) return null;
  if (minutes <= 0) return { text: 'Сейчас!', class: 'safe-now' };
  if (minutes < 15) return { text: '<15', class: 'safe-soon' };
  if (minutes < 30) return { text: '15-30', class: 'safe-moderate' };
  if (minutes < 60) return { text: '30-60', class: 'safe-good' };
  return { text: '>60', class: 'safe-excellent' };
}

// 22. Эффективность проветривания (скорость снижения CO2)
function calculateVentilationEfficiency() {
  if (!sensorData.co2_history || sensorData.co2_history.length < 2) return null;
  
  const first = sensorData.co2_history[0];
  const last = sensorData.co2_history[sensorData.co2_history.length - 1];
  const timeDiffMinutes = (last.time - first.time) / (1000 * 60);
  
  if (timeDiffMinutes <= 0) return null;
  const co2Change = first.value - last.value; // Отрицательное = рост, положительное = снижение
  
  return co2Change / timeDiffMinutes; // ppm/мин
}

function getVentilationEfficiencyDescription(efficiency) {
  if (efficiency === null) return null;
  if (efficiency > 10) return { text: 'Отлично', class: 'vent-eff-excellent' };
  if (efficiency > 5) return { text: 'Хорошо', class: 'vent-eff-good' };
  if (efficiency > 0) return { text: 'Нормально', class: 'vent-eff-fair' };
  if (efficiency > -5) return { text: 'Слабо', class: 'vent-eff-poor' };
  return { text: 'Растёт CO2', class: 'vent-eff-bad' };
}

// 23. Индекс духоты (Simpson Comfort Index)
function calculateDiscomfortIndex(temp, humidity) {
  if (temp === null || humidity === null || isNaN(temp) || isNaN(humidity)) return null;
  // Упрощённая формула: Temp + 0.33*Humidity - 0.7
  return temp + 0.33 * humidity - 0.7;
}

function getDiscomfortIndexDescription(index) {
  if (index === null || isNaN(index)) return null;
  if (index < 21) return { text: 'Холодно', class: 'discomfort-cold' };
  if (index < 24) return { text: 'Комфортно', class: 'discomfort-comfort' };
  if (index < 27) return { text: 'Тепло', class: 'discomfort-warm' };
  if (index < 30) return { text: 'Душно', class: 'discomfort-stuffy' };
  return { text: 'Жарко', class: 'discomfort-hot' };
}

// 24. Риск аллергии (комбинированный риск по частицам и влажности)
function calculateAllergyRisk(pm25, pm10, humidity) {
  if (pm25 === null || pm10 === null || humidity === null) return null;
  if (isNaN(pm25) || isNaN(pm10) || isNaN(humidity)) return null;

  let risk = 0;
  // PM2.5 и PM10 вклад (0-50 баллов)
  risk += Math.min(pm25, 50) * 0.6;
  risk += Math.min(pm10, 50) * 0.4;
  
  // Высокая влажность (>60%) увеличивает риск плесени и клещей
  if (humidity > 70) risk += 20;
  else if (humidity > 60) risk += 10;
  
  // Низкая влажность (<30%) увеличивает риск раздражения
  if (humidity < 25) risk += 10;
  
  return Math.min(100, risk);
}

function getAllergyRiskDescription(risk) {
  if (risk === null || isNaN(risk)) return null;
  if (risk < 20) return { text: 'Низкий', class: 'allergy-low' };
  if (risk < 40) return { text: 'Умеренный', class: 'allergy-moderate' };
  if (risk < 60) return { text: 'Высокий', class: 'allergy-high' };
  return { text: 'Опасный', class: 'allergy-danger' };
}

// 25. Оптимальное время для сна (рекомендация)
function calculateOptimalSleepTime(temp, humidity, co2, noise) {
  if (temp === null || humidity === null || co2 === null) return null;
  
  let score = 100;
  const hour = new Date().getHours();
  
  // Температура (оптимум для сна: 18-20°C)
  if (temp >= 18 && temp <= 20) score += 0;
  else if (temp >= 16 && temp <= 22) score -= 10;
  else if (temp >= 14 && temp <= 24) score -= 25;
  else score -= 40;
  
  // Влажность (оптимум: 40-60%)
  if (humidity >= 40 && humidity <= 60) score += 0;
  else if (humidity >= 30 && humidity <= 70) score -= 10;
  else score -= 25;
  
  // CO2 (оптимум: <800 ppm)
  if (co2 < 800) score += 0;
  else if (co2 < 1000) score -= 15;
  else if (co2 < 1400) score -= 30;
  else score -= 45;
  
  // Шум (оптимум: <35 дБ)
  if (noise !== null && !isNaN(noise)) {
    if (noise < 35) score += 0;
    else if (noise < 45) score -= 10;
    else if (noise < 55) score -= 25;
    else score -= 40;
  }
  
  score = Math.max(0, Math.min(100, score));
  
  // Определяем оптимальное время
  if (score >= 85) return { text: 'Сейчас!', class: 'sleep-time-now', value: hour };
  if (score >= 70) return { text: 'Через 1-2ч', class: 'sleep-time-soon', value: (hour + 1) % 24 };
  if (score >= 50) return { text: 'Лучше позже', class: 'sleep-time-later', value: (hour + 2) % 24 };
  return { text: 'Не сейчас', class: 'sleep-time-wait', value: null };
}

// 26. Индекс продуктивности (влияние на работоспособность)
function calculateProductivityIndex(co2, temp, lux) {
  if (co2 === null || temp === null || lux === null) return null;
  if (isNaN(co2) || isNaN(temp) || isNaN(lux)) return null;
  
  let score = 100;
  
  // CO2 влияние (при >1000 ppm продуктивность падает)
  if (co2 < 800) score += 0;
  else if (co2 < 1000) score -= 10;
  else if (co2 < 1400) score -= 25;
  else if (co2 < 2000) score -= 40;
  else score -= 60;
  
  // Температура (оптимум: 21-23°C)
  if (temp >= 21 && temp <= 23) score += 0;
  else if (temp >= 19 && temp <= 25) score -= 10;
  else if (temp >= 17 && temp <= 27) score -= 25;
  else score -= 40;
  
  // Освещение (оптимум для работы: 500-1000 лк)
  if (lux >= 500 && lux <= 1000) score += 0;
  else if (lux >= 300 && lux <= 1500) score -= 10;
  else if (lux >= 150 && lux <= 2000) score -= 25;
  else score -= 40;
  
  return Math.max(0, Math.min(100, score));
}

function getProductivityIndexDescription(score) {
  if (score === null || isNaN(score)) return null;
  if (score >= 85) return { text: 'Отлично', class: 'prod-excellent' };
  if (score >= 70) return { text: 'Хорошо', class: 'prod-good' };
  if (score >= 50) return { text: 'Нормально', class: 'prod-fair' };
  if (score >= 30) return { text: 'Низко', class: 'prod-poor' };
  return { text: 'Критично', class: 'prod-critical' };
}

// 27. Баланс кислорода (косвенная оценка по CO2 и давлению)
function calculateOxygenBalance(co2, pressure) {
  if (co2 === null || pressure === null) return null;
  if (isNaN(co2) || isNaN(pressure)) return null;
  
  // Нормальный уровень O2 ~20.9% при нормальном давлении
  // При росте CO2 доля O2 уменьшается
  const normalO2 = 20.9;
  const co2Impact = (co2 - 400) / 10000; // Влияние CO2 на O2
  const pressureImpact = (pressure - 1013) / 1013 * normalO2; // Влияние давления
  
  const estimatedO2 = normalO2 - co2Impact + pressureImpact * 0.1;
  return Math.max(0, Math.min(100, (estimatedO2 / normalO2) * 100));
}

function getOxygenBalanceDescription(balance) {
  if (balance === null || isNaN(balance)) return null;
  if (balance >= 95) return { text: 'Норма', class: 'o2-normal' };
  if (balance >= 85) return { text: 'Снижен', class: 'o2-low' };
  if (balance >= 70) return { text: 'Низкий', class: 'o2-very-low' };
  return { text: 'Гипоксия', class: 'o2-danger' };
}

// 28. Тепловая нагрузка (WBGT - Wet Bulb Globe Temperature)
function calculateHeatStressIndex(temp, humidity, lux) {
  if (temp === null || humidity === null || lux === null) return null;
  if (isNaN(temp) || isNaN(humidity) || isNaN(lux)) return null;
  
  // Упрощённый расчёт WBGT
  // Tw (влажный термометр) - аппроксимация по температуре и влажности
  const Tw = temp * Math.atan(0.151977 * Math.pow(humidity + 8.313659, 0.5)) 
           + Math.atan(temp + humidity) 
           - Math.atan(humidity - 1.676331) 
           + Math.pow(0.00391838 * humidity, 1.5) * Math.atan(0.023101 * humidity) 
           - 4.686035;
  
  // Tg (температура глобуса) - аппроксимация по освещению
  const Tg = temp + (lux / 1000) * 5; // Упрощённо
  
  // Ta (температура воздуха)
  const Ta = temp;
  
  // WBGT = 0.7*Tw + 0.2*Tg + 0.1*Ta
  const wbgt = 0.7 * Tw + 0.2 * Tg + 0.1 * Ta;
  
  return wbgt;
}

function getHeatStressIndexDescription(wbgt) {
  if (wbgt === null || isNaN(wbgt)) return null;
  if (wbgt < 18) return { text: 'Холодно', class: 'wbgt-cold' };
  if (wbgt < 24) return { text: 'Комфортно', class: 'wbgt-comfort' };
  if (wbgt < 28) return { text: 'Тепло', class: 'wbgt-warm' };
  if (wbgt < 32) return { text: 'Жарко', class: 'wbgt-hot' };
  return { text: 'Опасно', class: 'wbgt-danger' };
}

// Обновление всех расчётных показателей
function updateCalculatedValues() {
  // Точка росы (по HTU21DF -室内)
  const dewPoint = calculateDewPoint(sensorData.htu_temperature, sensorData.htu_humidity);
  const dewPointEl = document.querySelector('.calculated_value.dew_point');
  const dewPointCard = document.querySelector('[data-analytics="dew_point"]');
  if (dewPointEl) {
    dewPointEl.innerHTML = dewPoint !== null ? dewPoint.toFixed(1) : '---';
    // Цветовая дифференциация
    dewPointEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (dewPoint !== null) {
      if (dewPoint >= 10 && dewPoint <= 16) dewPointEl.classList.add('value-good');
      else if (dewPoint < 10 || dewPoint > 18) dewPointEl.classList.add('value-warning');
      else dewPointEl.classList.add('value-info');
    }
  }
  if (dewPointCard) {
    dewPointCard.setAttribute('title', dewPoint !== null
      ? `Точка росы: ${dewPoint.toFixed(1)}°C\n\n` +
        `Это температура, при которой воздух достигнет насыщения влагой.\n` +
        `<10°C: Сухо\n10-16°C: Комфортно\n16-18°C: Влажно\n>18°C: Очень душно\n\n` +
        `При такой температуре образуется роса, туман или конденсат.`
      : 'Нет данных для расчёта');
  }

  // Индекс жары
  const heatIndex = calculateHeatIndex(sensorData.htu_temperature, sensorData.htu_humidity);
  const heatIndexEl = document.querySelector('.calculated_value.heat_index');
  const heatIndexCard = document.querySelector('[data-analytics="heat_index"]');
  if (heatIndexEl) {
    heatIndexEl.innerHTML = heatIndex !== null ? heatIndex.toFixed(1) : '---';
    // Цветовая дифференциация
    heatIndexEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (heatIndex !== null) {
      if (heatIndex < 27) heatIndexEl.classList.add('value-good');
      else if (heatIndex < 32) heatIndexEl.classList.add('value-warning');
      else heatIndexEl.classList.add('value-critical');
    }
  }
  if (heatIndexCard) {
    heatIndexCard.setAttribute('title', heatIndex !== null
      ? `Индекс жары: ${heatIndex.toFixed(1)}°C\n\n` +
        `Ощущаемая температура с учётом влажности:\n` +
        `<27°C: Комфортно\n27-32°C: Внимание\n32-39°C: Опасно\n>39°C: Крайне опасно\n\n` +
        `Высокая влажность ухудшает теплоотдачу организма.`
      : 'Нет данных для расчёта');
  }

  // IAQ
  const iaq = calculateIAQ(
    sensorData.scd4x_co2,
    sensorData.pms_pm2_5,
    sensorData.pms_pm10,
    sensorData.ch2o_value
  );
  const iaqEl = document.querySelector('.calculated_value.iaq_index');
  const iaqCard = document.querySelector('[data-analytics="iaq"]');
  if (iaqEl) {
    iaqEl.innerHTML = iaq !== null ? iaq.toString() : '---';
    // Цветовая дифференциация
    iaqEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (iaq !== null) {
      if (iaq <= 50) iaqEl.classList.add('value-good');
      else if (iaq <= 150) iaqEl.classList.add('value-warning');
      else iaqEl.classList.add('value-critical');
    }
  }
  if (iaqCard) {
    iaqCard.setAttribute('title', iaq !== null
      ? `Индекс качества воздуха: ${iaq}\n\n` +
        `Комплексная оценка по CO2, PM2.5, PM10, формальдегиду:\n` +
        `0-50: Отлично - воздух чистый\n` +
        `51-100: Нормально - приемлемо\n` +
        `101-150: Посредственно - чувствительным людям стоит быть осторожнее\n` +
        `151-200: Плохо - вредно для здоровья\n` +
        `201-300: Очень плохо - опасно для всех\n` +
        `301-500: Опасно - чрезвычайная ситуация`
      : 'Нет данных для расчёта');
  }

  // Барометрическая тенденция
  const pressureTrend = calculatePressureTrend(sensorData.bme_pressure);
  const trendEl = document.querySelector('.calculated_value.pressure_trend');
  const trendCard = document.querySelector('[data-analytics="pressure_trend"]');
  if (trendEl) {
    trendEl.innerHTML = pressureTrend !== null ? pressureTrend.text : '---';
    // Цветовая дифференциация
    trendEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (pressureTrend !== null) {
      if (pressureTrend.text.includes('Стабильно')) trendEl.classList.add('value-good');
      else if (pressureTrend.text.includes('Растёт') || pressureTrend.text.includes('Падает')) trendEl.classList.add('value-warning');
    }
  }
  if (trendCard) {
    trendCard.setAttribute('title', pressureTrend !== null
      ? `Барометрическая тенденция за 3 часа\n\n` +
        `Народная примета:\n` +
        `Растёт → к улучшению погоды (ясно, сухо)\n` +
        `Падает → к ухудшению (дождь, ветер)\n` +
        `Стабильно → погода без изменений\n\n` +
        `Быстрое изменение (>2 гПа/3ч) указывает на приближение фронта.`
      : 'Нет данных для расчёта. Требуется минимум 3 часа данных.');
  }
  
  // УФ-рекомендация
  const uvRec = getUVRecommendation(sensorData.veml_uv);
  const uvRecEl = document.querySelector('.calculated_value.uv_recommendation');
  const uvCard = document.querySelector('[data-analytics="uv"]');
  if (uvRecEl && uvRec) {
    uvRecEl.innerHTML = uvRec.text;
    uvRecEl.className = 'calculated_value uv_recommendation ' + uvRec.class;
    // Цветовая дифференциация
    uvRecEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (sensorData.veml_uv !== null) {
      if (sensorData.veml_uv <= 5) uvRecEl.classList.add('value-good');
      else if (sensorData.veml_uv <= 7) uvRecEl.classList.add('value-warning');
      else uvRecEl.classList.add('value-critical');
    }
  }
  if (uvCard) {
    uvCard.setAttribute('title', sensorData.veml_uv !== null
      ? `УФ-индекс: ${sensorData.veml_uv}\n\n` +
        `Рекомендации ВОЗ:\n` +
        `0-2: Безопасно, защита не нужна\n` +
        `3-5: Очки + крем SPF 15+\n` +
        `6-7: Крем SPF 30+, одежда, шляпа\n` +
        `8-10: Избегать солнца 11:00-16:00\n` +
        `11+: Опасно! Оставаться в помещении\n\n` +
        `УФ-излучение вызывает ожоги и старение кожи.`
      : 'Нет данных УФ-датчика');
  }

  // Индекс комфорта
  const comfort = calculateComfortIndex(
    sensorData.htu_temperature,
    sensorData.htu_humidity,
    sensorData.scd4x_co2
  );
  const comfortEl = document.querySelector('.calculated_value.comfort_index');
  const comfortCard = document.querySelector('[data-analytics="comfort"]');
  if (comfortEl) {
    comfortEl.innerHTML = comfort !== null ? comfort.toString() : '---';
    // Цветовая дифференциация
    comfortEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (comfort !== null) {
      if (comfort >= 80) comfortEl.classList.add('value-good');
      else if (comfort >= 50) comfortEl.classList.add('value-warning');
      else comfortEl.classList.add('value-critical');
    }
  }
  if (comfortCard) {
    comfortCard.setAttribute('title', comfort !== null
      ? `Индекс комфорта: ${comfort}/100\n\n` +
        `Оценка по трём параметрам:\n` +
        `Температура (оптимум 20-23°C)\n` +
        `Влажность (оптимум 40-60%)\n` +
        `CO2 (оптимум <800 ppm)\n\n` +
        `80-100: Идеально\n` +
        `50-79: Приемлемо\n` +
        `<50: Нужно улучшить условия`
      : 'Нет данных для расчёта');
  }

  // Рекомендации по проветриванию
  const ventRec = getVentilationRecommendation();
  const ventRecEl = document.querySelector('.calculated_value.ventilation_recommendation');
  const ventCard = document.querySelector('[data-analytics="ventilation"]');
  if (ventRecEl && ventRec) {
    ventRecEl.innerHTML = ventRec.text;
    ventRecEl.className = 'calculated_value ventilation_recommendation ' + ventRec.class;
    // Цветовая дифференциация
    ventRecEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (ventRec.text.includes('Всё в норме') || ventRec.text.includes('Проветрить')) ventRecEl.classList.add('value-good');
    else if (ventRec.text.includes('Высокий CO2') || ventRec.text.includes('Закрыть')) ventRecEl.classList.add('value-warning');
    else ventRecEl.classList.add('value-critical');
  }
  if (ventCard) {
    ventCard.setAttribute('title', `Рекомендации по проветриванию\n\n` +
      `Анализируются:\n` +
      `Уровень CO2 в помещении\n` +
      `Разница температур внутри/снаружи\n` +
      `УФ-индекс (чтобы не запускать жару)\n` +
      `Время суток\n\n` +
      `Правильное проветривание:\n` +
      `5-10 минут каждые 2-3 часа\n` +
      `Лучше сквозное проветривание\n` +
      `Ночью можно оставить микропроветривание`);
  }

  // Абсолютная влажность
  const absHumidity = calculateAbsoluteHumidity(sensorData.htu_temperature, sensorData.htu_humidity);
  const absHumidityEl = document.querySelector('.calculated_value.absolute_humidity');
  const absHumidityCard = document.querySelector('[data-analytics="absolute_humidity"]');
  if (absHumidityEl) {
    absHumidityEl.innerHTML = absHumidity !== null ? absHumidity.toFixed(1) : '---';
    // Цветовая дифференциация
    absHumidityEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (absHumidity !== null) {
      if (absHumidity >= 8 && absHumidity <= 12) absHumidityEl.classList.add('value-good');
      else if (absHumidity < 6 || absHumidity > 14) absHumidityEl.classList.add('value-warning');
      else absHumidityEl.classList.add('value-info');
    }
  }
  if (absHumidityCard) {
    absHumidityCard.setAttribute('title', absHumidity !== null
      ? `Абсолютная влажность: ${absHumidity.toFixed(1)} г/м³\n\n` +
        `Количество водяного пара в 1 м³ воздуха.\n\n` +
        `Нормы для помещений:\n` +
        `8-10 г/м³: Комфортно\n` +
        `10-12 г/м³: Нормально\n` +
        `>14 г/м³: Душно\n` +
        `<6 г/м³: Сухо (риск для дыхательных путей)\n\n` +
        `Зимой в отапливаемых помещениях обычно 3-5 г/м³.`
      : 'Нет данных для расчёта');
  }

  // Рекомендации по освещению
  const lightRec = getLightingRecommendation(sensorData.bh1750_lighting);
  const lightRecEl = document.querySelector('.calculated_value.lighting_recommendation');
  const lightCard = document.querySelector('[data-analytics="lighting"]');
  if (lightRecEl && lightRec) {
    lightRecEl.innerHTML = lightRec.text;
    lightRecEl.className = 'calculated_value lighting_recommendation ' + lightRec.class;
    // Цветовая дифференциация
    lightRecEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (lightRec.text.includes('Хорошее') || lightRec.text.includes('Нормально')) lightRecEl.classList.add('value-good');
    else if (lightRec.text.includes('Недостаточно') || lightRec.text.includes('Вечернее')) lightRecEl.classList.add('value-warning');
    else lightRecEl.classList.add('value-info');
  }
  if (lightCard) {
    lightCard.setAttribute('title', sensorData.bh1750_lighting !== null
      ? `Освещённость: ${sensorData.bh1750_lighting.toFixed(0)} лк\n\n` +
        `Нормы освещённости:\n` +
        `• 300-500 лк: Офисная работа\n` +
        `• 150-300 лк: Жилая комната\n` +
        `• 50-150 лк: Коридор, кухня\n` +
        `• <50 лк: Вечернее освещение\n\n` +
        `Недостаток света вызывает усталость глаз\n` +
        `и снижает продуктивность.`
      : 'Нет данных датчика освещения');
  }

  // 11. Дефицит точки росы
  const dewPointDeficit = calculateDewPointDeficit(sensorData.htu_temperature, dewPoint);
  const dewPointDeficitEl = document.querySelector('.calculated_value.dew_point_deficit');
  const dewPointDeficitCard = document.querySelector('[data-analytics="dew_point_deficit"]');
  if (dewPointDeficitEl) {
    dewPointDeficitEl.innerHTML = dewPointDeficit !== null ? dewPointDeficit.toFixed(1) : '---';
    dewPointDeficitEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (dewPointDeficit !== null) {
      if (dewPointDeficit > 5 && dewPointDeficit <= 10) dewPointDeficitEl.classList.add('value-good');
      else if (dewPointDeficit > 10) dewPointDeficitEl.classList.add('value-warning');
      else dewPointDeficitEl.classList.add('value-info');
    }
  }
  if (dewPointDeficitCard) {
    const deficitDesc = getDewPointDeficitDescription(dewPointDeficit);
    dewPointDeficitCard.setAttribute('title', dewPointDeficit !== null
      ? `Дефицит точки росы: ${dewPointDeficit.toFixed(1)}°C\n\n` +
        `Разница между температурой и точкой росы.\n\n` +
        `>15°C: Очень сухо\n10-15°C: Сухо\n5-10°C: Норма\n<5°C: Влажно\n\n` +
        `Низкий дефицит указывает на высокую влажность.`
      : 'Нет данных для расчёта');
  }

  // 12. Энтальпия воздуха
  const enthalpy = calculateEnthalpy(sensorData.htu_temperature, sensorData.htu_humidity);
  const enthalpyEl = document.querySelector('.calculated_value.enthalpy');
  const enthalpyCard = document.querySelector('[data-analytics="enthalpy"]');
  if (enthalpyEl) {
    enthalpyEl.innerHTML = enthalpy !== null ? enthalpy.toFixed(1) : '---';
    enthalpyEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (enthalpy !== null) {
      if (enthalpy >= 40 && enthalpy <= 55) enthalpyEl.classList.add('value-good');
      else if (enthalpy < 25 || enthalpy > 70) enthalpyEl.classList.add('value-critical');
      else enthalpyEl.classList.add('value-warning');
    }
  }
  if (enthalpyCard) {
    const enthalpyDesc = getEnthalpyDescription(enthalpy);
    enthalpyCard.setAttribute('title', enthalpy !== null
      ? `Энтальпия воздуха: ${enthalpy.toFixed(1)} кДж/кг\n\n` +
        `Полная энергия влажного воздуха.\n\n` +
        `<25: Холодный\n25-40: Прохладный\n40-55: Комфортный\n55-70: Тёплый\n>70: Горячий\n\n` +
        `Используется для расчёта HVAC систем.`
      : 'Нет данных для расчёта');
  }

  // 13. Риск плесени
  const moldRisk = calculateMoldRisk(sensorData.htu_humidity, dewPoint, sensorData.htu_temperature);
  const moldRiskEl = document.querySelector('.calculated_value.mold_risk');
  const moldRiskCard = document.querySelector('[data-analytics="mold_risk"]');
  if (moldRiskEl) {
    const moldDesc = getMoldRiskDescription(moldRisk);
    moldRiskEl.innerHTML = moldRisk !== null ? `${moldRisk}%` : '---';
    moldRiskEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (moldRisk !== null) {
      if (moldRisk < 20) moldRiskEl.classList.add('value-good');
      else if (moldRisk < 40) moldRiskEl.classList.add('value-warning');
      else moldRiskEl.classList.add('value-critical');
    }
  }
  if (moldRiskCard) {
    moldRiskCard.setAttribute('title', moldRisk !== null
      ? `Риск плесени: ${moldRisk}%\n\n` +
        `Оценивается по влажности, точке росы и температуре.\n\n` +
        `<20%: Низкий риск\n20-40%: Умеренный\n40-60%: Высокий\n>60%: Опасный\n\n` +
        `Для снижения риска: проветривайте, используйте осушитель.`
      : 'Нет данных для расчёта');
  }

  // 14. Статическое электричество
  const staticRisk = calculateStaticElectricityRisk(sensorData.htu_humidity);
  const staticRiskEl = document.querySelector('.calculated_value.static_electricity');
  const staticRiskCard = document.querySelector('[data-analytics="static_electricity"]');
  if (staticRiskEl && staticRisk) {
    staticRiskEl.innerHTML = staticRisk.text;
    staticRiskEl.className = 'calculated_value static_electricity ' + staticRisk.class;
    staticRiskEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (staticRisk.class === 'static-none' || staticRisk.class === 'static-low') staticRiskEl.classList.add('value-good');
    else if (staticRisk.class === 'static-medium') staticRiskEl.classList.add('value-warning');
    else staticRiskEl.classList.add('value-critical');
  }
  if (staticRiskCard) {
    staticRiskCard.setAttribute('title', `Риск статического электричества\n\n` +
      `Зависит от влажности воздуха:\n` +
      `<20%: Высокий риск разрядов\n20-30%: Средний риск\n30-40%: Низкий риск\n>40%: Риска нет\n\n` +
      `Для снижения: увлажняйте воздух, используйте антистатик.`
    );
  }

  // 15. Ощущаемая температура
  const feelsLike = calculateFeelsLike(sensorData.htu_temperature, sensorData.htu_humidity);
  const feelsLikeEl = document.querySelector('.calculated_value.feels_like');
  const feelsLikeCard = document.querySelector('[data-analytics="feels_like"]');
  if (feelsLikeEl) {
    feelsLikeEl.innerHTML = feelsLike !== null ? feelsLike.toFixed(1) : '---';
    feelsLikeEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (feelsLike !== null) {
      if (feelsLike >= 18 && feelsLike <= 26) feelsLikeEl.classList.add('value-good');
      else if (feelsLike < 10 || feelsLike > 32) feelsLikeEl.classList.add('value-critical');
      else feelsLikeEl.classList.add('value-warning');
    }
  }
  if (feelsLikeCard) {
    feelsLikeCard.setAttribute('title', feelsLike !== null
      ? `Ощущается как: ${feelsLike.toFixed(1)}°C\n\n` +
        `Температура, которую ощущает человек.\n\n` +
        `Учитывает:\n` +
        `• При t>27°C: индекс жары (влажность)\n` +
        `• При t<10°C: wind chill (охлаждение)\n` +
        `• При 10-27°C: фактическая температура`
      : 'Нет данных для расчёта');
  }

  // 16. Время до проветривания
  const ventTime = calculateVentilationTime(sensorData.scd4x_co2);
  const ventTimeEl = document.querySelector('.calculated_value.ventilation_time');
  const ventTimeCard = document.querySelector('[data-analytics="ventilation_time"]');
  if (ventTimeEl) {
    const ventTimeDesc = getVentilationTimeDescription(ventTime);
    ventTimeEl.innerHTML = ventTime !== null ? (ventTimeDesc ? ventTimeDesc.text : Math.round(ventTime)) : '---';
    ventTimeEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (ventTime !== null) {
      if (ventTime > 30) ventTimeEl.classList.add('value-good');
      else if (ventTime > 15) ventTimeEl.classList.add('value-warning');
      else ventTimeEl.classList.add('value-critical');
    }
  }
  if (ventTimeCard) {
    ventTimeCard.setAttribute('title', ventTime !== null
      ? `Прогноз до проветривания: ${Math.round(ventTime)} мин\n\n` +
        `Время до достижения CO2 = 1000 ppm.\n\n` +
        `Рассчитывается по текущему уровню CO2\n` +
        `и скорости его роста за последние 30 мин.\n\n` +
        `0 мин = Пора проветривать!`
      : 'Нет данных для расчёта');
  }

  // 17. Качество сна
  const sleepQuality = calculateSleepQuality(sensorData.htu_temperature, sensorData.htu_humidity, sensorData.scd4x_co2);
  const sleepQualityEl = document.querySelector('.calculated_value.sleep_quality');
  const sleepQualityCard = document.querySelector('[data-analytics="sleep_quality"]');
  if (sleepQualityEl) {
    const sleepDesc = getSleepQualityDescription(sleepQuality);
    sleepQualityEl.innerHTML = sleepQuality !== null ? `${sleepQuality}%` : '---';
    sleepQualityEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (sleepQuality !== null) {
      if (sleepQuality >= 70) sleepQualityEl.classList.add('value-good');
      else if (sleepQuality >= 50) sleepQualityEl.classList.add('value-warning');
      else sleepQualityEl.classList.add('value-critical');
    }
  }
  if (sleepQualityCard) {
    sleepQualityCard.setAttribute('title', sleepQuality !== null
      ? `Качество сна: ${sleepQuality}%\n\n` +
        `Оценка условий для сна.\n\n` +
        `Оптимум:\n` +
        `• Температура: 18-20°C\n` +
        `• Влажность: 40-60%\n` +
        `• CO2: <800 ppm\n\n` +
        `>85%: Отлично\n70-85%: Хорошо\n50-70%: Нормально\n<50%: Плохо`
      : 'Нет данных для расчёта');
  }

  // 18. AQI по PM2.5
  const pm25Aqi = calculatePM25AQI(sensorData.pms_pm2_5);
  const pm25AqiEl = document.querySelector('.calculated_value.pm25_aqi');
  const pm25AqiCard = document.querySelector('[data-analytics="pm25_aqi"]');
  if (pm25AqiEl) {
    const pm25Desc = getPM25AQIDescription(pm25Aqi);
    pm25AqiEl.innerHTML = pm25Aqi !== null ? pm25Aqi.toString() : '---';
    // Применяем класс цвета из описания
    pm25AqiEl.className = 'calculated_value pm25_aqi';
    if (pm25Desc) {
      pm25AqiEl.classList.add(pm25Desc.class);
    }
    // Цветовая дифференциация для fallback
    pm25AqiEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (pm25Aqi !== null) {
      if (pm25Aqi <= 50) pm25AqiEl.classList.add('value-good');
      else if (pm25Aqi <= 150) pm25AqiEl.classList.add('value-warning');
      else pm25AqiEl.classList.add('value-critical');
    }
    // Отладка
    console.log('AQI PM2.5:', pm25Aqi, 'PM2.5:', sensorData.pms_pm2_5);
  }
  if (pm25AqiCard) {
    const pm25Val = sensorData.pms_pm2_5;
    pm25AqiCard.setAttribute('title', pm25Aqi !== null
      ? `AQI PM2.5: ${pm25Aqi}\n\n` +
        `Индекс качества воздуха по мелким частицам.\n\n` +
        `Шкала ВОЗ (мкг/м³):\n` +
        `0-50: Отлично (<15)\n51-100: Нормально (15-25)\n` +
        `101-150: Посредственно (25-50)\n` +
        `151-200: Плохо (50-100)\n` +
        `201-300: Очень плохо (100-200)\n` +
        `>300: Опасно (>200)\n\n` +
        `PM2.5: ${pm25Val !== null ? pm25Val.toFixed(1) : '---'} мкг/м³\n\n` +
        `PM2.5 проникает глубоко в лёгкие.`
      : 'Нет данных для расчёта');
  }

  // ============================================================
  // === ОБНОВЛЕНИЕ НОВЫХ ПОКАЗАТЕЛЕЙ (10) =======================
  // ============================================================

  // 19. Точка замерзания
  const freezingPoint = calculateFreezingPoint(sensorData.htu_temperature, dewPoint);
  const freezingPointEl = document.querySelector('.calculated_value.freezing_point');
  const freezingPointCard = document.querySelector('[data-analytics="freezing_point"]');
  if (freezingPointEl) {
    freezingPointEl.innerHTML = freezingPoint !== null ? freezingPoint.toFixed(1) : '---';
    freezingPointEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (freezingPoint !== null) {
      if (freezingPoint > 0) freezingPointEl.classList.add('value-good');
      else if (freezingPoint > -5) freezingPointEl.classList.add('value-warning');
      else freezingPointEl.classList.add('value-critical');
    }
  }
  if (freezingPointCard) {
    freezingPointCard.setAttribute('title', freezingPoint !== null
      ? `Точка замерзания: ${freezingPoint.toFixed(1)}°C\n\n` +
        `Температура, при которой образуется иней.\n\n` +
        `≤-5°C: Риск инея\n-5...0°C: Около нуля\n>0°C: Без риска\n\n` +
        `Важно для растений и уличных работ.`
      : 'Нет данных для расчёта');
  }

  // 20. Индекс загрязнения
  const pollutionIndex = calculatePollutionIndex(sensorData.pms_pm2_5, sensorData.pms_pm10);
  const pollutionIndexEl = document.querySelector('.calculated_value.pollution_index');
  const pollutionIndexCard = document.querySelector('[data-analytics="pollution_index"]');
  if (pollutionIndexEl) {
    const pollDesc = getPollutionIndexDescription(pollutionIndex);
    pollutionIndexEl.innerHTML = pollutionIndex !== null ? pollutionIndex.toFixed(0) : '---';
    pollutionIndexEl.className = 'calculated_value pollution_index';
    if (pollDesc) pollutionIndexEl.classList.add(pollDesc.class);
    // Отладка
    console.log('Pollution Index:', pollutionIndex, 'PM2.5:', sensorData.pms_pm2_5, 'PM10:', sensorData.pms_pm10);
  }
  if (pollutionIndexCard) {
    pollutionIndexCard.setAttribute('title', pollutionIndex !== null
      ? `Индекс загрязнения: ${pollutionIndex.toFixed(0)}\n\n` +
        `Комплексная оценка загрязнения частицами.\n\n` +
        `Формула: PM2.5*0.7 + PM10*0.3\n\n` +
        `≤25: Чисто\n26-50: Нормально\n51-75: Загрязнено\n` +
        `76-100: Плохо\n>100: Опасно`
      : 'Нет данных для расчёта');
  }

  // 21. Время безопасного пребывания
  const safeTime = calculateSafeExposureTime(sensorData.scd4x_co2);
  const safeTimeEl = document.querySelector('.calculated_value.safe_exposure_time');
  const safeTimeCard = document.querySelector('[data-analytics="safe_exposure_time"]');
  if (safeTimeEl) {
    const safeTimeDesc = getSafeExposureTimeDescription(safeTime);
    safeTimeEl.innerHTML = safeTime !== null ? (safeTimeDesc ? safeTimeDesc.text : Math.round(safeTime)) : '---';
    safeTimeEl.classList.remove('value-good', 'value-warning', 'value-critical');
    if (safeTime !== null) {
      if (safeTime > 60) safeTimeEl.classList.add('value-good');
      else if (safeTime > 30) safeTimeEl.classList.add('value-warning');
      else safeTimeEl.classList.add('value-critical');
    }
  }
  if (safeTimeCard) {
    safeTimeCard.setAttribute('title', safeTime !== null
      ? `Время до CO2=1400 ppm: ~${Math.round(safeTime)} мин\n\n` +
        `Прогноз безопасного пребывания в помещении.\n\n` +
        `Рассчитывается по текущему CO2 и скорости роста.\n\n` +
        `0 мин = Пора проветривать!\n>60 мин = Запас времени есть.`
      : 'Нет данных для расчёта');
  }

  // 22. Эффективность проветривания
  const ventEfficiency = calculateVentilationEfficiency();
  const ventEfficiencyEl = document.querySelector('.calculated_value.ventilation_efficiency');
  const ventEfficiencyCard = document.querySelector('[data-analytics="ventilation_efficiency"]');
  if (ventEfficiencyEl) {
    const ventEffDesc = getVentilationEfficiencyDescription(ventEfficiency);
    ventEfficiencyEl.innerHTML = ventEfficiency !== null ? ventEfficiency.toFixed(1) : '---';
    ventEfficiencyEl.className = 'calculated_value ventilation_efficiency';
    if (ventEffDesc) ventEfficiencyEl.classList.add(ventEffDesc.class);
  }
  if (ventEfficiencyCard) {
    ventEfficiencyCard.setAttribute('title', ventEfficiency !== null
      ? `Эффективность проветривания: ${ventEfficiency.toFixed(1)} ppm/мин\n\n` +
        `Скорость изменения CO2 за последние 30 мин.\n\n` +
        `>10: Отличная вентиляция\n5-10: Хорошая\n0-5: Слабая\n` +
        `<0: CO2 растёт (нет вентиляции)`
      : 'Нет данных для расчёта. Нужна история CO2.');
  }

  // 23. Индекс духоты
  const discomfortIndex = calculateDiscomfortIndex(sensorData.htu_temperature, sensorData.htu_humidity);
  const discomfortIndexEl = document.querySelector('.calculated_value.discomfort_index');
  const discomfortIndexCard = document.querySelector('[data-analytics="discomfort_index"]');
  if (discomfortIndexEl) {
    const discDesc = getDiscomfortIndexDescription(discomfortIndex);
    discomfortIndexEl.innerHTML = discomfortIndex !== null ? discomfortIndex.toFixed(1) : '---';
    discomfortIndexEl.className = 'calculated_value discomfort_index';
    if (discDesc) discomfortIndexEl.classList.add(discDesc.class);
  }
  if (discomfortIndexCard) {
    discomfortIndexCard.setAttribute('title', discomfortIndex !== null
      ? `Индекс духоты: ${discomfortIndex.toFixed(1)}\n\n` +
        `Упрощённый индекс дискомфорта.\n\n` +
        `Формула: Temp + 0.33*Humidity - 0.7\n\n` +
        `<21: Холодно\n21-24: Комфортно\n24-27: Тепло\n` +
        `27-30: Душно\n>30: Жарко`
      : 'Нет данных для расчёта');
  }

  // 24. Риск аллергии
  const allergyRisk = calculateAllergyRisk(sensorData.pms_pm2_5, sensorData.pms_pm10, sensorData.htu_humidity);
  const allergyRiskEl = document.querySelector('.calculated_value.allergy_risk');
  const allergyRiskCard = document.querySelector('[data-analytics="allergy_risk"]');
  if (allergyRiskEl) {
    const allergyDesc = getAllergyRiskDescription(allergyRisk);
    allergyRiskEl.innerHTML = allergyRisk !== null ? allergyRisk.toFixed(0) : '---';
    allergyRiskEl.className = 'calculated_value allergy_risk';
    if (allergyDesc) allergyRiskEl.classList.add(allergyDesc.class);
    // Отладка
    console.log('Allergy Risk:', allergyRisk, 'PM2.5:', sensorData.pms_pm2_5, 'PM10:', sensorData.pms_pm10, 'Hum:', sensorData.htu_humidity);
  }
  if (allergyRiskCard) {
    allergyRiskCard.setAttribute('title', allergyRisk !== null
      ? `Риск аллергии: ${allergyRisk.toFixed(0)}%\n\n` +
        `Комбинированный риск по частицам и влажности.\n\n` +
        `Учитывает:\n` +
        `• PM2.5 и PM10 (пыльца, пыль)\n` +
        `• Высокая влажность (плесень, клещи)\n` +
        `• Низкая влажность (раздражение)\n\n` +
        `<20%: Низкий\n20-40%: Умеренный\n40-60%: Высокий\n>60%: Опасный`
      : 'Нет данных для расчёта');
  }

  // 25. Оптимальное время для сна
  const optimalSleep = calculateOptimalSleepTime(
    sensorData.htu_temperature,
    sensorData.htu_humidity,
    sensorData.scd4x_co2,
    sensorData.microphone_noise
  );
  const optimalSleepEl = document.querySelector('.calculated_value.optimal_sleep_time');
  const optimalSleepCard = document.querySelector('[data-analytics="optimal_sleep_time"]');
  if (optimalSleepEl && optimalSleep) {
    optimalSleepEl.innerHTML = optimalSleep.text;
    optimalSleepEl.className = 'calculated_value optimal_sleep_time ' + optimalSleep.class;
  }
  if (optimalSleepCard) {
    optimalSleepCard.setAttribute('title', optimalSleep
      ? `Оптимальное время для сна\n\n` +
        `Оценка условий: ${optimalSleep.text}\n\n` +
        `Учитываются:\n` +
        `• Температура (оптимум: 18-20°C)\n` +
        `• Влажность (оптимум: 40-60%)\n` +
        `• CO2 (оптимум: <800 ppm)\n` +
        `• Шум (оптимум: <35 дБ)`
      : 'Нет данных для расчёта');
  }

  // 26. Индекс продуктивности
  const productivity = calculateProductivityIndex(sensorData.scd4x_co2, sensorData.htu_temperature, sensorData.bh1750_lighting);
  const productivityEl = document.querySelector('.calculated_value.productivity_index');
  const productivityCard = document.querySelector('[data-analytics="productivity_index"]');
  if (productivityEl) {
    const prodDesc = getProductivityIndexDescription(productivity);
    productivityEl.innerHTML = productivity !== null ? productivity.toFixed(0) : '---';
    productivityEl.className = 'calculated_value productivity_index';
    if (prodDesc) productivityEl.classList.add(prodDesc.class);
  }
  if (productivityCard) {
    productivityCard.setAttribute('title', productivity !== null
      ? `Индекс продуктивности: ${productivity.toFixed(0)}%\n\n` +
        `Влияние условий на работоспособность.\n\n` +
        `Учитываются:\n` +
        `• CO2 (влияет на концентрацию)\n` +
        `• Температура (комфорт)\n` +
        `• Освещение (продуктивность)\n\n` +
        `≥85%: Отлично\n70-84%: Хорошо\n50-69%: Нормально\n` +
        `30-49%: Низко\n<30%: Критично`
      : 'Нет данных для расчёта');
  }

  // 27. Баланс кислорода
  const oxygenBalance = calculateOxygenBalance(sensorData.scd4x_co2, sensorData.bme_pressure);
  const oxygenBalanceEl = document.querySelector('.calculated_value.oxygen_balance');
  const oxygenBalanceCard = document.querySelector('[data-analytics="oxygen_balance"]');
  if (oxygenBalanceEl) {
    const o2Desc = getOxygenBalanceDescription(oxygenBalance);
    oxygenBalanceEl.innerHTML = oxygenBalance !== null ? oxygenBalance.toFixed(0) : '---';
    oxygenBalanceEl.className = 'calculated_value oxygen_balance';
    if (o2Desc) oxygenBalanceEl.classList.add(o2Desc.class);
  }
  if (oxygenBalanceCard) {
    oxygenBalanceCard.setAttribute('title', oxygenBalance !== null
      ? `Баланс кислорода: ${oxygenBalance.toFixed(0)}%\n\n` +
        `Косвенная оценка уровня O2.\n\n` +
        `Рассчитывается по:\n` +
        `• CO2 (при росте CO2 доля O2 падает)\n` +
        `• Давлению (влияет на парциальное давление)\n\n` +
        `≥95%: Норма\n85-94%: Снижен\n70-84%: Низкий\n<70%: Гипоксия`
      : 'Нет данных для расчёта');
  }

  // 28. Тепловая нагрузка (WBGT)
  const heatStress = calculateHeatStressIndex(sensorData.htu_temperature, sensorData.htu_humidity, sensorData.bh1750_lighting);
  const heatStressEl = document.querySelector('.calculated_value.heat_stress_index');
  const heatStressCard = document.querySelector('[data-analytics="heat_stress_index"]');
  if (heatStressEl) {
    const heatDesc = getHeatStressIndexDescription(heatStress);
    heatStressEl.innerHTML = heatStress !== null ? heatStress.toFixed(1) : '---';
    heatStressEl.className = 'calculated_value heat_stress_index';
    if (heatDesc) heatStressEl.classList.add(heatDesc.class);
  }
  if (heatStressCard) {
    heatStressCard.setAttribute('title', heatStress !== null
      ? `Тепловая нагрузка (WBGT): ${heatStress.toFixed(1)}°C\n\n` +
        `Wet Bulb Globe Temperature - индекс теплового стресса.\n\n` +
        `Формула: 0.7*Tw + 0.2*Tg + 0.1*Ta\n\n` +
        `<18°C: Холодно\n18-24°C: Комфортно\n24-28°C: Тепло\n` +
        `28-32°C: Жарко\n>32°C: Опасно\n\n` +
        `Важно для спортивных нагрузок и физических работ.`
      : 'Нет данных для расчёта');
  }

}

// Форматирование значения с плавающей точкой
function formatValue(rawValue, decimals = 1) {
  if (rawValue === null || rawValue === undefined || isNaN(rawValue)) return '---';
  let val = rawValue;  // Данные приходят в оригинальных единицах (без масштабирования)
  if (decimals === 0) return Math.round(val).toString();
  return val.toFixed(decimals);
}

// Обновление значения датчика с цветовой индикацией
function updateSensorValue(type, value) {
  const el = document.querySelector(`.measured_value.${type}`);
  if (!el) return;
  
  el.innerHTML = value;
  
  // Получаем пороги для этого датчика
  const thresholds = SENSOR_THRESHOLDS[type];
  if (!thresholds) return;
  
  // Удаляем старые классы статуса
  el.classList.remove('status-ok', 'status-warning', 'status-critical');
  
  // Определяем и применяем новый статус
  const numValue = parseFloat(value);
  if (!isNaN(numValue)) {
    const status = getStatus(numValue, thresholds);
    if (status) {
      el.classList.add(`status-${status}`);
    }
  }
}

// Обработка WebSocket сообщений
function handleWebSocketMessage(event) {
  try {
    const obj = JSON.parse(event.data);
    const type = obj.type;
    const rawValue = parseFloat(obj.value);

    console.log('[WS] Received:', type, '=', rawValue);  // ← Добавили логирование

    // Системные данные
    if (type === 'esp32_cpu_freq') {
      const el = document.querySelector('.dungen_value.esp32_cpu_freq');
      if (el) el.innerHTML = (rawValue / 1000000).toFixed(0);
      return;
    }
    if (type === 'esp32_cpu_temp') {
      const el = document.querySelector('.dungen_value.esp32_cpu_temp');
      if (el) el.innerHTML = rawValue.toFixed(1);
      return;
    }
    if (type === 'esp32_free_heap') {
      const el = document.querySelector('.dungen_value.esp32_free_heap');
      if (el) {
        const heapKB = Math.round(rawValue / 1024);
        el.innerHTML = heapKB;
        // Цветовая индикация свободной памяти
        el.classList.remove('good', 'warning', 'critical');
        if (heapKB > 200) el.classList.add('good');
        else if (heapKB > 100) el.classList.add('warning');
        else el.classList.add('critical');
      }
      return;
    }
    if (type === 'esp32_cpu_load') {
      const el = document.querySelector('.dungen_value.esp32_cpu_load');
      if (el) {
        const load = Math.round(rawValue);
        el.innerHTML = load;
        // Цветовая индикация загрузки CPU
        el.classList.remove('low', 'medium', 'high');
        if (load < 50) el.classList.add('low');
        else if (load < 80) el.classList.add('medium');
        else el.classList.add('high');
      }
      return;
    }
    if (type === 'wifi_rssi') {
      const el = document.querySelector('.dungen_value.wifi_rssi');
      if (el) {
        const rssi = Math.round(rawValue);
        el.innerHTML = rssi;
        // Цветовая индикация качества сигнала
        el.classList.remove('excellent', 'good', 'fair', 'weak');
        if (rssi >= -50) el.classList.add('excellent');
        else if (rssi >= -60) el.classList.add('good');
        else if (rssi >= -70) el.classList.add('fair');
        else el.classList.add('weak');
      }
      return;
    }

    // Датчики - форматирование и отображение
    const sensors = {
      'bme_temperature': { class: 'bme_temperature', suffix: '°C', div: 1, decimals: 1 },
      'bme_pressure': { class: 'bme_pressure', suffix: 'гПа', div: 1, decimals: 1 },
      'bme_humidity': { class: 'bme_humidity', suffix: '%', div: 1, decimals: 1 },
      'htu_temperature': { class: 'htu_temperature', suffix: '°C', div: 1, decimals: 1 },
      'htu_humidity': { class: 'htu_humidity', suffix: '%', div: 1, decimals: 1 },
      'ms5611_pressure': { class: 'ms5611_pressure', suffix: 'гПа', div: 1, decimals: 1 },
      'ms5611_temperature': { class: 'ms5611_temperature', suffix: '°C', div: 1, decimals: 1 },
      'scd4x_co2': { class: 'scd4x_co2', suffix: 'ppm', div: 1, decimals: 0 },
      'scd4x_temperature': { class: 'scd4x_temperature', suffix: '°C', div: 1, decimals: 1 },
      'scd4x_humidity': { class: 'scd4x_humidity', suffix: '%', div: 1, decimals: 1 },
      'pms_pm1': { class: 'pms_pm1', suffix: 'мкг/м³', div: 1, decimals: 1 },
      'pms_pm2_5': { class: 'pms_pm2_5', suffix: 'мкг/м³', div: 1, decimals: 1 },
      'pms_pm10': { class: 'pms_pm10', suffix: 'мкг/м³', div: 1, decimals: 1 },
      'bh1750_lighting': { class: 'bh1750_lighting', suffix: 'лк', div: 1, decimals: 1 },
      'veml_uv': { class: 'veml_uv', suffix: '', div: 1, decimals: 0 },
      'ch2o_value': { class: 'ch2o_value', suffix: 'мг/м³', div: 1, decimals: 3 },
      'microphone_noise': { class: 'microphone_noise', suffix: 'дБ', div: 1, decimals: 1 }
    };

    if (sensors[type]) {
      const s = sensors[type];
      const val = (rawValue / s.div).toFixed(s.decimals);
      updateSensorValue(s.class, val);

      // Сохраняем данные для расчётных показателей
      if (sensorData.hasOwnProperty(type)) {
        sensorData[type] = rawValue / s.div;
        console.log('Saved to sensorData:', type, '=', sensorData[type]);
      }

      // Автовыбор темы по освещению
      if (type === 'bh1750_lighting') {
        applyAutoTheme();
      }

      // Обновляем расчётные показатели после получения новых данных
      updateCalculatedValues();
    }
  } catch (e) {
    console.error('WebSocket message error:', e);
  }
}

// Инициализация WebSocket
function initWebSocket() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  
  Socket.onopen = function() {
    console.log('WebSocket connected');
    // Обновляем IP адрес после подключения
    const ipEl = document.querySelector('.dungen_value.esp32-ip-adress');
    if (ipEl) ipEl.innerHTML = window.location.hostname;
  };
  
  Socket.onmessage = function(event) {
    handleWebSocketMessage(event);
  };
  
  Socket.onerror = function(error) {
    console.error('WebSocket error:', error);
  };
  
  Socket.onclose = function() {
    console.log('WebSocket disconnected, reconnecting...');
    setTimeout(initWebSocket, 3000);
  };
}

function CurrentTime(){
  const now = new Date();
  let num = String(now.getHours()).length;
  if (num === 1) {
     HoursVal="0"+now.getHours();
  }
  else {
    HoursVal=now.getHours();
  }
  let curTime=HoursVal+":"+String(now.getMinutes()).padStart(2,'0')+":"+String(now.getSeconds()).padStart(2,'0');
  document.getElementsByClassName("dungen_value time")[0].innerHTML=curTime;
}

function CurrentDate(){
  const now = new Date();
  let day = String(now.getDate()).padStart(2, '0');
  let month = String(now.getMonth() + 1).padStart(2, '0');
  let year = now.getFullYear();
  let DateVal = day + "." + month + "." + year;
  document.getElementsByClassName("dungen_value date")[0].innerHTML = DateVal;
}

function autoDarkMode(){
  const saved = localStorage.getItem('darkMode');
  if (saved === 'auto') {
    // Автовыбор темы по освещению
    applyAutoTheme();
  } else if (saved === 'true') {
    document.body.classList.add('dark-mode');
    document.querySelector('.dark-mode-button').src = 'on_bubl.png';
  } else {
    document.body.classList.remove('dark-mode');
    document.querySelector('.dark-mode-button').src = 'off_bubl.png';
  }
}

function applyAutoTheme() {
  // Если тема была переключена вручную, не применяем автовыбор
  if (localStorage.getItem('darkModeManual') === 'true') return;
  
  // Автовыбор темы по освещению: < 300 лк = тёмная, >= 300 лк = светлая
  const lux = sensorData.bh1750_lighting;
  if (lux !== null && !isNaN(lux)) {
    const isDark = lux < 300;
    document.body.classList.toggle('dark-mode', isDark);
    document.querySelector('.dark-mode-button').src = isDark ? 'on_bubl.png' : 'off_bubl.png';
  }
}

function toggleDarkMode() {
  // Устанавливаем флаг ручного переключения темы
  localStorage.setItem('darkModeManual', 'true');
  
  document.body.classList.toggle('dark-mode');
  const isDark = document.body.classList.contains('dark-mode');
  localStorage.setItem('darkMode', isDark);
  if (isDark) {
    document.querySelector('.dark-mode-button').src = 'on_bubl.png';
  } else {
    document.querySelector('.dark-mode-button').src = 'off_bubl.png';
  }
}

function setTextColor() {
  if (document.getElementsByClassName("text  indoor_temp")[0]){
  document.getElementsByClassName("text  indoor_temp")[0].style.color="cyan";
}
}

// Запускаем обновление времени каждую секунду
setInterval(function() {
  CurrentTime();
  CurrentDate();
}, 1000);

// Вызываем сразу при загрузке

window.onload = function() {
  CurrentTime();
  CurrentDate();
  setTextColor();
  autoDarkMode();
  initWebSocket();
};