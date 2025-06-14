/**
 * @file HT_st7735.cpp
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

#include "HT_st7735.hpp"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <utils.hpp>

static const char* TAG = "st7735";

esp_err_t HT_st7735::init()
{
    gpio_config_t io = {};
    io.mode = GPIO_MODE_OUTPUT;
    io.pin_bit_mask =
        (1ULL << _cs) | (1ULL << _rst) | (1ULL << _dc) | (1ULL << _led);
    if (_vtft != GPIO_NUM_NC)
        io.pin_bit_mask |= 1ULL << _vtft;
    ESP_ERROR_CHECK(gpio_config(&io));
    unselect();
    gpio_set_level(_led, 1);
    if (_vtft != GPIO_NUM_NC)
        gpio_set_level(_vtft, 1);

    spi_bus_config_t bus = {};
    bus.mosi_io_num = _mosi;
    bus.miso_io_num = -1;
    bus.sclk_io_num = _sclk;
    bus.max_transfer_sz = ST7735_WIDTH * ST7735_HEIGHT * 2 + 8;
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t dev = {};
    dev.clock_speed_hz = 26 * 1000 * 1000; // 26 MHz (40 MHz max for ST7735)
    dev.mode = 0;
    dev.spics_io_num = -1; // manual CS
    dev.queue_size = 7;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &dev, &_spi));

    select();
    reset();
    exec_cmd_list(init_cmds1);
    exec_cmd_list(init_cmds2);
    exec_cmd_list(init_cmds3);
    unselect();

    // --- add just after gpio_set_level(_led, 1);
    ledc_timer_config_t tcfg = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQ_HZ,
        .clk_cfg = LEDC_USE_RC_FAST_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&tcfg));

    ledc_channel_config_t ccfg = {.gpio_num = _led,
                                  .speed_mode = LEDC_MODE,
                                  .channel = LEDC_CH,
                                  .timer_sel = LEDC_TIMER,
                                  .duty = (1 << LEDC_RES_BITS) -
                                          1, // 100 % on start-up
                                  .hpoint = 0,
                                  .sleep_mode = LEDC_SLEEP_MODE_KEEP_ALIVE};
    ESP_ERROR_CHECK(ledc_channel_config(&ccfg));

    ESP_LOGI(TAG, "ST7735 initialised (%ux%u)", _width, _height);
    return ESP_OK;
}

void HT_st7735::reset()
{
    gpio_set_level(_rst, 0);
    utils::delay_ms(5);
    gpio_set_level(_rst, 1);
    utils::delay_ms(150);
}

inline void HT_st7735::cmd(uint8_t c)
{
    gpio_set_level(_dc, 0);
    spi_transaction_t t = {};
    t.flags = SPI_TRANS_USE_TXDATA;
    t.length = 8;
    t.tx_data[0] = c;
    ESP_ERROR_CHECK(spi_device_polling_transmit(_spi, &t));
}

inline void HT_st7735::data(const uint8_t* d, size_t len)
{
    if (!len)
        return;
    gpio_set_level(_dc, 1);
    spi_transaction_t t = {};
    t.length = len * 8;
    t.tx_buffer = d;
    ESP_ERROR_CHECK(spi_device_polling_transmit(_spi, &t));
}

void HT_st7735::exec_cmd_list(const uint8_t* a)
{
    uint8_t nCmd = *a++;
    while (nCmd--)
    {
        uint8_t c = *a++;
        cmd(c);
        uint8_t nArg = *a++;
        uint16_t ms = nArg & DELAY;
        nArg &= ~DELAY;
        data(a, nArg);
        a += nArg;
        if (ms)
        {
            ms = *a++;
            if (ms == 255)
                ms = 500;
            utils::delay_ms(ms);
        }
    }
}

void HT_st7735::addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    cmd(0x2A /*CASET*/);
    uint8_t d1[] = {0x00, (uint8_t)(x0 + _x_start), 0x00,
                    (uint8_t)(x1 + _x_start)};
    data(d1, 4);
    cmd(0x2B /*RASET*/);
    uint8_t d2[] = {0x00, (uint8_t)(y0 + _y_start), 0x00,
                    (uint8_t)(y1 + _y_start)};
    data(d2, 4);
    cmd(0x2C /*RAMWR*/);
}

