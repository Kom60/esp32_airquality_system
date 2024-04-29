let CPU_temp;

let output = document.getElementsByClassName("dungen_value esp32-voltage")[0];

let Socket;

function init() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
  Socket.onmessage = function(event) {
    processCommand(event);
  };
}
/*
function slider_changed () {
  var l_random_intensity = slider.value;
  console.log(l_random_intensity);
  var msg = { type: "random_intensity", value: l_random_intensity};
  Socket.send(JSON.stringify(msg)); 
}
*/
function processCommand(event) {
  var obj = JSON.parse(event.data);
  var type = obj.type;
  if (type.localeCompare("cpu_voltage") == 0) {
    var l_random_intensity = parseInt(obj.value); 
    console.log(l_random_intensity); 
    output.innerHTML = Math.floor(l_random_intensity/100)+","+l_random_intensity%100;
  }
  if (type.localeCompare("esp32_cpu_freq") == 0) {
    let esp32_cpu_freq = parseInt(obj.value); 
    esp32_cpu_freq=esp32_cpu_freq/1000000;
    document.getElementsByClassName("dungen_value esp32_cpu_freq")[0].innerHTML=esp32_cpu_freq;
  }

  if (type.localeCompare("indoor_temp") == 0) {
    let indoor_temp = parseInt(obj.value); 
    indoor_temp=Math.floor(indoor_temp/100)+","+ Math.abs(indoor_temp%100);
    document.getElementsByClassName("measured_value indoor_temp")[0].innerHTML=indoor_temp;
  }
  if (type.localeCompare("indoor_press") == 0) {
    let indoor_press = parseInt(obj.value); 
    indoor_press=Math.floor(indoor_press/100)+","+ indoor_press%100;
    document.getElementsByClassName("measured_value indoor_press")[0].innerHTML=indoor_press;
  }
  if (type.localeCompare("indoor_humidity") == 0) {
    let indoor_humidity = parseInt(obj.value); 
    indoor_humidity=Math.floor(indoor_humidity/100)+","+ indoor_humidity%100;
    document.getElementsByClassName("measured_value indoor_humidity")[0].innerHTML=indoor_humidity;
  }
  if (type.localeCompare("indoor_light") == 0) {
    let indoor_light = parseInt(obj.value); 
    indoor_light=Math.floor(indoor_light/10)+","+ indoor_light%10;
    document.getElementsByClassName("measured_value indoor_light")[0].innerHTML=indoor_light;
  }
  if (type.localeCompare("indoor_CO2") == 0) {
    let indoor_CO2 = parseInt(obj.value); 
    indoor_CO2=Math.floor(indoor_CO2);
    document.getElementsByClassName("measured_value indoor_CO2")[0].innerHTML=indoor_CO2;
  }
  if (type.localeCompare("indoor_CH2O") == 0) {
    let indoor_CH2O = parseInt(obj.value); 
    indoor_CH2O=Math.floor(indoor_CH2O/10)+","+ indoor_CH2O%10;
    document.getElementsByClassName("measured_value indoor_CH2O")[0].innerHTML=indoor_CH2O;
  }
  if (type.localeCompare("indoor_1_0") == 0) {
    let indoor_1_0 = parseInt(obj.value); 
    indoor_1_0=Math.floor(indoor_1_0/10)+","+ indoor_1_0%10;
    document.getElementsByClassName("measured_value indoor_1_0")[0].innerHTML=indoor_1_0;
  }
  if (type.localeCompare("indoor_pm2_5") == 0) {
    let indoor_pm2_5 = parseInt(obj.value); 
    indoor_pm2_5=Math.floor(indoor_pm2_5/10)+","+ indoor_pm2_5%10;
    document.getElementsByClassName("measured_value indoor_pm2_5")[0].innerHTML=indoor_pm2_5;
  }
  if (type.localeCompare("indoor_pm10") == 0) {
    let indoor_pm10 = parseInt(obj.value); 
    indoor_pm10=Math.floor(indoor_pm10/10)+","+ indoor_pm10%10;
    document.getElementsByClassName("measured_value indoor_pm10")[0].innerHTML=indoor_pm10;
  }
  if (type.localeCompare("indoor_radiation") == 0) {
    let indoor_radiation = parseInt(obj.value); 
    indoor_radiation=Math.floor(indoor_radiation/10)+","+ indoor_radiation%10;
    document.getElementsByClassName("measured_value indoor_radiation")[0].innerHTML=indoor_radiation;
  }
  if (type.localeCompare("indoor_noise") == 0) {
    let indoor_noise = parseInt(obj.value); 
    indoor_noise=Math.floor(indoor_noise/10)+","+ indoor_noise%10;
    document.getElementsByClassName("measured_value indoor_noise")[0].innerHTML=indoor_noise;
  }
  
 

  if (type.localeCompare("outdoor_temp") == 0) {
    let outdoor_temp = parseInt(obj.value); 
    outdoor_temp=Math.floor(outdoor_temp/100)+","+ Math.abs(outdoor_temp%100);
    document.getElementsByClassName("measured_value outdoor_temp")[0].innerHTML=outdoor_temp;
  }
  if (type.localeCompare("outdoor_press") == 0) {
    let outdoor_press = parseInt(obj.value); 
    outdoor_press=Math.floor(outdoor_press/100)+","+ outdoor_press%100;
    document.getElementsByClassName("measured_value outdoor_press")[0].innerHTML=outdoor_press;
  }
  if (type.localeCompare("outdoor_humidity") == 0) {
    let outdoor_humidity = parseInt(obj.value); 
    outdoor_humidity=Math.floor(outdoor_humidity/100)+","+ outdoor_humidity%100;
    document.getElementsByClassName("measured_value outdoor_humidity")[0].innerHTML=outdoor_humidity;
  }
  if (type.localeCompare("outdoor_light") == 0) {
    let outdoor_light = parseInt(obj.value); 
    outdoor_light=Math.floor(outdoor_light/10)+","+ outdoor_light%10;
    document.getElementsByClassName("measured_value outdoor_light")[0].innerHTML=outdoor_light;
  }
  if (type.localeCompare("outdoor_CO2") == 0) {
    let outdoor_CO2 = parseInt(obj.value); 
    outdoor_CO2=Math.floor(outdoor_CO2/10)+","+ outdoor_CO2%10;
    document.getElementsByClassName("measured_value outdoor_CO2")[0].innerHTML=outdoor_CO2;
  }
  if (type.localeCompare("outdoor_CH2O") == 0) {
    let outdoor_CH2O = parseInt(obj.value); 
    outdoor_CH2O=Math.floor(outdoor_CH2O/10)+","+ outdoor_CH2O%10;
    document.getElementsByClassName("measured_value outdoor_CH2O")[0].innerHTML=outdoor_CH2O;
  }
  if (type.localeCompare("outdoor_UV") == 0) {
    let outdoor_UV = parseInt(obj.value); 
    outdoor_UV=Math.floor(outdoor_UV);
    document.getElementsByClassName("measured_value outdoor_UV")[0].innerHTML=outdoor_UV;
  }
  
}

window.onload = function(event) {
  init();
}