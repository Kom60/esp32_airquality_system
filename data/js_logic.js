let CPU_temp;
let output = document.getElementsByClassName("dungen_value esp32-voltage")[0];
let Socket;

// === Пороговые значения для датчиков ===
// Все значения в "сырых" единицах (как приходят с ESP32), чтобы не конвертировать дважды
// Формат: { min: норма_мин, max: норма_макс, warnMin: предупреждение_мин, warnMax: предупреждение_макс }
const SENSOR_THRESHOLDS = {
  // Температура (значения *100, °C)
  'htu_temperature':      { min: 1800, max: 2600, warnMin: 1500, warnMax: 3000 },
  'ms5611_temperature':   { min: 1800, max: 2600, warnMin: 1500, warnMax: 3000 },
  'scd4x_temperature':    { min: 1800, max: 2600, warnMin: 1500, warnMax: 3000 },
  'bme_temperature':      { min: -1000, max: 4000, warnMin: -2000, warnMax: 5000 }, // уличные
  
  // Влажность (значения *100, %)
  'htu_humidity':         { min: 3000, max: 6000, warnMin: 2000, warnMax: 8000 },
  'scd4x_humidity':       { min: 3000, max: 6000, warnMin: 2000, warnMax: 8000 },
  'bme_humidity':         { min: 2000, max: 9000, warnMin: 1000, warnMax: 9500 },
  
  // Давление (гПа)
  'ms5611_pressure':      { min: 980, max: 1040, warnMin: 960, warnMax: 1060 },
  'bme_pressure':         { min: 980, max: 1040, warnMin: 960, warnMax: 1060 },
  
  // CO2 (значения *100, ppm)
  'scd4x_co2':            { min: 400, max: 1000, warnMin: 400, warnMax: 1400 },
  
  // PM частицы (значения *10, мкг/м³)
  'pms_pm1':              { min: 0, max: 150, warnMin: 0, warnMax: 350 },
  'pms_pm2_5':            { min: 0, max: 150, warnMin: 0, warnMax: 350 },
  'pms_pm10':             { min: 0, max: 450, warnMin: 0, warnMax: 1500 },
  
  // Освещённость (значения *10, люкс)
  'bh1750_lighting':      { min: 3000, max: 10000, warnMin: 1000, warnMax: 20000 },
  
  // Формальдегид (значения *10, ppm) - опасно уже при 0.1
  'ch2o_value':           { min: 0, max: 8, warnMin: 0, warnMax: 10 },
  
  // Шум (значения *10, дБ)
  'microphone_noise':     { min: 300, max: 550, warnMin: 200, warnMax: 700 },
  
  // УФ-индекс (целые числа)
  'veml_uv':              { min: 0, max: 5, warnMin: 0, warnMax: 8 }
};

/**
 * Определяет статус значения: 'ok' | 'warning' | 'critical'
 */
function getStatus(rawVal, thresholds) {
  if (rawVal === null || rawVal === undefined || isNaN(rawVal)) return null;
  if (rawVal < thresholds.warnMin || rawVal > thresholds.warnMax) return 'critical';
  if (rawVal < thresholds.min || rawVal > thresholds.max) return 'warning';
  return 'ok';
}

/**
 * Применяет цветовую индикацию к элементу датчика
 * @param {string} className - класс элемента (например, 'scd4x_co2')
 * @param {number} rawValue - "сырое" числовое значение с ESP32
 * @param {string} displayValue - уже отформатированная строка для отображения
 */
function applySensorStatus(className, rawValue, displayValue) {
  const elements = document.getElementsByClassName(`measured_value ${className}`);
  if (!elements.length) return;
  const el = elements[0];
  
  // Обновляем отображаемое значение
  el.innerHTML = displayValue;
  
  // Если нет порогов для этого датчика — выходим
  if (!SENSOR_THRESHOLDS[className]) return;
  
  // Удаляем старые классы статуса
  el.classList.remove('status-ok', 'status-warning', 'status-critical');
  
  // Определяем и применяем новый статус
  const status = getStatus(rawValue, SENSOR_THRESHOLDS[className]);
  if (status) {
    el.classList.add(`status-${status}`);
    // Добавляем подсказку
    const t = SENSOR_THRESHOLDS[className];
    const unit = className.includes('temperature') ? '°C' : 
                 className.includes('humidity') ? '%' :
                 className.includes('pressure') ? 'гПа' :
                 className.includes('co2') ? 'ppm' :
                 className.includes('pm') ? 'мкг/м³' :
                 className.includes('lighting') ? 'лк' :
                 className.includes('ch2o') ? 'ppm' :
                 className.includes('noise') ? 'дБ' :
                 className.includes('uv') ? 'индекс' : '';
    el.title = className.includes('pressure') ? 
    `Норма: ${t.min}–${t.max}${unit}\nДопустимо: ${t.warnMin}–${t.warnMax}${unit}` :
    `Норма: ${t.min/100}–${t.max/100}${unit}\nДопустимо: ${t.warnMin/100}–${t.warnMax/100}${unit}`;
  }
}

function init() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onmessage = function(event) {
    processCommand(event);
  };
  Socket.onopen = function() {
    console.log('WebSocket connected');
  };
  
  // Запускаем обновление времени каждую секунду
  setInterval(updateTime, 1000);
}

function updateTime() {
  const now = new Date();
  const timeEl = document.getElementsByClassName('dungen_value time')[0];
  const dateEl = document.getElementsByClassName('dungen_value date')[0];
  
  if (timeEl) {
    timeEl.innerHTML = now.getHours().toString().padStart(2, '0') + ':' + 
                       now.getMinutes().toString().padStart(2, '0') + ':' + 
                       now.getSeconds().toString().padStart(2, '0');
  }
  
  if (dateEl) {
    dateEl.innerHTML = now.getDate().toString().padStart(2, '0') + '.' + 
                       (now.getMonth() + 1).toString().padStart(2, '0') + '.' + 
                       now.getFullYear();
  }
}

