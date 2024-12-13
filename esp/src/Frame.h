#pragma once
#include <Arduino.h>

enum FrameType : uint8_t{
    animation_add,
    animation_remove,
    animation_get,
    animation_play,
    animation_clear,
    animation_names,
    animation_stop,
    get_size,
    info_size,
    spin_start,
    spin_stop,
    spin_data,
};


struct Frame
{
    FrameType type;
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