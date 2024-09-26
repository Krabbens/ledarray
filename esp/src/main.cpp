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
    Debug::info("Mqtt loop started...");
    while (true)
    {
        mqtt->loop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
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
    animDB->print();

    mqtt = new MQTT(wifiClient);

    mqtt->connectToBroker();
    mqtt->subscribe("upper_esp");

    xTaskCreatePinnedToCore(
        mqttLoop,
        "mqttLoop",
        4096,
        NULL,
        2,
        NULL,
        0
    );
}

int avgTime = 0;
int count = 0;
uint32_t maxTimeX = 0;
uint32_t debugTime = 0;
uint32_t timeX;

void loop()
{
    if(debugTime == 0) lastMicros = micros();
    if (ledArray != NULL && ledArray->isReady())
    {
        ledArray->nextFrame();
        debugTime = micros();
        timeX = debugTime - lastMicros;
        lastMicros = debugTime;
        count++;
        avgTime += timeX;
        if(maxTimeX < timeX) maxTimeX = timeX;
        if (count == 1000)
        {
            Debug::info("Average time: " + String(avgTime / count) + " Max time: " + String(maxTimeX));
            avgTime = 0;
            count = 0;
            maxTimeX = 0;
        }
    }
}
