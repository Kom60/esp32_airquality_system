#include <TFT_eSPI.h>       // Include the graphics library
#include <WiFi.h>
#include "headers.h"

extern TFT_eSPI tft;  // Create object "tft"

void display_setup();
void show_init_animation();
void display_all_data();