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


void bufferCallback()
{
    Frame frame;
    frame.type = ready; // Upewnij się, że `ready` jest prawidłowym typem
    frame.content_length = 0;

    mqtt->publish("external", (byte *)&frame, sizeof(frame));
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

    animDB = new AnimDB();
    animDB->Print();

    mqtt = new MQTT(wifiClient);

    mqtt->connectToBroker();
    mqtt->subscribe("upper_esp");

    leds_fb_test = (CRGB *)malloc(sizeof(CRGB) * 400 * 900);

    ledArray = new LedArray(bufferCallback);

    for (int i = 0; i < 900; i++)
    {
        for (int j = 0; j < 400; j++)
        {
            leds_fb_test[i * 400 + j] = CRGB::Black;
        }
    }

    for (int i = 0; i < 900; i++)
    {
        leds_fb_test[i * 400 + i % 400] = CRGB::Red;
    }

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

int avgTime = 0;
int count = 0;
uint32_t debugTime = 0;

void loop()
{
    mqtt->loop();
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
