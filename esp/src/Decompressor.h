#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "Debug.h"

class Decompressor
{
public:
    Decompressor();
    ~Decompressor() = default;

    static CRGB getColor(int index)
    {
        unsigned int palette[256][3] = {
            {0, 0, 0},
            {128, 0, 0},
            {0, 128, 0},
            {128, 128, 0},
            {0, 0, 128},
            {128, 0, 128},
            {0, 128, 128},
            {192, 192, 192},
            {128, 128, 128},
            {255, 0, 0},
            {0, 255, 0},
            {255, 255, 0},
            {0, 0, 255},
            {255, 0, 255},
            {0, 255, 255},
            {255, 255, 255},
            {0, 0, 0},
            {0, 0, 95},
            {0, 0, 135},
            {0, 0, 175},
            {0, 0, 215},
            {0, 0, 255},
            {0, 95, 0},
            {0, 95, 95},
            {0, 95, 135},
            {0, 95, 175},
            {0, 95, 215},
            {0, 95, 255},
            {0, 135, 0},
            {0, 135, 95},
            {0, 135, 135},
            {0, 135, 175},
            {0, 135, 215},
            {0, 135, 255},
            {0, 175, 0},
            {0, 175, 95},
            {0, 175, 135},
            {0, 175, 175},
            {0, 175, 215},
            {0, 175, 255},
            {0, 215, 0},
            {0, 215, 95},
            {0, 215, 135},
            {0, 215, 175},
            {0, 215, 215},
            {0, 215, 255},
            {0, 255, 0},
            {0, 255, 95},
            {0, 255, 135},
            {0, 255, 175},
            {0, 255, 215},
            {0, 255, 255},
            {95, 0, 0},
            {95, 0, 95},
            {95, 0, 135},
            {95, 0, 175},
            {95, 0, 215},
            {95, 0, 255},
            {95, 95, 0},
            {95, 95, 95},
            {95, 95, 135},
            {95, 95, 175},
            {95, 95, 215},
            {95, 95, 255},
            {95, 135, 0},
            {95, 135, 95},
            {95, 135, 135},
            {95, 135, 175},
            {95, 135, 215},
            {95, 135, 255},
            {95, 175, 0},
            {95, 175, 95},
            {95, 175, 135},
            {95, 175, 175},
            {95, 175, 215},
            {95, 175, 255},
            {95, 215, 0},
            {95, 215, 95},
            {95, 215, 135},
            {95, 215, 175},
            {95, 215, 215},
            {95, 215, 255},
            {95, 255, 0},
            {95, 255, 95},
            {95, 255, 135},
            {95, 255, 175},
            {95, 255, 215},
            {95, 255, 255},
            {135, 0, 0},
            {135, 0, 95},
            {135, 0, 135},
            {135, 0, 175},
            {135, 0, 215},
            {135, 0, 255},
            {135, 95, 0},
            {135, 95, 95},
            {135, 95, 135},
            {135, 95, 175},
            {135, 95, 215},
            {135, 95, 255},
            {135, 135, 0},
            {135, 135, 95},
            {135, 135, 135},
            {135, 135, 175},
            {135, 135, 215},
            {135, 135, 255},
            {135, 175, 0},
            {135, 175, 95},
            {135, 175, 135},
            {135, 175, 175},
            {135, 175, 215},
            {135, 175, 255},
            {135, 215, 0},
            {135, 215, 95},
            {135, 215, 135},
            {135, 215, 175},
            {135, 215, 215},
            {135, 215, 255},
            {135, 255, 0},
            {135, 255, 95},
            {135, 255, 135},
            {135, 255, 175},
            {135, 255, 215},
            {135, 255, 255},
            {215, 0, 0},
            {215, 0, 95},
            {215, 0, 135},
            {215, 0, 175},
            {215, 0, 215},
            {215, 0, 255},
            {215, 95, 0},
            {215, 95, 95},
            {215, 95, 135},
            {215, 95, 175},
            {215, 95, 215},
            {215, 95, 255},
            {215, 135, 0},
            {215, 135, 95},
            {215, 135, 135},
            {215, 135, 175},
            {215, 135, 215},
            {215, 135, 255},
            {215, 175, 0},
            {215, 175, 95},
            {215, 175, 135},
            {215, 175, 175},
            {215, 175, 215},
            {215, 175, 255},
            {215, 215, 0},
            {215, 215, 95},
            {215, 215, 135},
            {215, 215, 175},
            {215, 215, 215},
            {215, 215, 255},
            {255, 0, 0},
            {255, 0, 95},
            {255, 0, 135},
            {255, 0, 175},
            {255, 0, 215},
            {255, 0, 255},
            {255, 95, 0},
            {255, 95, 95},
            {255, 95, 135},
            {255, 95, 175},
            {255, 95, 215},
            {255, 95, 255},
            {255, 135, 0},
            {255, 135, 95},
            {255, 135, 135},
            {255, 135, 175},
            {255, 135, 215},
            {255, 135, 255},
            {255, 175, 0},
            {255, 175, 95},
            {255, 175, 135},
            {255, 175, 175},
            {255, 175, 215},
            {255, 175, 255},
            {255, 215, 0},
            {255, 215, 95},
            {255, 215, 135},
            {255, 215, 175},
            {255, 215, 215},
            {255, 215, 255},
            {255, 255, 0},
            {255, 255, 95},
            {255, 255, 135},
            {255, 255, 175},
            {255, 255, 215},
            {255, 255, 255},
        };
        return CRGB(palette[index][0], palette[index][1], palette[index][2]);
    }

    inline static void decompress(const uint8_t *input, CRGB *output, uint32_t frame, uint32_t num_of_frames)
    {
        uint32_t o = 0;
        uint32_t i = 0;
        char* buffer = new char[3];
        while (o < num_of_frames)
        {
            while (true)
            {
                int8_t idx;
                CRGB color;
                byte b = input[i++];
                
                
                if (b == 201)
                    is_bigger = true;
                if (b == 202)
                    is_bigger = false;

                idx = input[i];
                if (is_bigger)
                    idx += 200;

                if (idx < last_idx)
                {
                    last_idx = idx;
                    ++frame;
                    ++o;
                    break;
                }

                ++i;
                last_idx = idx;
                
                color = getColor(input[i++]);
                output[o * frame + idx] = color.r;
                output[o * frame + idx + 1] = color.g;
                output[o * frame + idx + 2] = color.b;
            }
        }
        free(buffer);
    }

private:
    inline static uint32_t i = 0;
    inline static bool is_bigger = false;
    inline static uint8_t last_idx = -1;
};