#include "webheaders.h"
#include "headers.h"

// global variables of the LED selected and the intensity of that LED
int random_intensity = 5;

const int ARRAY_LENGTH=10;
float sens_vals[ARRAY_LENGTH];

AsyncWebServer server(80);                         // the server uses port 80 (standard port for websites
WebSocketsServer webSocket = WebSocketsServer(81); // the websocket uses port 81 (standard port for websockets*/

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{ 
  switch (type)
  {                         
  case WStype_DISCONNECTED:
    Serial.println("Client " + String(num) + " disconnected");
    break;
  case WStype_CONNECTED:
    Serial.println("Client " + String(num) + " connected");

    // send variables to newly connected web client
    // BME280 sensor data
    sendJson("bme_temperature", String(AIR_data.bme_temperature * 100));
    sendJson("bme_pressure", String(AIR_data.bme_pressure));  // уже в гПа
    sendJson("bme_humidity", String(AIR_data.bme_humidity * 100));

    // HTU21DF sensor data
    sendJson("htu_temperature", String(AIR_data.htu_temperature * 100));
    sendJson("htu_humidity", String(AIR_data.htu_humidity * 100));

    // SCD4X sensor data
    sendJson("scd4x_co2", String(AIR_data.scd4x_co2));
    sendJson("scd4x_temperature", String(AIR_data.scd4x_temperature * 100));
    sendJson("scd4x_humidity", String(AIR_data.scd4x_humidity * 100));

    // PMS sensor data
    sendJson("pms_pm1", String(AIR_data.pms_pm1 * 10));
    sendJson("pms_pm2_5", String(AIR_data.pms_pm2_5 * 10));
    sendJson("pms_pm10", String(AIR_data.pms_pm10 * 10));

    // MS5611 sensor data
    sendJson("ms5611_pressure", String(AIR_data.ms5611_pressure));  // уже в гПа
    sendJson("ms5611_temperature", String(AIR_data.ms5611_temperature * 100));

    // BH1750 sensor data
    sendJson("bh1750_lighting", String(AIR_data.bh1750_lighting * 10));

    // VEML6070 sensor data
    sendJson("veml_uv", String(AIR_data.veml_uv));

    // CH2O sensor data
    sendJson("ch2o_value", String(AIR_data.ch2o_value * 10));

    // Microphone data
    sendJson("microphone_noise", String(AIR_data.microphone_noise * 10));
    
    // ESP32 system data
    sendJson("esp32_cpu_freq", String(esp_clk_cpu_freq()));
    sendJson("esp32_cpu_temp", String(temperatureRead() * 100));

    break;
  case WStype_TEXT:
    // try to decipher the JSON string received
    StaticJsonDocument<200> doc;
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