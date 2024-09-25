#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "Decompressor.h"
#include "Debug.h"

#define NUM_LEDS 200
#define ALL_LEDS 400
#define NUM_LINES 2

void decompTask(void *param);

class LedArray
{
public:
    LedArray(uint8_t *file, size_t fileSize);
    ~LedArray();

    void fillBuffer(CRGB *input);
    void nextFrame();
    void swapBuffer();
    bool isReady() { return doneProcessing; }

    CRGB *leds_fb;
    CRGB *leds_bb;
    uint16_t led_index = 0;

    CRGB *buffer_ptr;
    CRGB *other_buffer;
    uint8_t *file;
    size_t fileSize;
    size_t fileOffset = 0;

    bool doneProcessing = false;

private:
    CLEDController *controllers[NUM_LINES];
    
    TaskHandle_t xDecompTaskHandle;
    void startDecompTask();

    
    uint32_t lastMicros = 0;
    uint32_t debugTime = 0;
    int avgTime = 0;
    int count = 0;
};

LedArray::~LedArray()
{
    free(file);
    free(leds_fb);
    free(leds_bb);
}

LedArray::LedArray(uint8_t *file, size_t fileSize)
{
    leds_fb = (CRGB *)malloc(sizeof(CRGB) * 400 * 900);
    leds_bb = (CRGB *)malloc(sizeof(CRGB) * 400 * 900);

    for (int i = 0; i < 900; i++)
    {
        for (int j = 0; j < 400; j++)
        {
            leds_fb[i * 400 + j] = CRGB::Black;
            leds_bb[i * 400 + j] = CRGB::Black;
        }
    }

    buffer_ptr = leds_bb;

    controllers[0] = &FastLED.addLeds<SK9822, 2, 1, RGB, DATA_RATE_MHZ(10), 0>(buffer_ptr, NUM_LEDS);
    controllers[1] = &FastLED.addLeds<SK9822, 4, 3, RGB, DATA_RATE_MHZ(10), 1>(buffer_ptr + NUM_LEDS, NUM_LEDS);

    FastLED.setBrightness(10);

    this->file = file;
    this->fileSize = fileSize;

    swapBuffer();
    startDecompTask();
    swapBuffer();
}

void LedArray::startDecompTask()
{
    xTaskCreatePinnedToCore(
        decompTask,
        "decompTask",
        4096*8,
        this,
        1,
        &xDecompTaskHandle,
        0);
}

void LedArray::fillBuffer(CRGB *leds)
{
    CRGB *inactive_buffer = (buffer_ptr == leds_fb) ? leds_bb : leds_fb;
    memcpy(inactive_buffer, leds, sizeof(CRGB) * 400 * 900);
}

void LedArray::swapBuffer()
{
    buffer_ptr = (buffer_ptr == leds_fb) ? leds_bb : leds_fb;
    other_buffer = (buffer_ptr == leds_fb) ? leds_bb : leds_fb;
}

void LedArray::nextFrame()
{
    if (led_index == 0)
    {
        startDecompTask();
    }
    controllers[0]->setLeds(buffer_ptr, NUM_LEDS);
    controllers[1]->setLeds(buffer_ptr + NUM_LEDS, NUM_LEDS);

    buffer_ptr += ALL_LEDS;
    led_index++;

    int32_t start = micros();

    controllers[0]->showLeds(10);
    controllers[1]->showLeds(10);

    controllers[0]->selectSPI();
    controllers[0]->writePixels();
    controllers[0]->releaseSPI();

    controllers[1]->selectSPI();
    controllers[1]->writePixels();
    controllers[1]->releaseSPI();

    if (led_index == 900)
    {
        led_index = 0;
        swapBuffer();
    }
}

void decompTask(void *param)
{
    LedArray *ledArray = (LedArray *)param;
    Decompressor::decompress(ledArray->file, ledArray->other_buffer, ledArray->fileSize, 400 * 900, ledArray->fileOffset);
    ledArray->doneProcessing = true;
    vTaskDelete(NULL);
}