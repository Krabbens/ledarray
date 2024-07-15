#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "Debug.h"
#include "Wireless.h"
#include "MQTT.h"

#define D_LOG

WiFiClientSecure* wifiClient;
Wireless* wireless;
MQTT* mqtt;

void setup() {
  Debug::init();
  Debug::info("Starting...");
  wifiClient = new WiFiClientSecure();
  wireless = new Wireless();
  while(!wireless->isConnected()) {
    delay(1000);
  }
  mqtt = new MQTT(wifiClient);
  
  mqtt->connectToBroker();
  mqtt->subscribe("esp32/ledarray");
  mqtt->publish("esp32/ledarray", "Hello from ESP32!");
}

void loop() {
  mqtt->loop();
}