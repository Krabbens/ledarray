#include <Arduino.h>
#include "Wireless.h"

#define AP_SSID "ledarray"

// put function declarations here:
int myFunction(int, int);
PriorityWireless wireless("ledarray");

void setup() {
  Debug::init();
  Debug::info("Starting...");
  wireless.begin();
}

void loop() {
  
}