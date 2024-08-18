#include <Arduino.h>
#include <FastLED.h>
#include "Debug.h"

#define NUM_LEDS 200
#define ALL_LEDS 400
#define NUM_LINES 2

const int clockPin = 4;
const int dataPins[10] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

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

    // CLEDController* controller_1;
    // CLEDController* controller_2;

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

    // controller_1 = &FastLED.addLeds<SK9822, 11, 13, BGR>(buffer_ptr, NUM_LEDS);
    // controller_2 = &FastLED.addLeds<SK9822, 10, 12, BGR>(buffer_ptr, NUM_LEDS, NUM_LEDS);

    controllers[0] = &FastLED.addLeds<SK9822, 2, 1, RGB>(buffer_ptr, NUM_LEDS);
    controllers[1] = &FastLED.addLeds<SK9822, 4, 3, RGB>(buffer_ptr + NUM_LEDS, NUM_LEDS);
    //controllers[2] = &FastLED.addLeds<SK9822, 6, 5, BGR>(buffer_ptr + NUM_LEDS * 2, NUM_LEDS);
    //controllers[3] = &FastLED.addLeds<SK9822, 8, 7, BGR>(buffer_ptr + NUM_LEDS * 3, NUM_LEDS);
    // controllers[4] = &FastLED.addLeds<SK9822, 10, 9, BGR>(buffer_ptr + NUM_LEDS * 4, NUM_LEDS);
    // controllers[5] = &FastLED.addLeds<SK9822, 12, 11, BGR>(buffer_ptr + NUM_LEDS * 5, NUM_LEDS);
    // controllers[6] = &FastLED.addLeds<SK9822, 14, 13, BGR>(buffer_ptr + NUM_LEDS * 6, NUM_LEDS);
    // controllers[7] = &FastLED.addLeds<SK9822, 16, 15, BGR>(buffer_ptr + NUM_LEDS * 7, NUM_LEDS);
    // controllers[8] = &FastLED.addLeds<SK9822, 18, 17, BGR>(buffer_ptr + NUM_LEDS * 8, NUM_LEDS);
    // controllers[9] = &FastLED.addLeds<SK9822, 48, 47, BGR>(buffer_ptr + NUM_LEDS * 9, NUM_LEDS);

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
    // Debug::raw(" INFO: Next frame: ");
    // Debug::raw(led_index);

    

    //FastLED.show();
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

    // Debug::raw(" INFO: Show time: ");
    // Debug::raw(micros() - start);

    // Debug::raw(" INFO: Setting leds: ");
    // Debug::raw(led_index);
    // Debug::raw("\n");

    if (led_index == 900)
    {
        led_index = 0;
        swapBuffer();
    }
}