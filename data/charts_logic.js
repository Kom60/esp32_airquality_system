// Charts and AQI logic
const MAX_DATA_POINTS = 50;
let autoScroll = true;
let dataHistory = {
  labels: [],
  bme_temperature: [],
  htu_temperature: [],
  scd4x_temperature: [],
  bme_humidity: [],
  htu_humidity: [],
  scd4x_humidity: [],
  bme_pressure: [],
  ms5611_pressure: [],
  scd4x_co2: [],
  pms_pm2_5: [],
  pms_pm10: [],
  ch2o_value: [],
  bh1750_lighting: [],
  veml_uv: [],
  microphone_noise: []
};

let charts = {};
let Socket;

// AQI thresholds
const AQI_THRESHOLDS = {
  pm25: { good: 12, moderate: 35, unhealthy_sensitive: 55, unhealthy: 150, very_unhealthy: 250 },
  pm10: { good: 54, moderate: 154, unhealthy_sensitive: 254, unhealthy: 354, very_unhealthy: 424 },
  co2: { good: 800, moderate: 1000, unhealthy_sensitive: 1400, unhealthy: 2000, very_unhealthy: 5000 },
  ch2o: { good: 0.03, moderate: 0.08, unhealthy_sensitive: 0.1, unhealthy: 0.2, very_unhealthy: 0.5 }
};

// Initialize charts
function initCharts() {
  const commonOptions = {
    responsive: true,
    maintainAspectRatio: false,
    animation: { duration: 0 },
    scales: {
      xAxes: [{
        type: 'category',
        ticks: {
          maxTicksLimit: 10,  // Увеличено количество меток
          fontColor: '#000000',  // Чёрный цвет
          fontFamily: '"Press Start 2P"',
          fontSize: getResponsiveFontSize(8)
        },
        gridLines: { color: '#333' }
      }],
      yAxes: [{
        ticks: {
          fontColor: '#000000',  // Чёрный цвет
          fontFamily: '"Press Start 2P"',
          fontSize: getResponsiveFontSize(8)
        },
        gridLines: { color: '#333' }
      }]
    },
    legend: {
      labels: {
        fontColor: '#000000',  // Чёрный цвет
        fontFamily: '"Press Start 2P"',
        fontSize: getResponsiveFontSize(8)
      }
    }
  };

  // Temperature Chart
  charts.temp = new Chart(document.getElementById('tempChart'), {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'BME (out)',
          data: [],
          borderColor: '#00D4FF',
          backgroundColor: 'rgba(0, 212, 255, 0.1)',
          fill: true,
          tension: 0.4
        },
        {
          label: 'HTU (in)',
          data: [],
          borderColor: '#FF6B6B',
          backgroundColor: 'rgba(255, 107, 107, 0.1)',
          fill: true,
          tension: 0.4
        },
        {
          label: 'SCD',
          data: [],
          borderColor: '#4ECDC4',
          backgroundColor: 'rgba(78, 205, 196, 0.1)',
          fill: true,
          tension: 0.4
        }
      ]
    },
    options: commonOptions
  });

  // Humidity Chart
  charts.humidity = new Chart(document.getElementById('humidityChart'), {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'HTU',
          data: [],
          borderColor: '#6C5CE7',
          backgroundColor: 'rgba(108, 92, 231, 0.1)',
          fill: true,
          tension: 0.4
        },
        {
          label: 'SCD',
          data: [],
          borderColor: '#FFD93D',
          backgroundColor: 'rgba(255, 217, 61, 0.1)',
          fill: true,
          tension: 0.4
        }
      ]
    },
    options: commonOptions
  });

  // CO2 Chart
  charts.co2 = new Chart(document.getElementById('co2Chart'), {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'CO₂ (ppm)',
        data: [],
        borderColor: '#4ECDC4',
        backgroundColor: 'rgba(78, 205, 196, 0.1)',
        fill: true,
        tension: 0.4
      }]
    },
    options: commonOptions
  });

  // PM2.5 Chart
  charts.pm25 = new Chart(document.getElementById('pm25Chart'), {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'PM2.5',
        data: [],
        borderColor: '#FF6B6B',
        backgroundColor: 'rgba(255, 107, 107, 0.1)',
        fill: true,
        tension: 0.4
      }]
    },
    options: commonOptions
  });

  // PM10 Chart
  charts.pm10 = new Chart(document.getElementById('pm10Chart'), {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'PM10',
        data: [],
        borderColor: '#FFA500',
        backgroundColor: 'rgba(255, 165, 0, 0.1)',
        fill: true,
        tension: 0.4
      }]
    },
    options: commonOptions
  });

  // Pressure Chart
  charts.pressure = new Chart(document.getElementById('pressureChart'), {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'BME',
          data: [],
          borderColor: '#00D4FF',
          backgroundColor: 'rgba(0, 212, 255, 0.1)',
          fill: true,
          tension: 0.4
        },
        {
          label: 'MS5611',
          data: [],
          borderColor: '#FFD93D',
          backgroundColor: 'rgba(255, 217, 61, 0.1)',
          fill: true,
          tension: 0.4
        }
      ]
    },
    options: commonOptions
  });
}

