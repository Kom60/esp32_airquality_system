#include "headers.h"
#include "main.h"


void mic_i2s_reader_task(void *parameter)
{
  mic_i2s_init();

  // Discard first block, microphone may have startup time (i.e. INMP441 up to 83ms)
  size_t bytes_read = 0;
  i2s_read(I2S_PORT, &samples, SAMPLES_SHORT * sizeof(int32_t), &bytes_read, portMAX_DELAY);

  while (true)
  {
    // Block and wait for microphone values from I2S
    //
    // Data is moved from DMA buffers to our 'samples' buffer by the driver ISR
    // and when there is requested ammount of data, task is unblocked
    //
    // Note: i2s_read does not care it is writing in float[] buffer, it will write
    //       integer values to the given address, as received from the hardware peripheral.
    i2s_read(I2S_PORT, &samples, SAMPLES_SHORT * sizeof(SAMPLE_T), &bytes_read, portMAX_DELAY);

    TickType_t start_tick = xTaskGetTickCount();

    // Convert (including shifting) integer microphone values to floats,
    // using the same buffer (assumed sample size is same as size of float),
    // to save a bit of memory
    SAMPLE_T *int_samples = (SAMPLE_T *)&samples;
    for (int i = 0; i < SAMPLES_SHORT; i++)
      samples[i] = MIC_CONVERT(int_samples[i]);

    sum_queue_t q;
    // Apply equalization and calculate Z-weighted sum of squares,
    // writes filtered samples back to the same buffer.
    q.sum_sqr_SPL = MIC_EQUALIZER.filter(samples, samples, SAMPLES_SHORT);

    // Apply weighting and calucate weigthed sum of squares
    q.sum_sqr_weighted = WEIGHTING.filter(samples, samples, SAMPLES_SHORT);

    // Debug only. Ticks we spent filtering and summing block of I2S data
    // q.proc_ticks = xTaskGetTickCount() - start_tick;

    // Send the sums to FreeRTOS queue where main task will pick them up
    // and further calcualte decibel values (division, logarithms, etc...)
    xQueueSend(samples_queue, &q, portMAX_DELAY);
  }
}


void setup(void)
{
  Serial.begin(115200);
  htu_setup();
  MS5611_setup();
  bme_setup();
  BH1750_setup();
  SCD40_setup();
  // pms.init();
  PMS_setup();
  CH2O_setup();
  VEML_setup();
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS could not initialize");
  }

  WiFi.begin(ssid, password);
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { // define here wat the webserver needs to do
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "File not found"); });

  server.serveStatic("/", SPIFFS, "/");

  webSocket.begin();                 // start websocket
  webSocket.onEvent(webSocketEvent); // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"

  server.begin();
  // Create FreeRTOS queue
  samples_queue = xQueueCreate(8, sizeof(sum_queue_t));

  // Create the I2S reader FreeRTOS task
  // NOTE: Current version of ESP-IDF will pin the task
  //       automatically to the first core it happens to run on
  //       (due to using the hardware FPU instructions).
  //       For manual control see: xTaskCreatePinnedToCore
  xTaskCreate(mic_i2s_reader_task, "Mic I2S Reader", I2S_TASK_STACK, NULL, I2S_TASK_PRI, NULL);
  xTaskCreatePinnedToCore(INMP441_measurementTaskFunction, "INMP441MeasurementTask", 2048, NULL, 1, &INMP441_measurementTask, 0);
}

