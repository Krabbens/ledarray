#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS FSPI
#include <SPI.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <FastLED.h>
#include "Debug.h"
#include "Wireless.h"
#include "MQTT.h"
#include "LedArray.h"

#define D_LOG

WiFiClientSecure *wifiClient;
Wireless *wireless;
MQTT *mqtt;
LedArray *ledArray;

uint32_t maxTime = 0;
uint32_t lastMicros = micros();

CRGB *leds_fb_test;
CRGB *leds_bb_test;

void setup()
{
  //Debug::init();
  //Debug::info("Starting...");
  // wifiClient = new WiFiClientSecure();
  // wireless = new Wireless();
  // while (!wireless->isConnected())
  // {
  //   delay(1000);
  // }
  // mqtt = new MQTT(wifiClient);

  // mqtt->connectToBroker();
  // mqtt->subscribe("esp32/check_alive");

  leds_fb_test = (CRGB *)malloc(sizeof(CRGB) * 400 * 900);
  // 10 lines of 40 leds, 30 frames, 4 seconds of animation
  // leds_bb_test = (CRGB*)malloc(sizeof(CRGB) * NUM_LEDS * 10 * 30 * 4);

  ledArray = new LedArray();
  

  // ledArray->fillBuffer(leds_fb_test);
  // ledArray->swapBuffer();
  // ledArray->fillBuffer(leds_bb_test);
  // ledArray->swapBuffer();

  // xTaskCreatePinnedToCore(
  //     mqttLoop,
  //     "mqttLoop",
  //     409600,
  //     NULL,
  //     1,
  //     NULL,
  //     0
  // );
}

// void mqttLoop(void *parameter)
// {
//   while (true)
//   {
//     mqtt->loop();
//   }
// }

int avgTime = 0;
int count = 0;
uint32_t debugTime = 0;

void loop()
{
    lastMicros = micros();
    ledArray->nextFrame();
    debugTime = micros();
    count++;
    avgTime += debugTime - lastMicros;
    if (count == 10000)
    {
      Debug::raw(" INFO: Average time: ");
      Debug::raw(avgTime / count);
      Debug::raw("\n");
      avgTime = 0;
      count = 0;
    }
}