// Calculate AQI from sensor values
function calculateAQI(pm25, pm10, co2, ch2o) {
  let aqi = 0;
  let dominant = 'PM2.5';

  // PM2.5 AQI
  if (pm25 <= AQI_THRESHOLDS.pm25.good) aqi = Math.max(aqi, (pm25 / AQI_THRESHOLDS.pm25.good) * 50);
  else if (pm25 <= AQI_THRESHOLDS.pm25.moderate) aqi = Math.max(aqi, 50 + ((pm25 - AQI_THRESHOLDS.pm25.good) / (AQI_THRESHOLDS.pm25.moderate - AQI_THRESHOLDS.pm25.good)) * 50);
  else if (pm25 <= AQI_THRESHOLDS.pm25.unhealthy_sensitive) aqi = Math.max(aqi, 100 + ((pm25 - AQI_THRESHOLDS.pm25.moderate) / (AQI_THRESHOLDS.pm25.unhealthy_sensitive - AQI_THRESHOLDS.pm25.moderate)) * 50);
  else if (pm25 <= AQI_THRESHOLDS.pm25.unhealthy) aqi = Math.max(aqi, 150 + ((pm25 - AQI_THRESHOLDS.pm25.unhealthy_sensitive) / (AQI_THRESHOLDS.pm25.unhealthy - AQI_THRESHOLDS.pm25.unhealthy_sensitive)) * 50);
  else aqi = Math.max(aqi, 200 + ((pm25 - AQI_THRESHOLDS.pm25.unhealthy) / (AQI_THRESHOLDS.pm25.very_unhealthy - AQI_THRESHOLDS.pm25.unhealthy)) * 100);

  // PM10 AQI
  if (pm10 <= AQI_THRESHOLDS.pm10.good) aqi = Math.max(aqi, (pm10 / AQI_THRESHOLDS.pm10.good) * 50);
  else if (pm10 <= AQI_THRESHOLDS.pm10.moderate) aqi = Math.max(aqi, 50 + ((pm10 - AQI_THRESHOLDS.pm10.good) / (AQI_THRESHOLDS.pm10.moderate - AQI_THRESHOLDS.pm10.good)) * 50);
  else if (pm10 <= AQI_THRESHOLDS.pm10.unhealthy_sensitive) aqi = Math.max(aqi, 100 + ((pm10 - AQI_THRESHOLDS.pm10.moderate) / (AQI_THRESHOLDS.pm10.unhealthy_sensitive - AQI_THRESHOLDS.pm10.moderate)) * 50);
  else if (pm10 <= AQI_THRESHOLDS.pm10.unhealthy) aqi = Math.max(aqi, 150 + ((pm10 - AQI_THRESHOLDS.pm10.unhealthy_sensitive) / (AQI_THRESHOLDS.pm10.unhealthy - AQI_THRESHOLDS.pm10.unhealthy_sensitive)) * 50);
  else aqi = Math.max(aqi, 200 + ((pm10 - AQI_THRESHOLDS.pm10.unhealthy) / (AQI_THRESHOLDS.pm10.very_unhealthy - AQI_THRESHOLDS.pm10.unhealthy)) * 100);

  // CO2 AQI (indoor air quality)
  if (co2 <= AQI_THRESHOLDS.co2.good) aqi = Math.max(aqi, (co2 / AQI_THRESHOLDS.co2.good) * 50);
  else if (co2 <= AQI_THRESHOLDS.co2.moderate) aqi = Math.max(aqi, 50 + ((co2 - AQI_THRESHOLDS.co2.good) / (AQI_THRESHOLDS.co2.moderate - AQI_THRESHOLDS.co2.good)) * 50);
  else if (co2 <= AQI_THRESHOLDS.co2.unhealthy_sensitive) aqi = Math.max(aqi, 100 + ((co2 - AQI_THRESHOLDS.co2.moderate) / (AQI_THRESHOLDS.co2.unhealthy_sensitive - AQI_THRESHOLDS.co2.moderate)) * 50);
  else if (co2 <= AQI_THRESHOLDS.co2.unhealthy) aqi = Math.max(aqi, 150 + ((co2 - AQI_THRESHOLDS.co2.unhealthy_sensitive) / (AQI_THRESHOLDS.co2.unhealthy - AQI_THRESHOLDS.co2.unhealthy_sensitive)) * 50);
  else aqi = Math.max(aqi, 200 + ((co2 - AQI_THRESHOLDS.co2.unhealthy) / (AQI_THRESHOLDS.co2.very_unhealthy - AQI_THRESHOLDS.co2.unhealthy)) * 100);

  // CH2O AQI
  if (ch2o <= AQI_THRESHOLDS.ch2o.good) aqi = Math.max(aqi, (ch2o / AQI_THRESHOLDS.ch2o.good) * 50);
  else if (ch2o <= AQI_THRESHOLDS.ch2o.moderate) aqi = Math.max(aqi, 50 + ((ch2o - AQI_THRESHOLDS.ch2o.good) / (AQI_THRESHOLDS.ch2o.moderate - AQI_THRESHOLDS.ch2o.good)) * 50);
  else if (ch2o <= AQI_THRESHOLDS.ch2o.unhealthy_sensitive) aqi = Math.max(aqi, 100 + ((ch2o - AQI_THRESHOLDS.ch2o.moderate) / (AQI_THRESHOLDS.ch2o.unhealthy_sensitive - AQI_THRESHOLDS.ch2o.moderate)) * 50);
  else if (ch2o <= AQI_THRESHOLDS.ch2o.unhealthy) aqi = Math.max(aqi, 150 + ((ch2o - AQI_THRESHOLDS.ch2o.unhealthy_sensitive) / (AQI_THRESHOLDS.ch2o.unhealthy - AQI_THRESHOLDS.ch2o.unhealthy_sensitive)) * 50);
  else aqi = Math.max(aqi, 200 + ((ch2o - AQI_THRESHOLDS.ch2o.unhealthy) / (AQI_THRESHOLDS.ch2o.very_unhealthy - AQI_THRESHOLDS.ch2o.unhealthy)) * 100);

  return Math.min(300, Math.round(aqi));
}