void loop()
{
  webSocket.loop();             // Update function for the webSockets
  unsigned long now = millis(); // read out the current "time" ("millis()" gives the time in ms since the Arduino started)
  if ((unsigned long)(now - previousMillis) > interval)
  { // check if "interval" ms has passed since last time the clients were updated
    previousMillis = now;
    tft.fillScreen(TFT_BLACK);
    // MS5611_printValues();
    // pms.read();
    // float lux=0;
    // float temp = htu.readTemperature();
    // float hum = htu.readHumidity();
    // formaldehyde=analogRead(formaldehyde_Pin);
    // MS5611.read();
    // if (lightMeter.measurementReady()) {
    //   lux = lightMeter.readLightLevel();}
    // AIR_data.print_values();
    // AIR_data.update_pms_data(pms.pm01, pms.pm25, pms.pm10);
    // AIR_data.update_htu_data(temp,hum);
    // AIR_data.update_CO2_data(co2Value);
    // AIR_data.update_MS5611_data(MS5611.getPressure());
    // AIR_data.update_BH1750_data(lux);
    // AIR_data.update_CH2O_data(formaldehyde/496.36);
    // AIR_data.update_bme_data(bme.readTemperature(),bme.readPressure() / 100.0F,bme.readHumidity());
    display_bme();
    display_indoor();

    sendJson("cpu_voltage", String(random(360)));

    // sendJson("indoor_temp", String(AIR_data.Indoor_temp*100));
    // sendJson("indoor_press", String(AIR_data.Indoor_pressure*100));
    // sendJson("indoor_humidity", String(AIR_data.Indoor_humidity*100));
    // sendJson("indoor_light", String(AIR_data.Lighting*10));
    // sendJson("indoor_CO2", String(co2Value));
    // sendJson("indoor_CH2O", String(AIR_data.CH2O*10));
    // sendJson("indoor_1_0", String(AIR_data.Indoor_PM1*10));
    // sendJson("indoor_pm2_5", String(AIR_data.Indoor_PM2*10));
    // sendJson("indoor_pm10", String(AIR_data.Indoor_PM10*10));
    sendJson("indoor_radiation", String(0));
    // sendJson("indoor_noise", String(0));

    // sendJson("outdoor_temp", String(AIR_data.Outdoor_temp*100));
    // sendJson("outdoor_press", String(AIR_data.Outdoor_pressure*100));
    // sendJson("outdoor_humidity", String(AIR_data.Outdoor_Humidity*100));
    sendJson("outdoor_light", String(0));
    sendJson("outdoor_CO2", String(0));
    sendJson("outdoor_CH2O", String(0));

    // sendJson("indoor_humidity", String(50+random(50)));
    sendJson("esp32_cpu_freq", String(esp_clk_cpu_freq()));

    // SCD40_printValues();
  }
}

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{ // the parameters of this callback function are always the same -> num: id of the client who send the event, type: type of message, payload: actual data sent and length: length of payload
  switch (type)
  {                         // switch on the type of information sent
  case WStype_DISCONNECTED: // if a client is disconnected, then type == WStype_DISCONNECTED
    Serial.println("Client " + String(num) + " disconnected");
    break;
  case WStype_CONNECTED: // if a client is connected, then type == WStype_CONNECTED
    Serial.println("Client " + String(num) + " connected");

    // send variables to newly connected web client -> as optimization step one could send it just to the new client "num", but for simplicity I left that out here
    sendJson("random_intensity", String(random_intensity));
    sendJsonArray("graph_update", sens_vals);

    break;
  case WStype_TEXT: // if a client has sent data, then type == WStype_TEXT
    // try to decipher the JSON string received
    StaticJsonDocument<200> doc; // create JSON container
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    else
    {
      // JSON string was received correctly, so information can be retrieved:
      const char *l_type = doc["type"];
      const int l_value = doc["value"];
      Serial.println("Type: " + String(l_type));
      Serial.println("Value: " + String(l_value));

      // if random_intensity value is received -> update and write to all web clients
      if (String(l_type) == "random_intensity")
      {
        random_intensity = int(l_value);
        sendJson("random_intensity", String(l_value));
      }
    }
    Serial.println("");
    break;
  }
}

// Simple function to send information to the web clients
void sendJson(String l_type, String l_value)
{
  String jsonString = "";                   // create a JSON string for sending data to the client
  StaticJsonDocument<200> doc;              // create JSON container
  JsonObject object = doc.to<JsonObject>(); // create a JSON Object
  object["type"] = l_type;                  // write data into the JSON object
  object["value"] = l_value;
  serializeJson(doc, jsonString);     // convert JSON object to string
  webSocket.broadcastTXT(jsonString); // send JSON string to all clients
}

// Simple function to send information to the web clients
void sendJsonArray(String l_type, float l_array_values[])
{
  String jsonString = ""; // create a JSON string for sending data to the client
  const size_t CAPACITY = JSON_ARRAY_SIZE(ARRAY_LENGTH) + 100;
  StaticJsonDocument<CAPACITY> doc; // create JSON container

  JsonObject object = doc.to<JsonObject>(); // create a JSON Object
  object["type"] = l_type;                  // write data into the JSON object
  JsonArray value = object.createNestedArray("value");
  for (int i = 0; i < ARRAY_LENGTH; i++)
  {
    value.add(l_array_values[i]);
  }
  serializeJson(doc, jsonString);     // convert JSON object to string
  webSocket.broadcastTXT(jsonString); // send JSON string to all clients
}
