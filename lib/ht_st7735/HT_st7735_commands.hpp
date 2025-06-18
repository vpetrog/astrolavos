/**
 * @file HT_st7735_commands.hpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief Commands for the ST7735 TFT display driver
 * @version 0.1
 * @date 2025-06-18
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once
#include <cstdint>

constexpr uint8_t ST7735_NOP = 0x00;
constexpr uint8_t ST7735_SWRESET = 0x01;
constexpr uint8_t ST7735_RDDID = 0x04;
constexpr uint8_t ST7735_RDDST = 0x09;

constexpr uint8_t ST7735_SLPIN = 0x10;
constexpr uint8_t ST7735_SLPOUT = 0x11;
constexpr uint8_t ST7735_PTLON = 0x12;
constexpr uint8_t ST7735_NORON = 0x13;

constexpr uint8_t ST7735_INVOFF = 0x20;
constexpr uint8_t ST7735_INVON = 0x21;
constexpr uint8_t ST7735_GAMSET = 0x26;
constexpr uint8_t ST7735_DISPOFF = 0x28;
constexpr uint8_t ST7735_DISPON = 0x29;
constexpr uint8_t ST7735_CASET = 0x2A;
constexpr uint8_t ST7735_RASET = 0x2B;
constexpr uint8_t ST7735_RAMWR = 0x2C;
constexpr uint8_t ST7735_RAMRD = 0x2E;

constexpr uint8_t ST7735_PTLAR = 0x30;
constexpr uint8_t ST7735_COLMOD = 0x3A;
constexpr uint8_t ST7735_MADCTL = 0x36;

constexpr uint8_t ST7735_FRMCTR1 = 0xB1;
constexpr uint8_t ST7735_FRMCTR2 = 0xB2;
constexpr uint8_t ST7735_FRMCTR3 = 0xB3;
constexpr uint8_t ST7735_INVCTR = 0xB4;
constexpr uint8_t ST7735_DISSET5 = 0xB6;

constexpr uint8_t ST7735_PWCTR1 = 0xC0;
constexpr uint8_t ST7735_PWCTR2 = 0xC1;
constexpr uint8_t ST7735_PWCTR3 = 0xC2;
constexpr uint8_t ST7735_PWCTR4 = 0xC3;
constexpr uint8_t ST7735_PWCTR5 = 0xC4;
constexpr uint8_t ST7735_VMCTR1 = 0xC5;

constexpr uint8_t ST7735_RDID1 = 0xDA;
constexpr uint8_t ST7735_RDID2 = 0xDB;
constexpr uint8_t ST7735_RDID3 = 0xDC;
constexpr uint8_t ST7735_RDID4 = 0xDD;

constexpr uint8_t ST7735_PWCTR6 = 0xFC;

constexpr uint8_t ST7735_GMCTRP1 = 0xE0;
constexpr uint8_t ST7735_GMCTRN1 = 0xE1;
