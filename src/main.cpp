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
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <BatteryMonitor.hpp>
#include <HT_st7735.hpp>
#include <QMC5883L.hpp>
#include <gnss.hpp>
#include <pins.hpp>
#include <utils.hpp>
#include <lora.hpp>

static const char* TAG = "main";

constexpr size_t BLINKING_TASK_SLEEP = 3500;

void blinking_task(void* args)
{
    gpio_set_direction(heltec::PIN_LED_WRITE_CTRL, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "Blinking Task started");
    while (true)
    {
        ESP_LOGI(TAG, "Blinking");
        gpio_hold_dis(heltec::PIN_LED_WRITE_CTRL);
        gpio_set_level(heltec::PIN_LED_WRITE_CTRL, 1);
        gpio_hold_en(heltec::PIN_LED_WRITE_CTRL);

        utils::delay_ms(BLINKING_TASK_SLEEP);

        gpio_hold_dis(heltec::PIN_LED_WRITE_CTRL);
        gpio_set_level(heltec::PIN_LED_WRITE_CTRL, 0);
        gpio_hold_en(heltec::PIN_LED_WRITE_CTRL);
        utils::delay_ms(BLINKING_TASK_SLEEP);
    }
}

extern "C" void app_main()
{
    utils::delay_ms(1000);
    HT_st7735 display;
    display.init();
    display.set_backlight(10);
    display.fill_screen(ST7735_BLACK);

    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 10,
        .light_sleep_enable = true,
    };

    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));

    // xTaskCreate(gnss_task, "gnss_task", 4096, &display, 5, NULL);
    // xTaskCreate(battery_task, "battery_task", 4096, &display, 2, NULL);
    // xTaskCreate(blinking_task, "blinking_task", 4096, &display, 3, NULL);
    // xTaskCreate(heading_task, "heading_task", 4096, &display, 4, NULL);
    xTaskCreate(lora_task, "lora_task", 4096, &display, 5, NULL);

    vTaskSuspend(NULL);
}