// Get AQI status
function getAQIStatus(aqi) {
  if (aqi <= 50) return { text: 'Хорошее', class: 'aqi-good', color: '#4ECDC4' };
  if (aqi <= 100) return { text: 'Умеренное', class: 'aqi-moderate', color: '#FFD93D' };
  if (aqi <= 150) return { text: 'Вредно для чувствительных', class: 'aqi-unhealthy-sensitive', color: '#FFA500' };
  if (aqi <= 200) return { text: 'Вредно', class: 'aqi-unhealthy', color: '#FF6B6B' };
  if (aqi <= 300) return { text: 'Очень вредно', class: 'aqi-very-unhealthy', color: '#8B0000' };
  return { text: 'Опасно', class: 'aqi-hazardous', color: '#4B0082' };
}

// Update AQI display
function updateAQI(pm25, pm10, co2, ch2o) {
  const aqi = calculateAQI(pm25, pm10, co2, ch2o);
  const status = getAQIStatus(aqi);
  
  document.getElementById('aqi_value').textContent = aqi;
  document.getElementById('aqi_value').style.color = status.color;
  document.getElementById('aqi_status').textContent = status.text;
  document.getElementById('aqi_status').className = 'aqi-status ' + status.class;
  
  const barFill = document.getElementById('aqi_bar');
  barFill.style.width = Math.min(100, (aqi / 300) * 100) + '%';
  barFill.style.backgroundColor = status.color;
  
  // Update individual values
  document.getElementById('aqi_pm25').textContent = pm25.toFixed(1);
  document.getElementById('aqi_pm10').textContent = pm10.toFixed(1);
  document.getElementById('aqi_co2').textContent = Math.round(co2);
  document.getElementById('aqi_ch2o').textContent = ch2o.toFixed(3);
}

