#pragma once

#include <cstdint>

typedef struct
{
    const uint8_t width;
    uint8_t height;
    const uint16_t* data;
} FontDef;

extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;
