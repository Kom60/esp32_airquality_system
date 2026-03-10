// Загрузка настроек при открытии страницы
window.onload = function() {
  loadSettings();
  autoDarkMode();
};

// Загрузка настроек с сервера
async function loadSettings() {
  try {
    const response = await fetch('/api/settings');
    if (!response.ok) throw new Error('Failed to load settings');
    const settings = await response.json();
    
    // WiFi настройки
    document.getElementById('wifi_ssid').value = settings.wifi_ssid || '';
    document.getElementById('wifi_password').value = settings.wifi_password || '';
    
    // Интервал обновления
    document.getElementById('update_interval').value = settings.update_interval || 10;
    
    // Калибровка температуры
    document.getElementById('temp_offset_bme').value = settings.temp_offset_bme || 0;
    document.getElementById('temp_offset_htu').value = settings.temp_offset_htu || 0;
    document.getElementById('temp_offset_scd').value = settings.temp_offset_scd || 0;
    
    // Калибровка влажности
    document.getElementById('hum_offset_bme').value = settings.hum_offset_bme || 0;
    document.getElementById('hum_offset_htu').value = settings.hum_offset_htu || 0;
    
    // Калибровка давления
    document.getElementById('press_offset_bme').value = settings.press_offset_bme || 0;
    document.getElementById('press_offset_ms').value = settings.press_offset_ms || 0;
    
    // Пороги CO2
    document.getElementById('co2_warning').value = settings.co2_warning || 1000;
    document.getElementById('co2_critical').value = settings.co2_critical || 1400;
    
    // Пороги PM2.5
    document.getElementById('pm25_warning').value = settings.pm25_warning || 35;
    document.getElementById('pm25_critical').value = settings.pm25_critical || 50;
    
    // Ночной режим
    document.getElementById('night_mode_start').value = settings.night_mode_start || 23;
    document.getElementById('night_mode_end').value = settings.night_mode_end || 7;
    
  } catch (error) {
    showStatus('Ошибка загрузки настроек: ' + error.message, 'error');
  }
}

// Сохранение настроек
async function saveSettings() {
  const settings = {
    wifi_ssid: document.getElementById('wifi_ssid').value,
    wifi_password: document.getElementById('wifi_password').value,
    update_interval: parseInt(document.getElementById('update_interval').value),
    temp_offset_bme: parseFloat(document.getElementById('temp_offset_bme').value),
    temp_offset_htu: parseFloat(document.getElementById('temp_offset_htu').value),
    temp_offset_scd: parseFloat(document.getElementById('temp_offset_scd').value),
    hum_offset_bme: parseInt(document.getElementById('hum_offset_bme').value),
    hum_offset_htu: parseInt(document.getElementById('hum_offset_htu').value),
    press_offset_bme: parseInt(document.getElementById('press_offset_bme').value),
    press_offset_ms: parseInt(document.getElementById('press_offset_ms').value),
    co2_warning: parseInt(document.getElementById('co2_warning').value),
    co2_critical: parseInt(document.getElementById('co2_critical').value),
    pm25_warning: parseInt(document.getElementById('pm25_warning').value),
    pm25_critical: parseInt(document.getElementById('pm25_critical').value),
    night_mode_start: parseInt(document.getElementById('night_mode_start').value),
    night_mode_end: parseInt(document.getElementById('night_mode_end').value)
  };
  
  try {
    const response = await fetch('/api/settings', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(settings)
    });
    
    if (response.ok) {
      showStatus('Настройки сохранены! Перезагрузите ESP32 для применения.', 'success');
    } else {
      showStatus('Ошибка сохранения настроек', 'error');
    }
  } catch (error) {
    showStatus('Ошибка: ' + error.message, 'error');
  }
}

// Сброс настроек
async function resetSettings() {
  if (!confirm('Сбросить все настройки к заводским?')) return;
  
  try {
    const response = await fetch('/api/settings/reset', { method: 'POST' });
    if (response.ok) {
      showStatus('Настройки сброшены. Перезагрузите ESP32.', 'success');
      loadSettings();
    } else {
      showStatus('Ошибка сброса настроек', 'error');
    }
  } catch (error) {
    showStatus('Ошибка: ' + error.message, 'error');
  }
}

// Перезагрузка ESP32
async function rebootESP() {
  if (!confirm('Перезагрузить ESP32?')) return;
  
  try {
    const response = await fetch('/api/reboot', { method: 'POST' });
    if (response.ok) {
      showStatus('ESP32 перезагружается...', 'success');
      setTimeout(() => location.reload(), 5000);
    } else {
      showStatus('Ошибка перезагрузки', 'error');
    }
  } catch (error) {
    showStatus('Ошибка: ' + error.message, 'error');
  }
}

// Применение настроек WiFi
async function applyWiFi() {
  const ssid = document.getElementById('wifi_ssid').value;
  const password = document.getElementById('wifi_password').value;
  
  if (!ssid) {
    showStatus('Введите SSID сети', 'error');
    return;
  }
  
  if (!confirm('Применить настройки WiFi и перезагрузить ESP32?')) return;
  
  const settings = {
    wifi_ssid: ssid,
    wifi_password: password
  };
  
  try {
    const response = await fetch('/api/settings', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(settings)
    });
    
    if (response.ok) {
      showStatus('Настройки WiFi сохранены. ESP32 перезагружается...', 'success');
      setTimeout(() => {
        fetch('/api/reboot', { method: 'POST' });
        setTimeout(() => location.reload(), 3000);
      }, 1000);
    } else {
      showStatus('Ошибка сохранения настроек WiFi', 'error');
    }
  } catch (error) {
    showStatus('Ошибка: ' + error.message, 'error');
  }
}

// Отображение сообщения
function showStatus(message, type) {
  const el = document.getElementById('status_message');
  el.textContent = message;
  el.className = 'status-message ' + type;
  setTimeout(() => el.className = 'status-message', 5000);
}

// Тёмная тема
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

function toggleDarkMode() {
  document.body.classList.toggle('dark-mode');
  const isDark = document.body.classList.contains('dark-mode');
  localStorage.setItem('darkMode', isDark);
  document.querySelector('.dark-mode-button').src = isDark ? 'on_bubl.png' : 'off_bubl.png';
}