function processCommand(event) {
  var obj = JSON.parse(event.data);
  var type = obj.type;
  var rawValue = parseFloat(obj.value); // используем parseFloat вместо parseInt
  
  // === Системные данные (без индикации) ===
  if (type === "cpu_voltage") {
    let val = rawValue / 100;
    output.innerHTML = val.toFixed(2);
    return;
  }
  if (type === "esp32_cpu_freq") {
    let val = rawValue / 1000000;
    document.getElementsByClassName("dungen_value esp32_cpu_freq")[0].innerHTML = val.toFixed(0);
    return;
  }
  if (type === "esp32_cpu_temp") {
    let val = rawValue / 100;
    let el = document.getElementsByClassName("dungen_value esp32_cpu_temp")[0];
    if (el) el.innerHTML = val.toFixed(1);
    return;
  }

  // === BME280 (наружные) ===
  if (type === "bme_temperature") {
    let display = Math.floor(rawValue/100) + "," + (Math.abs(rawValue) % 100).toString().padStart(2, '0');
    applySensorStatus('bme_temperature', rawValue, display);
    return;
  }
  if (type === "bme_pressure") {
    let display = Math.floor(rawValue) + "," + (rawValue % 1).toFixed(1).toString().padStart(2, '0');
    applySensorStatus('bme_pressure', rawValue, display);
    return;
  }
  if (type === "bme_humidity") {
    let display = Math.floor(rawValue/100) + "," + (rawValue % 100).toString().padStart(2, '0');
    applySensorStatus('bme_humidity', rawValue, display);
    return;
  }

  // === HTU21DF (внутренние) ===
  if (type === "htu_temperature") {
    let display = Math.floor(rawValue/100) + "," + (Math.abs(rawValue) % 100).toString().padStart(2, '0');
    applySensorStatus('htu_temperature', rawValue, display);
    return;
  }
  if (type === "htu_humidity") {
    let display = Math.floor(rawValue/100) + "," + (rawValue % 100).toString().padStart(2, '0');
    applySensorStatus('htu_humidity', rawValue, display);
    return;
  }

  // === SCD4X ===
  if (type === "scd4x_co2") {
    let display = Math.floor(rawValue); // CO2 отображаем как целое
    applySensorStatus('scd4x_co2', rawValue, display);
    return;
  }
  if (type === "scd4x_temperature") {
    let display = Math.floor(rawValue/100) + "," + (Math.abs(rawValue) % 100).toString().padStart(2, '0');
    applySensorStatus('scd4x_temperature', rawValue, display);
    return;
  }
  if (type === "scd4x_humidity") {
    let display = Math.floor(rawValue/100) + "," + (rawValue % 100).toString().padStart(2, '0');
    applySensorStatus('scd4x_humidity', rawValue, display);
    return;
  }

  // === PMS5003 (частицы, значения *10) ===
  if (type === "pms_pm1") {
    let display = Math.floor(rawValue/10) + "," + (rawValue % 10);
    applySensorStatus('pms_pm1', rawValue, display);
    return;
  }
  if (type === "pms_pm2_5") {
    let display = Math.floor(rawValue/10) + "," + (rawValue % 10);
    applySensorStatus('pms_pm2_5', rawValue, display);
    return;
  }
  if (type === "pms_pm10") {
    let display = Math.floor(rawValue/10) + "," + (rawValue % 10);
    applySensorStatus('pms_pm10', rawValue, display);
    return;
  }

  // === MS5611 ===
if (type === "ms5611_pressure") {
    // rawValue в гПа, форматируем для отображения
    let display = Math.floor(rawValue) + "," + 
                  (Math.round((rawValue - Math.floor(rawValue)) * 100)).toString().padStart(2, '0');
    applySensorStatus('ms5611_pressure', rawValue, display);
    return;
}
  if (type === "ms5611_temperature") {
    let display = Math.floor(rawValue/100) + "," + (Math.abs(rawValue) % 100).toString().padStart(2, '0');
    applySensorStatus('ms5611_temperature', rawValue, display);
    return;
  }

  // === BH1750 ===
  if (type === "bh1750_lighting") {
    let display = Math.floor(rawValue/10) + "," + (rawValue % 10);
    applySensorStatus('bh1750_lighting', rawValue, display);
    return;
  }

  // === VEML6070 ===
  if (type === "veml_uv") {
    let display = Math.floor(rawValue);
    applySensorStatus('veml_uv', rawValue, display);
    return;
  }

  // === CH2O (формальдегид) ===
  if (type === "ch2o_value") {
    let display = Math.floor(rawValue/10) + "," + (rawValue % 10);
    applySensorStatus('ch2o_value', rawValue, display);
    return;
  }

  // === Микрофон (шум) ===
  if (type === "microphone_noise") {
    let display = Math.floor(rawValue/10) + "," + (rawValue % 10);
    applySensorStatus('microphone_noise', rawValue, display);
    return;
  }
}

// === Dark Mode ===
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

function autoDarkMode() {
  const saved = localStorage.getItem('darkMode');
  if (saved === 'true') {
    document.body.classList.add('dark-mode');
    document.querySelector('.dark-mode-button').src = 'on_bubl.png';
  } else {
    document.body.classList.remove('dark-mode');
    document.querySelector('.dark-mode-button').src = 'off_bubl.png';
  }
}

window.onload = function(event) {
  init();
  autoDarkMode();
}