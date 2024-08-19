#pragma once
#include <Arduino.h>

enum FrameType{
    animation = 0,
    check_alive = 1,
    alive_status = 2,
    ready = 3,
    buffer_size = 4
};

struct Frame
{
    FrameType type; // 0 = animation
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