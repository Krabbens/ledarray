#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "Debug.h"
#include "Wireless.h"
#include "MQTT.h"

#define D_LOG

WiFiClientSecure* wifiClient;
Wireless* wireless;
MQTT* mqtt;

static void callback(char* topic, byte* payload, unsigned int length) {
  Debug::info("Message arrived [" + String(topic) + "]");
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Debug::info("Message: " + message);
  mqtt->publish("esp32/alive_status", "1");
}

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
  mqtt->subscribe("esp32/check_alive");
  mqtt->publish("esp32/alive_status", "1");
}

void loop() {
  mqtt->loop();
}