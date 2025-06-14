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
#include <gnss.hpp>

static const char* TAG = "main";

extern "C" void app_main()
{
    utils::delay_ms(1000);
    HT_st7735 display;
    display.init();
    display.fill_screen(ST7735_BLACK);
    xTaskCreate(gnss_task, "gnss_task", 4096, &display, 5, NULL);
    while (true)
        utils::delay_ms(5000); // keep the main task alive
    
}