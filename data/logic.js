let counter=1;
let Socket;

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

// Форматирование значения с плавающей точкой
function formatValue(rawValue, decimals = 1) {
  if (rawValue === null || rawValue === undefined || isNaN(rawValue)) return '---';
  let val = rawValue / 100;
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
      if (el) el.innerHTML = (rawValue / 100).toFixed(1);
      return;
    }
    
    // Датчики - форматирование и отображение
    const sensors = {
      'bme_temperature': { class: 'bme_temperature', suffix: '°C', div: 100, decimals: 1 },
      'bme_pressure': { class: 'bme_pressure', suffix: 'гПа', div: 100, decimals: 1 },
      'bme_humidity': { class: 'bme_humidity', suffix: '%', div: 100, decimals: 1 },
      'htu_temperature': { class: 'htu_temperature', suffix: '°C', div: 100, decimals: 1 },
      'htu_humidity': { class: 'htu_humidity', suffix: '%', div: 100, decimals: 1 },
      'ms5611_pressure': { class: 'ms5611_pressure', suffix: 'гПа', div: 100, decimals: 1 },  // уже в гПа
      'ms5611_temperature': { class: 'ms5611_temperature', suffix: '°C', div: 100, decimals: 1 },
      'scd4x_co2': { class: 'scd4x_co2', suffix: 'ppm', div: 1, decimals: 0 },
      'scd4x_temperature': { class: 'scd4x_temperature', suffix: '°C', div: 100, decimals: 1 },
      'scd4x_humidity': { class: 'scd4x_humidity', suffix: '%', div: 100, decimals: 1 },
      'pms_pm1': { class: 'pms_pm1', suffix: 'мкг/м³', div: 10, decimals: 1 },
      'pms_pm2_5': { class: 'pms_pm2_5', suffix: 'мкг/м³', div: 10, decimals: 1 },
      'pms_pm10': { class: 'pms_pm10', suffix: 'мкг/м³', div: 10, decimals: 1 },
      'bh1750_lighting': { class: 'bh1750_lighting', suffix: 'лк', div: 10, decimals: 1 },
      'veml_uv': { class: 'veml_uv', suffix: '', div: 1, decimals: 0 },
      'ch2o_value': { class: 'ch2o_value', suffix: 'ppm', div: 10, decimals: 3 },
      'microphone_noise': { class: 'microphone_noise', suffix: 'дБ', div: 10, decimals: 1 }
    };
    
    if (sensors[type]) {
      const s = sensors[type];
      const val = (rawValue / s.div).toFixed(s.decimals);
      updateSensorValue(s.class, val);
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
  if (saved === 'true') {
    document.body.classList.add('dark-mode');
    document.querySelector('.dark-mode-button').src = 'on_bubl.png';
  } else {
    document.body.classList.remove('dark-mode');
    document.querySelector('.dark-mode-button').src = 'off_bubl.png';
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