#include "webheaders.h"
#include "headers.h"

// global variables of the LED selected and the intensity of that LED
int random_intensity = 5;

const int ARRAY_LENGTH=10;
float sens_vals[ARRAY_LENGTH];

AsyncWebServer server(80);                         // the server uses port 80 (standard port for websites
WebSocketsServer webSocket = WebSocketsServer(81); // the websocket uses port 81 (standard port for websockets*/

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
    sendJson("outdoor_temp", String(AIR_data.Outdoor_temp * 100));
		sendJson("outdoor_press", String(AIR_data.Outdoor_pressure * 100));
		sendJson("outdoor_humidity", String(AIR_data.Outdoor_Humidity * 100));

    sendJson("indoor_temp", String(AIR_data.Indoor_temp * 100));
		sendJson("indoor_humidity", String(AIR_data.Indoor_humidity * 100));

    sendJson("indoor_light", String(AIR_data.Lighting * 10));
    sendJson("indoor_CH2O", String(AIR_data.CH2O * 10));
    sendJson("indoor_CO2", String(co2Value));
    sendJson("indoor_1_0", String(pms.pm01 * 10));
		sendJson("indoor_pm2_5", String(pms.pm25 * 10));
		sendJson("indoor_pm10", String(pms.pm10 * 10));

    sendJson("indoor_press", String(AIR_data.Indoor_pressure));
    sendJson("outdoor_UV", String(AIR_data.Outdoor_UV));
    sendJson("indoor_noise", String(10.0*AIR_data.Indoor_noise));

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