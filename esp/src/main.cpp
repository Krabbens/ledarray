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
#include "State.h"

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

State currentState;

TaskHandle_t* mqttLoopHandle = NULL;

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
    frame.type = ready;
    frame.content_length = 0;

    mqtt->publish("external", (byte *)&frame, sizeof(frame));
}

void prepareMqtt(){
    mqtt->connectToBroker();
    mqtt->subscribe("upper_esp");
    xTaskCreatePinnedToCore(
        mqttLoop,
        "mqttLoop",
        4096,
        NULL,
        2,
        mqttLoopHandle,
        0
    );
}

void setup()
{
    Debug::init();
    Debug::info("Starting...");
    
    wifiClient = new WiFiClientSecure();
    wireless = new Wireless();

    animDB = new AnimDB();
    animDB->print();

    mqtt = new MQTT(wifiClient);
    
    currentState = INIT;
}

int avgTime = 0;
int count = 0;
uint32_t maxTimeX = 0;
uint32_t debugTime = 0;
uint32_t timeX;

State previousState = INIT;

void loop()
{
    switch (currentState)
    {
        case INIT:
        { 
            Debug::info("Initializing...");
            
            if (wireless->isConnected()) {
                Debug::info("WiFi already connected. Switching to READY state.");
                currentState = READY;
            } else {
                Debug::info("WiFi not connected. Switching to CONNECT_WIFI state.");
                currentState = CONNECT_WIFI;
            }
            break;
        }

        case CONNECT_WIFI:
        {
            Debug::info("Trying to connect to WiFi...");
            
            wireless->connectToNetwork();
            
            if (wireless->isConnected()) {
                Debug::info("Successfully connected to WiFi. Switching to READY state.");
                prepareMqtt();
                currentState = READY;
            } else {
                Debug::info("WiFi connection failed. Switching to SETUP_WIFI state.");
                currentState = SETUP_WIFI;
            }
            break;
        }

        case SETUP_WIFI:
        {
            Debug::info("Setting up Access Point (AP)...");
            
            wireless->startAP();

            if (wireless->isConnected()) {
                Debug::info("Successfully connected to WiFi after AP setup. Switching to READY state.");
                wireless->stopAP();
                currentState = READY;
            } else {
                Debug::info("Waiting for WiFi credentials from AP.");
            }
            break;
        }

        case READY:
        {
            Debug::info("System is ready. Waiting for LED animation to start...");
            
            if (ledArray != NULL && ledArray->isReady()) {
                Debug::info("LED animation ready to display. Switching to ANIMATION state.");
                currentState = ANIMATION;
            }
            break;
        }

        case ANIMATION:
        {
            if (ledArray != NULL && ledArray->isReady()) {
                ledArray->nextFrame();

                uint32_t currentTime = micros();
                uint32_t frameTime = currentTime - lastMicros;
                lastMicros = currentTime;

                count++;
                avgTime += frameTime;
                if (maxTimeX < frameTime) maxTimeX = frameTime;

                if (count == 1000) {
                    Debug::info("Average time: " + String(avgTime / count) + " Max time: " + String(maxTimeX));
                    avgTime = 0;
                    count = 0;
                    maxTimeX = 0;
                }
            }
            break;
        }

        case ERROR:
        {
            Debug::error("An error occurred. Restarting WiFi setup...");
            wireless->clearCredentials();
            currentState = CONNECT_WIFI;
            break;
        }
    }
    if (currentState == previousState && currentState != ANIMATION) {
        vTaskDelay(500 / portTICK_PERIOD_MS); 
    }
    previousState = currentState;
}