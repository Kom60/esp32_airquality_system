#include "headers.h"
#include "main.h"

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
    display_bme();
    display_indoor();

    sendJson("cpu_voltage", String(random(360)));
    sendJson("indoor_radiation", String(0));
    sendJson("outdoor_light", String(0));
    sendJson("outdoor_CO2", String(0));
    sendJson("outdoor_CH2O", String(0));
    sendJson("esp32_cpu_freq", String(esp_clk_cpu_freq()));
  }
}