// Add data point to chart
function addDataPoint(chart, label, data) {
  chart.data.labels.push(label);
  chart.data.datasets.forEach((dataset, i) => {
    if (data[i] !== undefined) {
      chart.data.datasets[i].data.push(data[i]);
    }
  });
  
  // Remove old data points
  if (chart.data.labels.length > MAX_DATA_POINTS) {
    chart.data.labels.shift();
    chart.data.datasets.forEach(dataset => dataset.data.shift());
  }
  
  chart.update();
}

// WebSocket handler
function processCommand(event) {
  const obj = JSON.parse(event.data);
  const type = obj.type;
  const rawValue = parseFloat(obj.value);

  const now = new Date();
  // Формат ЧЧ:ММ
  const timeLabel = now.getHours().toString().padStart(2, '0') + ':' +
                    now.getMinutes().toString().padStart(2, '0');

  // Store data
  if (dataHistory[type] !== undefined) {
    dataHistory[type].push(rawValue);
    if (dataHistory[type].length > MAX_DATA_POINTS) dataHistory[type].shift();
  }

  if (dataHistory.labels.length === 0 || dataHistory.labels.length < MAX_DATA_POINTS) {
    dataHistory.labels.push(timeLabel);
    if (dataHistory.labels.length > MAX_DATA_POINTS) dataHistory.labels.shift();
  }

  // Update charts based on sensor type
  switch(type) {
    case 'bme_temperature':
    case 'htu_temperature':
    case 'scd4x_temperature':
      updateTempChart();
      break;
    case 'bme_humidity':
    case 'htu_humidity':
    case 'scd4x_humidity':
      updateHumidityChart();
      break;
    case 'scd4x_co2':
      updateCO2Chart(rawValue);
      updateAQI(
        parseFloat(obj.value) || 0,
        dataHistory.pms_pm10[dataHistory.pms_pm10.length - 1] || 0,
        rawValue,
        dataHistory.ch2o_value[dataHistory.ch2o_value.length - 1] || 0
      );
      break;
    case 'pms_pm2_5':
      updatePM25Chart(rawValue);  // Данные в µg/m³ (без /10)
      updateAQI(
        rawValue,
        dataHistory.pms_pm10[dataHistory.pms_pm10.length - 1] || 0,
        dataHistory.scd4x_co2[dataHistory.scd4x_co2.length - 1] || 0,
        dataHistory.ch2o_value[dataHistory.ch2o_value.length - 1] || 0
      );
      break;
    case 'pms_pm10':
      updatePM10Chart(rawValue);  // Данные в µg/m³ (без /10)
      break;
    case 'bme_pressure':
    case 'ms5611_pressure':
      updatePressureChart();
      break;
  }
}

function updateTempChart() {
  const data = [
    dataHistory.bme_temperature[dataHistory.bme_temperature.length - 1] || 0,
    dataHistory.htu_temperature[dataHistory.htu_temperature.length - 1] || 0,
    dataHistory.scd4x_temperature[dataHistory.scd4x_temperature.length - 1] || 0
  ];
  addDataPoint(charts.temp, dataHistory.labels[dataHistory.labels.length - 1], data);
}

function updateHumidityChart() {
  const data = [
    dataHistory.htu_humidity[dataHistory.htu_humidity.length - 1] || 0,
    dataHistory.scd4x_humidity[dataHistory.scd4x_humidity.length - 1] || 0
  ];
  addDataPoint(charts.humidity, dataHistory.labels[dataHistory.labels.length - 1], data);
}

function updateCO2Chart(value) {
  addDataPoint(charts.co2, dataHistory.labels[dataHistory.labels.length - 1], [value]);
}

function updatePM25Chart(value) {
  addDataPoint(charts.pm25, dataHistory.labels[dataHistory.labels.length - 1], [value]);  // Данные в µg/m³
}

function updatePM10Chart(value) {
  addDataPoint(charts.pm10, dataHistory.labels[dataHistory.labels.length - 1], [value]);  // Данные в µg/m³
}

