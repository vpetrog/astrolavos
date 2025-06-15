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
#include <BatteryMonitor.hpp>
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
    display.set_backlight(10);
    display.fill_screen(ST7735_BLACK);
    xTaskCreate(gnss_task, "gnss_task", 4096, &display, 5, NULL);
    xTaskCreate(battery_task, "battery_task", 4096, &display, 2, NULL);
    
    vTaskSuspend(NULL);
}