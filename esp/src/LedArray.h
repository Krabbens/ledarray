#include <Arduino.h>
#include <FastLED.h>
#include "Debug.h"

#define NUM_LEDS 200
#define ALL_LEDS 400
#define NUM_LINES 2

class LedArray
{
public:
    LedArray();
    ~LedArray();

    void fillBuffer(CRGB *input);
    void nextFrame();
    void swapBuffer();

private:
    CRGB* leds_fb;
    CRGB* leds_bb;
    uint16_t led_index = 0;

    CRGB* buffer_ptr;

    CLEDController* controllers[NUM_LINES];
};

LedArray::~LedArray()
{
    free(leds_fb);
    free(leds_bb);
}

LedArray::LedArray()
{
    leds_fb = (CRGB*)malloc(sizeof(CRGB) * 400 * 900);
    leds_bb = (CRGB*)malloc(sizeof(CRGB) * 400 * 900);
    buffer_ptr = leds_bb;

    controllers[0] = &FastLED.addLeds<SK9822, 2, 1, RGB, DATA_RATE_MHZ(24), 0>(buffer_ptr, NUM_LEDS);
    controllers[1] = &FastLED.addLeds<SK9822, 4, 3, RGB, DATA_RATE_MHZ(24), 1>(buffer_ptr + NUM_LEDS, NUM_LEDS);

    FastLED.setBrightness(10);
}

void LedArray::fillBuffer(CRGB *leds)
{
    CRGB *inactive_buffer = (buffer_ptr == leds_fb) ? leds_bb : leds_fb;
    memcpy(inactive_buffer, leds, sizeof(CRGB) * 400 * 900);
}

void LedArray::swapBuffer()
{
    buffer_ptr = (buffer_ptr == leds_fb) ? leds_bb : leds_fb;
    buffer_ptr = leds_fb;
}

void LedArray::nextFrame()
{
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