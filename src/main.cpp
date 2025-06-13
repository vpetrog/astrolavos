/**
 * @file main.cpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief Main Application
 * @version 0.1
 * @date 2025-06-13
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <pins.hpp>
#include <utils.hpp>
#include <HT_st7735.hpp>

static const char* TAG = "main";

extern "C" void app_main(void)
{
    HT_st7735 st7735;

    utils::delay_ms(500);
    st7735.init();
    ESP_LOGI(TAG, "TFT Initialised");

    while (true)
    {
        st7735.fill_screen(ST7735_BLACK);

        for (int x = 0; x < ST7735_WIDTH; x++)
        {
            st7735.draw_pixel(x, 0, ST7735_RED);
            st7735.draw_pixel(x, ST7735_HEIGHT - 1, ST7735_RED);
        }

        for (int y = 0; y < ST7735_HEIGHT; y++)
        {
            st7735.draw_pixel(0, y, ST7735_RED);
            st7735.draw_pixel(ST7735_WIDTH - 1, y, ST7735_RED);
        }

        vTaskDelay(pdMS_TO_TICKS(3000));

        /* Demo Fonts */
        st7735.fill_screen(ST7735_BLACK);
        st7735.write_str(0, 0,
                         "Font_7x10, red on black, lorem ipsum dolor sit amet",
                         Font_7x10, ST7735_RED, ST7735_BLACK);
        st7735.write_str(0, 3 * 10, "Font_11x18, green, lorem ipsum",
                         Font_11x18, ST7735_GREEN, ST7735_BLACK);
        st7735.write_str(0, 3 * 10 + 3 * 18, "Font_16x26", Font_16x26,
                         ST7735_BLUE, ST7735_BLACK);
        utils::delay_ms(2000);

        /* Demo Colours */
        st7735.fill_screen(ST7735_BLACK);
        st7735.write_str(0, 0, "BLACK", Font_11x18, ST7735_WHITE, ST7735_BLACK);
        utils::delay_ms(500);

        st7735.fill_screen(ST7735_BLUE);
        st7735.write_str(0, 0, "BLUE", Font_11x18, ST7735_BLACK, ST7735_BLUE);
        utils::delay_ms(500);

        st7735.fill_screen(ST7735_RED);
        st7735.write_str(0, 0, "RED", Font_11x18, ST7735_BLACK, ST7735_RED);
        utils::delay_ms(500);

        st7735.fill_screen(ST7735_GREEN);
        st7735.write_str(0, 0, "GREEN", Font_11x18, ST7735_BLACK, ST7735_GREEN);
        utils::delay_ms(500);

        st7735.fill_screen(ST7735_CYAN);
        st7735.write_str(0, 0, "CYAN", Font_11x18, ST7735_BLACK, ST7735_CYAN);
        utils::delay_ms(500);

        st7735.fill_screen(ST7735_MAGENTA);
        st7735.write_str(0, 0, "MAGENTA", Font_11x18, ST7735_BLACK,
                         ST7735_MAGENTA);
        utils::delay_ms(500);

        st7735.fill_screen(ST7735_YELLOW);
        st7735.write_str(0, 0, "YELLOW", Font_11x18, ST7735_BLACK,
                         ST7735_YELLOW);
        utils::delay_ms(500);

        st7735.fill_screen(ST7735_WHITE);
        st7735.write_str(0, 0, "WHITE", Font_11x18, ST7735_BLACK, ST7735_WHITE);
        utils::delay_ms(500);
    }
}
