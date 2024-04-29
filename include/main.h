#include "headers.h"


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

// global variables of the LED selected and the intensity of that LED
int random_intensity = 5;



// We want to periodically send values to the clients, so we need to define an "interval" and remember the last time we sent data to the client (with "previousMillis")
int interval = 10000;              // send data to the client every 1000ms -> 1s
unsigned long previousMillis = 0; // we use the "millis()" command for time reference and this will output an unsigned long

// Initialization of webserver and websocket
AsyncWebServer server(80);                         // the server uses port 80 (standard port for websites
WebSocketsServer webSocket = WebSocketsServer(81); // the websocket uses port 81 (standard port for websockets*/

const int ARRAY_LENGTH = 10;
float sens_vals[ARRAY_LENGTH];


TaskHandle_t INMP441_measurementTask;
// Calculate reference amplitude value at compile time
constexpr double MIC_REF_AMPL = pow(10, double(MIC_SENSITIVITY)/20) * ((1<<(MIC_BITS-1))-1);
//No_IIR_Filter DC_BLOCKER;
#define MIC_EQUALIZER     INMP441  

QueueHandle_t samples_queue;

// Static buffer for block of samples
float samples[SAMPLES_SHORT] __attribute__((aligned(4)));

extern "C" {
  float sos_filter_sum_sqr_f32(float *input, float *output, int len, const SOS_Coefficients &coeffs, SOS_Delay_State &w, float gain);
}
__asm__ (
  //
  // ESP32 implementation of IIR Second-Order section filter with applied gain.
  // Assumes a0 and b0 coefficients are one (1.0)
  // Returns sum of squares of filtered samples
  //
  // float* a2 = input;
  // float* a3 = output;
  // int    a4 = len;
  // float* a5 = coeffs;
  // float* a6 = w;
  // float  a7 = gain;
  //
  ".text                    \n"
  ".align  4                \n"
  ".global sos_filter_sum_sqr_f32 \n"
  ".type   sos_filter_sum_sqr_f32,@function \n"
  "sos_filter_sum_sqr_f32:  \n"
  "  entry   a1, 16         \n" 
  "  lsi     f0, a5, 0      \n"  // float f0 = coeffs.b1;
  "  lsi     f1, a5, 4      \n"  // float f1 = coeffs.b2;
  "  lsi     f2, a5, 8      \n"  // float f2 = coeffs.a1;
  "  lsi     f3, a5, 12     \n"  // float f3 = coeffs.a2;
  "  lsi     f4, a6, 0      \n"  // float f4 = w[0];
  "  lsi     f5, a6, 4      \n"  // float f5 = w[1];
  "  wfr     f6, a7         \n"  // float f6 = gain;
  "  const.s f10, 0         \n"  // float sum_sqr = 0;
  "  loopnez a4, 1f         \n"  // for (; len>0; len--) {
  "    lsip    f7, a2, 4    \n"  //   float f7 = *input++;
  "    madd.s  f7, f2, f4   \n"  //   f7 += f2 * f4; // coeffs.a1 * w0
  "    madd.s  f7, f3, f5   \n"  //   f7 += f3 * f5; // coeffs.a2 * w1;
  "    mov.s   f8, f7       \n"  //   f8 = f7; // b0 assumed 1.0
  "    madd.s  f8, f0, f4   \n"  //   f8 += f0 * f4; // coeffs.b1 * w0;
  "    madd.s  f8, f1, f5   \n"  //   f8 += f1 * f5; // coeffs.b2 * w1; 
  "    mul.s   f9, f8, f6   \n"  //   f9 = f8 * f6;  // f8 * gain -> result
  "    ssip    f9, a3, 4    \n"  //   *output++ = f9;
  "    mov.s   f5, f4       \n"  //   f5 = f4; // w1 = w0
  "    mov.s   f4, f7       \n"  //   f4 = f7; // w0 = f7;
  "    madd.s  f10, f9, f9  \n"  //   f10 += f9 * f9; // sum_sqr += f9 * f9;
  "  1:                     \n"  // }
  "  ssi     f4, a6, 0      \n"  // w[0] = f4;
  "  ssi     f5, a6, 4      \n"  // w[1] = f5;
  "  rfr     a2, f10        \n"  // return sum_sqr; 
  "  retw.n                 \n"  // 
);

extern "C" {
  int sos_filter_f32(float *input, float *output, int len, const SOS_Coefficients &coeffs, SOS_Delay_State &w);
} 
__asm__ (
  //
  // ESP32 implementation of IIR Second-Order Section filter 
  // Assumes a0 and b0 coefficients are one (1.0)
  //
  // float* a2 = input;
  // float* a3 = output;
  // int    a4 = len;
  // float* a5 = coeffs;
  // float* a6 = w; 
  // float  a7 = gain;
  //
  ".text                    \n"
  ".align  4                \n"
  ".global sos_filter_f32   \n"
  ".type   sos_filter_f32,@function\n"
  "sos_filter_f32:          \n"
  "  entry   a1, 16         \n"
  "  lsi     f0, a5, 0      \n" // float f0 = coeffs.b1;
  "  lsi     f1, a5, 4      \n" // float f1 = coeffs.b2;
  "  lsi     f2, a5, 8      \n" // float f2 = coeffs.a1;
  "  lsi     f3, a5, 12     \n" // float f3 = coeffs.a2;
  "  lsi     f4, a6, 0      \n" // float f4 = w[0];
  "  lsi     f5, a6, 4      \n" // float f5 = w[1];
  "  loopnez a4, 1f         \n" // for (; len>0; len--) { 
  "    lsip    f6, a2, 4    \n" //   float f6 = *input++;
  "    madd.s  f6, f2, f4   \n" //   f6 += f2 * f4; // coeffs.a1 * w0
  "    madd.s  f6, f3, f5   \n" //   f6 += f3 * f5; // coeffs.a2 * w1
  "    mov.s   f7, f6       \n" //   f7 = f6; // b0 assumed 1.0
  "    madd.s  f7, f0, f4   \n" //   f7 += f0 * f4; // coeffs.b1 * w0
  "    madd.s  f7, f1, f5   \n" //   f7 += f1 * f5; // coeffs.b2 * w1 -> result
  "    ssip    f7, a3, 4    \n" //   *output++ = f7;
  "    mov.s   f5, f4       \n" //   f5 = f4; // w1 = w0
  "    mov.s   f4, f6       \n" //   f4 = f6; // w0 = f6
  "  1:                     \n" // }
  "  ssi     f4, a6, 0      \n" // w[0] = f4;
  "  ssi     f5, a6, 4      \n" // w[1] = f5;
  "  movi.n   a2, 0         \n" // return 0;
  "  retw.n                 \n"
);
//
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


