#pragma once
#include <Arduino.h>

enum FrameType{
    animation_add,
    animation_remove,
    animation_get,
    animation_play,
    animation_clear,
    animation_names,
    animation_stop,
    get_size,
    info_size,
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