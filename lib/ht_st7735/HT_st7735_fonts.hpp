/**
 * @file HT_st7735_fonts.hpp
 * @author Evangelos Petrongonas  (vpetrog@ieee.org)
 * @brief ST 7355 Font Declarations
 * @version 0.1
 * @date 2025-06-13
 *
 * @copyright Copyright (c) 2025
 *
 */

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
