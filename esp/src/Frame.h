#pragma once
#include <Arduino.h>

enum FrameType{
    animation = 0,
    check_alive = 1,
    alive_status = 2,
    ready = 3,
    buffer_size = 4,
    animation_add = 5,
    animation_remove = 6,
    animation_get = 7,
    animation_play = 8,
    animation_clear = 9,
    animation_names = 10,
    get_size = 11,
    info_size = 12,
    animation_stop = 13,
};

struct Frame
{
    FrameType type; // 0 = animation
    uint32_t content_length; // content length in bytes
};

struct SizeInfo
{
    size_t totalBytes;
    size_t usedBytes;
};

struct AnimationFrame
{
    uint16_t frame;
    uint16_t index;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};