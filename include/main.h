
SCD4X co2;
double co2Value = 0, temperature = 0, humidity = 0;
TaskHandle_t measurementTask;

const int formaldehyde_Pin = 34;
float formaldehyde=0.0;

SerialPM pms(PMSx003, 17,16); // PMSx003, UART
Adafruit_HTU21DF htu = Adafruit_HTU21DF(); //0x40 adress
//MS5611 MS5611(0x77);
BH1750 lightMeter(0x23);
Adafruit_BME280 bme; // I2C 0x77 adress


TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

Meteo_data AIR_data = Meteo_data();

int i=true;


unsigned long delayTime;

// SSID and password of Wifi connection:
const char *ssid = "SAFARI";
const char *password = "133713371337";

// Configure IP addresses of the local access point
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 2);
IPAddress subnet(255, 255, 255, 0);

// global variables of the LED selected and the intensity of that LED
int random_intensity = 5;

const int ARRAY_LENGTH = 10;
float sens_vals[ARRAY_LENGTH];

// We want to periodically send values to the clients, so we need to define an "interval" and remember the last time we sent data to the client (with "previousMillis")
int interval = 10000;              // send data to the client every 1000ms -> 1s
unsigned long previousMillis = 0; // we use the "millis()" command for time reference and this will output an unsigned long

// Initialization of webserver and websocket
AsyncWebServer server(80);                         // the server uses port 80 (standard port for websites
WebSocketsServer webSocket = WebSocketsServer(81); // the websocket uses port 81 (standard port for websockets*/


