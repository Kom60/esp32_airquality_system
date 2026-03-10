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
  
  // Формальдегид вклад (0-100 баллов)
  if (ch2o !== null && !isNaN(ch2o)) {
    if (ch2o <= 0.08) aqi += Math.round((ch2o / 0.08) * 50);
    else aqi += Math.round(50 + Math.min((ch2o - 0.08) / 0.02, 50));
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
      ? `Изменение давления за 3 часа: ${pressureTrend.value > 0 ? '+' : ''}${pressureTrend.value.toFixed(1)} гПа\n\n` +
        `Народная примета:\n` +
        `Растёт -> к улучшению погоды (ясно, сухо)\n` +
        `Падает -> к ухудшению (дождь, ветер)\n` +
        `Стабильно -> погода без изменений\n\n` +
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
      'ch2o_value': { class: 'ch2o_value', suffix: 'ppm', div: 1, decimals: 3 },
      'microphone_noise': { class: 'microphone_noise', suffix: 'дБ', div: 1, decimals: 1 }
    };

    if (sensors[type]) {
      const s = sensors[type];
      const val = (rawValue / s.div).toFixed(s.decimals);
      updateSensorValue(s.class, val);

      // Сохраняем данные для расчётных показателей
      if (sensorData.hasOwnProperty(type)) {
        sensorData[type] = rawValue / s.div;
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
  // Автовыбор темы по освещению: < 300 лк = тёмная, >= 300 лк = светлая
  const lux = sensorData.bh1750_lighting;
  if (lux !== null && !isNaN(lux)) {
    const isDark = lux < 300;
    document.body.classList.toggle('dark-mode', isDark);
    document.querySelector('.dark-mode-button').src = isDark ? 'on_bubl.png' : 'off_bubl.png';
  }
}

function toggleDarkMode() {
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