function updatePressureChart() {
  const data = [
    (dataHistory.bme_pressure[dataHistory.bme_pressure.length - 1] || 0),  // уже в гПа
    (dataHistory.ms5611_pressure[dataHistory.ms5611_pressure.length - 1] || 0)  // уже в гПа
  ];
  addDataPoint(charts.pressure, dataHistory.labels[dataHistory.labels.length - 1], data);
}

// Control functions
function clearCharts() {
  Object.values(charts).forEach(chart => {
    chart.data.labels = [];
    chart.data.datasets.forEach(dataset => dataset.data = []);
    chart.update();
  });
  dataHistory.labels = [];
  Object.keys(dataHistory).forEach(key => {
    if (key !== 'labels') dataHistory[key] = [];
  });
}

// Обновление размеров шрифтов при изменении окна
let resizeTimeout;
window.addEventListener('resize', function() {
  clearTimeout(resizeTimeout);
  resizeTimeout = setTimeout(function() {
    updateChartFonts();
  }, 250);
});

function getResponsiveFontSize(baseSize) {
  const width = window.innerWidth;
  // Увеличиваем на 10% от базового размера
  const increasedSize = baseSize * 1.1;
  if (width <= 480) return increasedSize * 0.7;
  if (width <= 768) return increasedSize * 0.85;
  return increasedSize;
}

function updateChartFonts() {
  const fontSize = getResponsiveFontSize(8);
  
  Object.values(charts).forEach(chart => {
    // Обновить шрифты осей (чёрный цвет и на 10% крупнее)
    chart.options.scales.xAxes[0].ticks.fontSize = fontSize;
    chart.options.scales.xAxes[0].ticks.fontColor = '#000000';
    chart.options.scales.xAxes[0].ticks.maxTicksLimit = 10;  // Больше меток
    chart.options.scales.yAxes[0].ticks.fontSize = fontSize;
    chart.options.scales.yAxes[0].ticks.fontColor = '#000000';
    chart.options.legend.fontSize = fontSize;
    chart.options.legend.fontColor = '#000000';
    
    chart.update();
  });
}

function toggleAutoScroll() {
  autoScroll = !autoScroll;
}

function exportData() {
  let csv = 'Time,BME_Temp,HTU_Temp,SCD_Temp,BME_Humidity,HTU_Humidity,SCD_Humidity,CO2,PM2.5,PM10\n';

  for (let i = 0; i < dataHistory.labels.length; i++) {
    csv += dataHistory.labels[i] + ',';
    csv += (dataHistory.bme_temperature[i] || 0) + ',';
    csv += (dataHistory.htu_temperature[i] || 0) + ',';
    csv += (dataHistory.scd4x_temperature[i] || 0) + ',';
    csv += (dataHistory.bme_humidity[i] || 0) + ',';
    csv += (dataHistory.htu_humidity[i] || 0) + ',';
    csv += (dataHistory.scd4x_humidity[i] || 0) + ',';
    csv += (dataHistory.scd4x_co2[i] || 0) + ',';
    csv += (dataHistory.pms_pm2_5[i] || 0) + ',';
    csv += (dataHistory.pms_pm10[i] || 0) + '\n';
  }
  
  const blob = new Blob([csv], { type: 'text/csv' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = 'air_quality_data_' + new Date().toISOString().slice(0, 10) + '.csv';
  a.click();
  URL.revokeObjectURL(url);
}

// Dark mode toggle
function toggleDarkMode() {
  document.body.classList.toggle('dark-mode');
  const isDark = document.body.classList.contains('dark-mode');
  localStorage.setItem('darkMode', isDark);
  document.querySelector('.dark-mode-button').src = isDark ? 'on_bubl.png' : 'off_bubl.png';
}

function autoDarkMode() {
  const saved = localStorage.getItem('darkMode');
  if (saved === 'true') {
    document.body.classList.add('dark-mode');
    document.querySelector('.dark-mode-button').src = 'on_bubl.png';
  }
}

// Initialize
function init() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onmessage = function(event) {
    processCommand(event);
  };
  Socket.onopen = function() {
    console.log('WebSocket connected');
  };
  
  initCharts();
  autoDarkMode();
}

window.onload = init;
