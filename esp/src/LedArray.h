#include <Arduino.h>
#include <FastLED.h>
#include "Debug.h"

#define NUM_LEDS 40
#define ALL_LEDS NUM_LEDS * 10

class LedArray
{
public:
    LedArray();
    ~LedArray();

    void fillBuffer(CRGB *input); // fills current buffer with info from input
    void nextFrame(); // moves to the next frame
    void swapBuffer(); // swaps the front and back buffers

private:
    CRGB* leds_fb;
    CRGB* leds_bb;
    uint16_t led_index = 0;

    CRGB* buffer_ptr;

    CLEDController* controller_1;
    CLEDController* controller_2;
};

LedArray::~LedArray()
{
    free(leds_fb);
    free(leds_bb);
}

LedArray::LedArray()
{
    leds_fb = (CRGB*)malloc(sizeof(CRGB) * ALL_LEDS * 4 * 30);
    leds_bb = (CRGB*)malloc(sizeof(CRGB) * ALL_LEDS * 4 * 30);
    buffer_ptr = leds_bb;

    controller_1 = &FastLED.addLeds<SK9822, 11, 13, BGR>(buffer_ptr, NUM_LEDS);
    controller_2 = &FastLED.addLeds<SK9822, 10, 12, BGR>(buffer_ptr, NUM_LEDS, NUM_LEDS);

    FastLED.setBrightness(10);
}

void LedArray::fillBuffer(CRGB *leds)
{
    CRGB *inactive_buffer = (buffer_ptr == leds_fb) ? leds_bb : leds_fb;
    memcpy(inactive_buffer, leds, sizeof(CRGB) * ALL_LEDS * 4 * 30);
}

void LedArray::swapBuffer()
{
    buffer_ptr = (buffer_ptr == leds_fb) ? leds_bb : leds_fb;
}

void LedArray::nextFrame()
{
    Debug::raw(" INFO: Next frame: ");
    Debug::raw(led_index);
    Debug::raw("\n");
    FastLED.show();
    buffer_ptr = leds_fb + ALL_LEDS * led_index; // ALL_LEDS * 4 * 30
    
    led_index++;
    
    controller_1->setLeds(buffer_ptr, NUM_LEDS);
    controller_2->setLeds(buffer_ptr + NUM_LEDS, NUM_LEDS);

    if (led_index == 30 * 4)
    {
        led_index = 0;
        swapBuffer();
    }
}