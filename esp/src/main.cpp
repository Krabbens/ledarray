#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <FastLED.h>
#include "Debug.h"
#include "Wireless.h"
#include "MQTT.h"
#include "LedArray.h"
#include "AnimDB.h"
#include "FS.h"
#include "SPIFFS.h"

#define D_LOG

WiFiClientSecure *wifiClient;
Wireless *wireless;
MQTT *mqtt;
LedArray *ledArray;
AnimDB *animDB;

uint32_t maxTime = 0;
uint32_t lastMicros = micros();

CRGB *leds_fb_test;
CRGB *leds_bb_test;

void mqttLoop(void *parameter)
{
  while (true)
  {
    mqtt->loop();
  }
}

void bufferCallback(){
  Frame frame;
    frame.type = ready; // Upewnij się, że `ready` jest prawidłowym typem
    frame.content_length = 0;
    
    mqtt->publish("external", (byte*)&frame, sizeof(frame));
}

void setup()
{
  Debug::init();
  Debug::info("Starting...");
  wifiClient = new WiFiClientSecure();
  wireless = new Wireless();
  while (!wireless->isConnected())
  {
    delay(1000);
  }
  mqtt = new MQTT(wifiClient);

  mqtt->connectToBroker();
  mqtt->subscribe("upper_esp");

  leds_fb_test = (CRGB *)malloc(sizeof(CRGB) * NUM_LEDS * NUM_LINES * FRAMES_PER_SEC * SEC_IN_BUFFER);
  // 10 lines of 40 leds, 30 frames, 4 seconds of animation
  // leds_bb_test = (CRGB*)malloc(sizeof(CRGB) * NUM_LEDS * 10 * 30 * 4);

  ledArray = new LedArray(bufferCallback);

  // rainbow animation 120 frames of changing color of all leds
  for (int i = 0; i < FRAMES_PER_SEC * SEC_IN_BUFFER; i++)
  {
    for (int j = 0; j < NUM_LEDS * NUM_LINES; j++)
    {
      leds_fb_test[i * NUM_LEDS * NUM_LINES + j] = CHSV(i * 2, 255, 255);
    }
  }
  

  FastLED.setBrightness(10);
  FastLED.show();

  ledArray->fillBuffer(leds_fb_test);
  ledArray->swapBuffer();
  ledArray->fillBuffer(leds_fb_test);
  ledArray->swapBuffer();

  xTaskCreatePinnedToCore(
      mqttLoop,
      "mqttLoop",
      409600,
      NULL,
      1,
      NULL,
      0
  );



  Debug::info("TESTING ANIMDB\n");
  if(!FileManager::begin()){
    Debug::error("An error occurred while mounting SPIFFS");
  }
  const char test[20] = "hello world";
  animDB = new AnimDB();
  animDB->Print();
  bool err = animDB->addAnimation("first", (byte*)test, sizeof(test));
  animDB->Print();
  if(err) {
    size_t size = animDB->getAnimationSize("first");
    if(size > 0){
      char* test1 = (char*)malloc(size);
      err = animDB->getAnimation("first", (byte*)test1, size);
      Debug::info("TEST RESULT: "+ String(test1));
    }
  }
  animDB->Print();
  animDB->Clear();
}

void loop()
{
  mqtt->loop();

  ledArray->nextFrame();
}


