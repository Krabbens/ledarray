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
  Debug::init();
  Debug::info("Starting...");
  // wifiClient = new WiFiClientSecure();
  // wireless = new Wireless();
  // while (!wireless->isConnected())
  // {
  //   delay(1000);
  // }
  // mqtt = new MQTT(wifiClient);

  // mqtt->connectToBroker();
  // mqtt->subscribe("esp32/check_alive");

  leds_fb_test = (CRGB *)malloc(sizeof(CRGB) * NUM_LEDS * 10 * 30 * 4);
  // 10 lines of 40 leds, 30 frames, 4 seconds of animation
  // leds_bb_test = (CRGB*)malloc(sizeof(CRGB) * NUM_LEDS * 10 * 30 * 4);

  FastLED.addLeds<SK9822, 11, 13, BGR>(leds_fb_test, NUM_LEDS);
  FastLED.addLeds<SK9822, 10, 12, BGR>(leds_fb_test, NUM_LEDS, NUM_LEDS);

  ledArray = new LedArray();

  // rainbow animation 120 frames of changing color of all leds
  for (int i = 0; i < 120; i++)
  {
    for (int j = 0; j < NUM_LEDS * 10; j++)
    {
      leds_fb_test[i * NUM_LEDS * 10 + j] = CHSV(i * 2, 255, 255);
    }
  }
  

  FastLED.setBrightness(10);
  FastLED.show();

  ledArray->fillBuffer(leds_fb_test);
  ledArray->swapBuffer();
  ledArray->fillBuffer(leds_fb_test);
  ledArray->swapBuffer();

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

void loop()
{
  if (micros() - lastMicros > 10000)
  {
    ledArray->nextFrame();
    Debug::raw("Time: ");
    Debug::raw(micros() - lastMicros);
    lastMicros = micros();
  }
}


