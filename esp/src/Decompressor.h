#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <lz4.h>
#include "Debug.h"
#include <cstring> 

#define BLOCK_SIZE 400

class Decompressor
{
public:
    Decompressor();
    ~Decompressor() = default;

    static unsigned int palette[256][3];

    static CRGB getColor(int index)
    {
        return CRGB(palette[index][0], palette[index][1], palette[index][2]);
    }

    inline static void decompress(const uint8_t *input, CRGB *output, size_t inputSize, size_t outputCapacity, size_t& offset)
    {
        
        int start = micros();

        uint16_t size;
        size_t total_size = 0;
        while(total_size < outputCapacity){
            if(offset + sizeof(size) > inputSize) offset = 0;
            std::memcpy(&size, (input + offset), sizeof(size));
            offset+=sizeof(size);
            int decompressed_size = LZ4_decompress_safe((char*)(input + offset), (char*)(bytes_buff), size, BLOCK_SIZE);
            if (decompressed_size != BLOCK_SIZE) {
                Debug::error("ERROR: "+ String(decompressed_size));
                return;
            }
            for(int j = 0; j < 400; j++){
                colors_buff[j] = getColor(bytes_buff[j]);
            }
            offset += size;
            total_size += decompressed_size;
        }

        Debug::info("Decompressed {" + String(micros() - start) + " us}");
    }

private:
    inline static uint8_t bytes_buff[400];
    inline static CRGB colors_buff[400];
};

unsigned int Decompressor::palette[256][3] = {
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