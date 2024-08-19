#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "Debug.h"

#define NUM_LEDS 40
#define ALL_LEDS NUM_LEDS * 10
#define FRAMES_PER_SEC 40
#define NUM_LINES 10
#define SEC_IN_BUFFER 4

const int clockPin = 4;
const int dataPins[10] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

class LedArray
{
public:
    LedArray(void (*bufferCallback)());
    ~LedArray();

    void fillBuffer(CRGB *input); // fills current buffer with info from input
    void nextFrame(); // moves to the next frame
    void swapBuffer(); // swaps the front and back buffers

private:
    CRGB* leds_fb;
    CRGB* leds_bb;
    uint16_t led_index = 0;

    CRGB* buffer_ptr;

    // CLEDController* controller_1;
    // CLEDController* controller_2;

    CLEDController* controllers[NUM_LINES];

    void (*swapBufferCallback)();
};

LedArray::~LedArray()
{
    free(leds_fb);
    free(leds_bb);
}

LedArray::LedArray(void (*bufferCallback)())
{
    leds_fb = (CRGB*)malloc(sizeof(CRGB) * ALL_LEDS * SEC_IN_BUFFER * FRAMES_PER_SEC);
    leds_bb = (CRGB*)malloc(sizeof(CRGB) * ALL_LEDS * SEC_IN_BUFFER * FRAMES_PER_SEC);
    buffer_ptr = leds_bb;

    // controller_1 = &FastLED.addLeds<SK9822, 11, 13, BGR>(buffer_ptr, NUM_LEDS);
    // controller_2 = &FastLED.addLeds<SK9822, 10, 12, BGR>(buffer_ptr, NUM_LEDS, NUM_LEDS);

    controllers[0] = &FastLED.addLeds<SK9822, 5, clockPin, BGR>(buffer_ptr + NUM_LEDS * 0, NUM_LEDS);
    controllers[1] = &FastLED.addLeds<SK9822, 6, clockPin, BGR>(buffer_ptr + NUM_LEDS * 1, NUM_LEDS);
    controllers[2] = &FastLED.addLeds<SK9822, 7, clockPin, BGR>(buffer_ptr + NUM_LEDS * 2, NUM_LEDS);
    controllers[3] = &FastLED.addLeds<SK9822, 15, clockPin, BGR>(buffer_ptr + NUM_LEDS * 3, NUM_LEDS);
    controllers[4] = &FastLED.addLeds<SK9822, 16, clockPin, BGR>(buffer_ptr + NUM_LEDS * 4, NUM_LEDS);
    controllers[5] = &FastLED.addLeds<SK9822, 17, clockPin, BGR>(buffer_ptr + NUM_LEDS * 5, NUM_LEDS);
    controllers[6] = &FastLED.addLeds<SK9822, 18, clockPin, BGR>(buffer_ptr + NUM_LEDS * 6, NUM_LEDS);
    controllers[7] = &FastLED.addLeds<SK9822, 8, clockPin, BGR>(buffer_ptr + NUM_LEDS * 7, NUM_LEDS);
    controllers[8] = &FastLED.addLeds<SK9822, 3, clockPin, BGR>(buffer_ptr + NUM_LEDS * 8, NUM_LEDS);
    controllers[9] = &FastLED.addLeds<SK9822, 9, clockPin, BGR>(buffer_ptr + NUM_LEDS * 9, NUM_LEDS);

    FastLED.setBrightness(10);

    swapBufferCallback = bufferCallback;
}

void LedArray::fillBuffer(CRGB *leds)
{
    CRGB *inactive_buffer = (buffer_ptr == leds_fb) ? leds_bb : leds_fb;
    memcpy(inactive_buffer, leds, sizeof(CRGB) * ALL_LEDS * SEC_IN_BUFFER * FRAMES_PER_SEC);
}

void LedArray::swapBuffer()
{
    buffer_ptr = (buffer_ptr == leds_fb) ? leds_bb : leds_fb;
    swapBufferCallback();
}

void LedArray::nextFrame()
{
    //Debug::raw(" INFO: Next frame: ");
    //Debug::raw(led_index);
    //Debug::raw("\n");
    FastLED.show();
    buffer_ptr = leds_fb + ALL_LEDS * led_index; // ALL_LEDS * 4 * FRAMES_PER_SEC
    
    led_index++;
    
    // controller_1->setLeds(buffer_ptr, NUM_LEDS);
    // controller_2->setLeds(buffer_ptr + NUM_LEDS, NUM_LEDS);

    for (int i = 0; i < NUM_LINES; i++) {
        controllers[i]->setLeds(buffer_ptr + NUM_LEDS * i, NUM_LEDS);
    }

    if (led_index == FRAMES_PER_SEC * SEC_IN_BUFFER)
    {
        led_index = 0;
        swapBuffer();
    }
}