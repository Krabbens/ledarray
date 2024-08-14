#include <Arduino.h>

struct Frame
{
    uint8_t type; // 0 = animation
    uint32_t content_length; // content length in bytes
};

struct AnimationFrame
{
    uint16_t frame;
    uint16_t index;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};