void HT_st7735::draw_pixel(uint16_t x, uint16_t y, uint16_t col)
{
    if (x >= _width || y >= _height)
        return;
    select();
    addr_window(x, y, x, y);
    uint8_t d[] = {(uint8_t)(col >> 8), (uint8_t)col};
    data(d, 2);
    unselect();
}

void HT_st7735::write_char(uint16_t x, uint16_t y, char ch, FontDef f,
                           uint16_t col, uint16_t bg)
{
    addr_window(x, y, x + f.width - 1, y + f.height - 1);
    for (uint32_t i = 0; i < f.height; i++)
    {
        uint32_t row = f.data[(ch - 32) * f.height + i];
        for (uint32_t j = 0; j < f.width; j++)
        {
            uint16_t c = ((row << j) & 0x8000) ? col : bg;
            uint8_t d[] = {(uint8_t)(c >> 8), (uint8_t)c};
            data(d, 2);
        }
    }
}

void HT_st7735::write_str(uint16_t x, uint16_t y, const char* s, FontDef f,
                          uint16_t col, uint16_t bg)
{
    select();
    uint16_t orig_x = x;
    while (*s)
    {
        if (*s == '\n')
        {
            x = orig_x;
            y += f.height;
            ++s;
            if (y + f.height > _height)
                break;
            continue;
        }
        if (x + f.width > _width)
        {
            x = orig_x;
            y += f.height;
            if (y + f.height > _height)
                break;
        }
        write_char(x, y, *s, f, col, bg);
        x += f.width;
        ++s;
    }
    unselect();
}

void HT_st7735::fill_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                               uint16_t col)
{
    if (x >= _width || y >= _height)
        return;
    if (x + w - 1 >= _width)
        w = _width - x;
    if (y + h - 1 >= _height)
        h = _height - y;
    select();
    addr_window(x, y, x + w - 1, y + h - 1);
    uint32_t pixels = w * h;
    const uint32_t max_buf = 256;
    uint8_t buf[max_buf * 2];
    for (uint32_t i = 0; i < max_buf; ++i)
    {
        buf[2 * i] = (uint8_t)(col >> 8);
        buf[2 * i + 1] = (uint8_t)col;
    }
    gpio_set_level(_dc, 1);
    while (pixels)
    {
        uint32_t chunk = (pixels > max_buf) ? max_buf : pixels;
        data(buf, chunk * 2);
        pixels -= chunk;
    }
    unselect();
}

void HT_st7735::draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                           const uint16_t* img)
{
    if (x >= _width || y >= _height || x + w - 1 >= _width ||
        y + h - 1 >= _height)
        return;
    select();
    addr_window(x, y, x + w - 1, y + h - 1);
    data(reinterpret_cast<const uint8_t*>(img), w * h * 2);
    unselect();
}

void HT_st7735::invert_colors(bool inv)
{
    select();
    cmd(inv ? 0x21 /*INVON*/ : 0x20 /*INVOFF*/);
    unselect();
}
void HT_st7735::set_gamma(uint8_t g)
{
    select();
    cmd(0x26 /*GAMSET*/);
    data(&g, 1);
    unselect();
}

void HT_st7735::hold_pins()
{
    /* The _led pin is not configured as it managed by the ledc*/
    gpio_hold_en(_cs);
    gpio_hold_en(_rst);
    gpio_hold_en(_dc);
    if (_vtft != GPIO_NUM_NC)
        gpio_hold_en(_vtft);
}

void HT_st7735::unhold_pins()
{
    /* The _led pin is not configured as it managed by the ledc*/
    gpio_hold_dis(_cs);
    gpio_hold_dis(_rst);
    gpio_hold_dis(_dc);
    if (_vtft != GPIO_NUM_NC)
        gpio_hold_dis(_vtft);
}

void HT_st7735::set_backlight(uint8_t percent)
{
    if (percent > 100)
        percent = 100;
    uint32_t duty = ((uint32_t)percent * ((1 << LEDC_RES_BITS) - 1)) / 100;
    ledc_set_duty(LEDC_MODE, LEDC_CH, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CH);
}