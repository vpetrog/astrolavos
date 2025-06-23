/**
 * @file HT_st7735.hpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief ST7735 TFT Driver for Heltec Wireless Tracker
 * @version 0.1
 * @date 2025-06-13
 *
 * @copyright Copyright (c) 2025
 *
 * The driver is a port of https://github.com/HelTecAutomation/Heltec_ESP32
 * for the esp-idf framework
 */

#include "HT_st7735_fonts.hpp"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <stdint.h>

#pragma once

/* ----------------------- GPIO defaults  ----------------------- */
constexpr gpio_num_t ST7735_CS_Pin = GPIO_NUM_38;
constexpr gpio_num_t ST7735_REST_Pin = GPIO_NUM_39;
constexpr gpio_num_t ST7735_DC_Pin = GPIO_NUM_40;
constexpr gpio_num_t ST7735_SCLK_Pin = GPIO_NUM_41;
constexpr gpio_num_t ST7735_MOSI_Pin = GPIO_NUM_42;
constexpr gpio_num_t ST7735_LED_K_Pin = GPIO_NUM_21;
constexpr gpio_num_t ST7735_VTFT_CTRL_Pin = GPIO_NUM_3;

/* ------------------------  Compile‑time display config ------------------- */
#define ST7735_IS_160X80
constexpr uint8_t ST7735_XSTART = 1;
constexpr uint8_t ST7735_YSTART = 26;
constexpr uint8_t ST7735_WIDTH = 160;
constexpr uint8_t ST7735_HEIGHT = 80;
constexpr uint8_t ST7735_MADCTL_MX = 0x40;
constexpr uint8_t ST7735_MADCTL_MY = 0x80;
constexpr uint8_t ST7735_MADCTL_MV = 0x20;
constexpr uint8_t ST7735_MADCTL_BGR = 0x08;
constexpr uint8_t ST7735_ROTATION =
    (ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_BGR);

/* ------------------- Color definitions ---------------------------------- */
#define ST7735_BLACK 0x0000
#define ST7735_BLUE 0x001F
#define ST7735_RED 0xF800
#define ST7735_GREEN 0x07E0
#define ST7735_CYAN 0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW 0xFFE0
#define ST7735_WHITE 0xFFFF
#define ST7735_ORANGE 0xFD20
#define ST7735_COLOR565(r, g, b)                                               \
    (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

typedef enum
{
    GAMMA_10 = 0x01,
    GAMMA_25 = 0x02,
    GAMMA_22 = 0x04,
    GAMMA_18 = 0x08
} GammaDef;

#define DELAY 0x80

/* ---------------- Initialisation Command Sequences ------------------------ */
extern const uint8_t init_cmds1[]; // paste as‑is or place in another .c file
extern const uint8_t init_cmds2[];
extern const uint8_t init_cmds3[];

/* ---------------------------------  Driver ---------------------------------
 */
class HT_st7735
{
public:
    HT_st7735(gpio_num_t cs = ST7735_CS_Pin, gpio_num_t rst = ST7735_REST_Pin,
              gpio_num_t dc = ST7735_DC_Pin, gpio_num_t sclk = ST7735_SCLK_Pin,
              gpio_num_t mosi = ST7735_MOSI_Pin,
              gpio_num_t led = ST7735_LED_K_Pin,
              gpio_num_t vtft = ST7735_VTFT_CTRL_Pin)
        : _cs(cs), _rst(rst), _dc(dc), _sclk(sclk), _mosi(mosi), _led(led),
          _vtft(vtft), _spi(nullptr), _width(ST7735_WIDTH),
          _height(ST7735_HEIGHT), _x_start(ST7735_XSTART),
          _y_start(ST7735_YSTART)
    {
    }

    esp_err_t init();
    void draw_pixel(uint16_t x, uint16_t y, uint16_t color);
    void write_char(uint16_t x, uint16_t y, char ch, FontDef font,
                    uint16_t color, uint16_t bgcolor);
    void write_str(uint16_t x, uint16_t y, const char* str, FontDef font,
                   uint16_t color, uint16_t bgcolor);
    void fill_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        uint16_t c);
    void fill_screen(uint16_t color)
    {
        fill_rectangle(0, 0, _width, _height, color);
    }
    void draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                    const uint16_t* d);
    void invert_colors(bool inv);
    void set_gamma(uint8_t gamma);
    void hold_pins(void);
    void unhold_pins(void);
    void set_backlight(const uint8_t percent);
    void hold_backlight(bool enable = true);
    void turn_off();
    void turn_on();

private:
    /* SPI helpers */
    inline void select() { gpio_set_level(_cs, 0); }
    inline void unselect() { gpio_set_level(_cs, 1); }
    void reset();
    inline void cmd(uint8_t c);
    inline void data(const uint8_t* d, size_t len);
    void exec_cmd_list(const uint8_t* addr);
    void addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

    gpio_num_t _cs, _rst, _dc, _sclk, _mosi, _led, _vtft;
    spi_device_handle_t _spi;
    uint16_t _width, _height, _x_start, _y_start;
    static constexpr ledc_mode_t LEDC_MODE = LEDC_LOW_SPEED_MODE;
    static constexpr ledc_timer_t LEDC_TIMER = LEDC_TIMER_0;
    static constexpr ledc_channel_t LEDC_CH = LEDC_CHANNEL_0;
    static constexpr uint32_t LEDC_FREQ_HZ = 1000;
    static constexpr uint32_t LEDC_RES_BITS = 10;
    SemaphoreHandle_t _mutex = nullptr;
};
