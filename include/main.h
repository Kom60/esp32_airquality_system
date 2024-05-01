#include "headers.h"
#include "microphone.h"
#include "webheaders.h"

SCD4X co2;
double co2Value = 0, temperature = 0, humidity = 0;

const int formaldehyde_Pin = 34;


SerialPM pms(PMSx003, 17,16); // PMSx003, UART
Adafruit_HTU21DF htu = Adafruit_HTU21DF(); //0x40 adress
MS5611 ms5611;
BH1750 lightMeter(0x23);
Adafruit_BME280 bme; // I2C 0x77 adress
Adafruit_VEML6070 uv = Adafruit_VEML6070();


TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

Meteo_data AIR_data = Meteo_data();


unsigned long delayTime;

// SSID and password of Wifi connection:
const char *ssid = "SAFARI";
const char *password = "133713371337";

// Configure IP addresses of the local access point
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 2);
IPAddress subnet(255, 255, 255, 0);




// We want to periodically send values to the clients, so we need to define an "interval" and remember the last time we sent data to the client (with "previousMillis")
int interval = 10000;              // send data to the client every 1000ms -> 1s
unsigned long previousMillis = 0; // we use the "millis()" command for time reference and this will output an unsigned long





TaskHandle_t INMP441_measurementTask;
// Calculate reference amplitude value at compile time
//constexpr double MIC_REF_AMPL = pow(10, double(MIC_SENSITIVITY)/20) * ((1<<(MIC_BITS-1))-1);
//No_IIR_Filter DC_BLOCKER;
#define MIC_EQUALIZER     INMP441  

QueueHandle_t samples_queue;

// Static buffer for block of samples
float samples[SAMPLES_SHORT] __attribute__((aligned(4)));

extern "C" {
  float sos_filter_sum_sqr_f32(float *input, float *output, int len, const SOS_Coefficients &coeffs, SOS_Delay_State &w, float gain);
}


extern "C" {
  int sos_filter_f32(float *input, float *output, int len, const SOS_Coefficients &coeffs, SOS_Delay_State &w);
} 

// I2S Reader Task
//
// Rationale for separate task reading I2S is that IIR filter
// processing cam be scheduled to different core on the ESP32
// while main task can do something else, like update the 
// display in the example
//
// As this is intended to run as separate hihg-priority task, 
// we only do the minimum required work with the I2S data
// until it is 'compressed' into sum of squares 
//
// FreeRTOS priority and stack size (in 32-bit words) 
#define I2S_TASK_PRI   4
#define I2S_TASK_STACK 2048
//
void mic_i2s_reader_task(void* parameter);


