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

static const char* TAG = "main";

extern "C" void app_main(void)
{
    utils::delay_ms(500);
    gpio_reset_pin(heltec::PIN_LED_WRITE_CTRL);
    gpio_set_direction(heltec::PIN_LED_WRITE_CTRL, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "Ready to Blink");

    while (true)
    {
        ESP_LOGI(TAG, "Blinking");
        gpio_set_level(heltec::PIN_LED_WRITE_CTRL, 1);
        utils::delay_ms(500);
        gpio_set_level(heltec::PIN_LED_WRITE_CTRL, 0);
        utils::delay_ms(500);
    }
}
