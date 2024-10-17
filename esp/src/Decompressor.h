#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <lz4.h>
#include "Debug.h"
#include <cstring> 

#define BLOCK_SIZE 400
#define COLOR_BLOCK_SIZE BLOCK_SIZE * sizeof(CRGB)

class Decompressor
{
public:
    Decompressor();
    ~Decompressor() = default;

    inline static void decompress(const uint8_t *input, CRGB *output, size_t inputSize, size_t outputCapacity, size_t& offset)
    {
        int start = micros();
        uint16_t size;
        size_t total_size = 0;

        while(total_size < outputCapacity){

            if (offset + sizeof(size) > inputSize) {
                offset = 0;
            }

            std::memcpy(&size, (input + offset), sizeof(size));
            offset += sizeof(size);

            if (offset + size > inputSize) {
                Debug::error("ERROR: Not enought data to decode. Offset: " + String(offset) + " Size: " + String(size));
                return;
            }

            char* output_end = (char*)(output + total_size / sizeof(CRGB));

            int decompressed_size = LZ4_decompress_safe((char*)(input + offset), output_end, size, outputCapacity - total_size);
            
            if (decompressed_size != COLOR_BLOCK_SIZE) {
                Debug::error("ERROR: Compression error! Offset: " + String(offset) + " Decompressed: "+ String(decompressed_size) + " / " + String(size));
                return;
            }

            if (total_size + decompressed_size > outputCapacity) {
                Debug::error("ERROR: Output buffer overload. Total size: " + String(total_size));
                return;
            }

            offset += size;
            total_size += decompressed_size;
            ++frame_count;
        }
        frame_count = 0;
    }

private:
    inline static int frame_count = 0;
};