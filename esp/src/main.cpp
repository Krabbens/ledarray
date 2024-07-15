#include <Arduino.h>
#include "Debug.h"
#include "Wireless.h"

#define D_LOG

void setup() {
  Serial.begin(115200);
  delay(4000);

  Debug::info("Starting...");

  Wireless* wireless = new Wireless();
}

void loop() {
}