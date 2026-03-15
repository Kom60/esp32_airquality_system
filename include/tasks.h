#define I2S_TASK_PRI 4
#define I2S_TASK_STACK 2048
#define WEBSOCKET_TASK_STACK 4096
#define WEBSOCKET_TASK_PRI 2

extern TaskHandle_t webSocketTaskHandle;

void CO2_measurementTaskFunction(void *parameter);
void BME_measurementTaskFunction(void *parameter);
void HTU_measurementTaskFunction(void *parameter);
void BH1750_measurementTaskFunction(void *parameter);
void CH2O_measurementTaskFunction(void *parameter);
void PMS_measurementTaskFunction(void *parameter);
void MS5611_measurementTaskFunction(void *parameter);
void VEML_measurementTaskFunction(void *parameter);
void DISPLAY_measurementTaskFunction(void *parameter);
void microphone_init(void);
void webSocketTaskFunction(void *parameter);

