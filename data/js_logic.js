let CPU_temp;
let output = document.getElementsByClassName("dungen_value esp32-voltage")[0];
let Socket;

function init() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onmessage = function(event) {
    processCommand(event);
  };
}

function processCommand(event) {
  var obj = JSON.parse(event.data);
  var type = obj.type;
  
  // Общие системные данные
  if (type.localeCompare("cpu_voltage") == 0) {
    var l_random_intensity = parseInt(obj.value); 
    output.innerHTML = Math.floor(l_random_intensity/100)+","+l_random_intensity%100;
  }
  if (type.localeCompare("esp32_cpu_freq") == 0) {
    let esp32_cpu_freq = parseInt(obj.value); 
    esp32_cpu_freq=esp32_cpu_freq/1000000;
    document.getElementsByClassName("dungen_value esp32_cpu_freq")[0].innerHTML=esp32_cpu_freq;
  }

  // Данные с BME280 (наружные показания)
  if (type.localeCompare("bme_temperature") == 0) {
    let bme_temperature = parseInt(obj.value); 
    bme_temperature = Math.floor(bme_temperature/100) + "," + (Math.abs(bme_temperature) % 100).toString().padStart(2, '0');
    document.getElementsByClassName("measured_value bme_temperature")[0].innerHTML = bme_temperature;
  }
  if (type.localeCompare("bme_pressure") == 0) {
    let bme_pressure = parseInt(obj.value); 
    bme_pressure = Math.floor(bme_pressure/100) + "," + (bme_pressure % 100).toString().padStart(2, '0');
    document.getElementsByClassName("measured_value bme_pressure")[0].innerHTML = bme_pressure;
  }
  if (type.localeCompare("bme_humidity") == 0) {
    let bme_humidity = parseInt(obj.value); 
    bme_humidity = Math.floor(bme_humidity/100) + "," + (bme_humidity % 100).toString().padStart(2, '0');
    document.getElementsByClassName("measured_value bme_humidity")[0].innerHTML = bme_humidity;
  }

  // Данные с HTU21DF (внутренние показания)
  if (type.localeCompare("htu_temperature") == 0) {
    let htu_temperature = parseInt(obj.value); 
    htu_temperature = Math.floor(htu_temperature/100) + "," + (Math.abs(htu_temperature) % 100).toString().padStart(2, '0');
    document.getElementsByClassName("measured_value htu_temperature")[0].innerHTML = htu_temperature;
  }
  if (type.localeCompare("htu_humidity") == 0) {
    let htu_humidity = parseInt(obj.value); 
    htu_humidity = Math.floor(htu_humidity/100) + "," + (htu_humidity % 100).toString().padStart(2, '0');
    document.getElementsByClassName("measured_value htu_humidity")[0].innerHTML = htu_humidity;
  }

  // Данные с SCD4X
  if (type.localeCompare("scd4x_co2") == 0) {
    let scd4x_co2 = parseInt(obj.value); 
    scd4x_co2 = Math.floor(scd4x_co2/100);
    document.getElementsByClassName("measured_value scd4x_co2")[0].innerHTML = scd4x_co2;
  }
  if (type.localeCompare("scd4x_temperature") == 0) {
    let scd4x_temperature = parseInt(obj.value); 
    scd4x_temperature = Math.floor(scd4x_temperature/100) + "," + (Math.abs(scd4x_temperature) % 100).toString().padStart(2, '0');
    document.getElementsByClassName("measured_value scd4x_temperature")[0].innerHTML = scd4x_temperature;
  }
  if (type.localeCompare("scd4x_humidity") == 0) {
    let scd4x_humidity = parseInt(obj.value); 
    scd4x_humidity = Math.floor(scd4x_humidity/100) + "," + (scd4x_humidity % 100).toString().padStart(2, '0');
    document.getElementsByClassName("measured_value scd4x_humidity")[0].innerHTML = scd4x_humidity;
  }

  // Данные с PMS
  if (type.localeCompare("pms_pm1") == 0) {
    let pms_pm1 = parseInt(obj.value); 
    pms_pm1 = Math.floor(pms_pm1/10) + "," + (pms_pm1 % 10);
    document.getElementsByClassName("measured_value pms_pm1")[0].innerHTML = pms_pm1;
  }
  if (type.localeCompare("pms_pm2_5") == 0) {
    let pms_pm2_5 = parseInt(obj.value); 
    pms_pm2_5 = Math.floor(pms_pm2_5/10) + "," + (pms_pm2_5 % 10);
    document.getElementsByClassName("measured_value pms_pm2_5")[0].innerHTML = pms_pm2_5;
  }
  if (type.localeCompare("pms_pm10") == 0) {
    let pms_pm10 = parseInt(obj.value); 
    pms_pm10 = Math.floor(pms_pm10/10) + "," + (pms_pm10 % 10);
    document.getElementsByClassName("measured_value pms_pm10")[0].innerHTML = pms_pm10;
  }

  // Данные с MS5611
  if (type.localeCompare("ms5611_pressure") == 0) {
    let ms5611_pressure = parseInt(obj.value); 
    ms5611_pressure = Math.floor(ms5611_pressure/100) + "," + (ms5611_pressure % 100).toString().padStart(2, '0');
    document.getElementsByClassName("measured_value ms5611_pressure")[0].innerHTML = ms5611_pressure;
  }
  if (type.localeCompare("ms5611_temperature") == 0) {
    let ms5611_temperature = parseInt(obj.value); 
    ms5611_temperature = Math.floor(ms5611_temperature/100) + "," + (Math.abs(ms5611_temperature) % 100).toString().padStart(2, '0');
    document.getElementsByClassName("measured_value ms5611_temperature")[0].innerHTML = ms5611_temperature;
  }

  // Данные с BH1750
  if (type.localeCompare("bh1750_lighting") == 0) {
    let bh1750_lighting = parseInt(obj.value); 
    bh1750_lighting = Math.floor(bh1750_lighting/10) + "," + (bh1750_lighting % 10);
    document.getElementsByClassName("measured_value bh1750_lighting")[0].innerHTML = bh1750_lighting;
  }

  // Данные с VEML6070
  if (type.localeCompare("veml_uv") == 0) {
    let veml_uv = parseInt(obj.value); 
    veml_uv = Math.floor(veml_uv);
    document.getElementsByClassName("measured_value veml_uv")[0].innerHTML = veml_uv;
  }

  // Данные с CH2O
  if (type.localeCompare("ch2o_value") == 0) {
    let ch2o_value = parseInt(obj.value); 
    ch2o_value = Math.floor(ch2o_value/10) + "," + (ch2o_value % 10);
    document.getElementsByClassName("measured_value ch2o_value")[0].innerHTML = ch2o_value;
  }

  // Данные с микрофона
  if (type.localeCompare("microphone_noise") == 0) {
    let microphone_noise = parseInt(obj.value); 
    microphone_noise = Math.floor(microphone_noise/10) + "," + (microphone_noise % 10);
    document.getElementsByClassName("measured_value microphone_noise")[0].innerHTML = microphone_noise;
  }
}

window.onload = function(event) {
  init();
}