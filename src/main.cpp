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
#include "nvs_flash.h"
#include <Astrolavos.hpp>
#include <BatteryMonitor.hpp>
#include <HT_st7735.hpp>
#include <LoRaMockup.hpp>
#include <QMC5883L.hpp>
#include <gnss.hpp>
#include <lora.hpp>
#include <pins.hpp>
#include <utils.hpp>

static const char* TAG = "main";

void blinking_task(void* args)
{
    astrolavos::Astrolavos* astrolavos_app =
        static_cast<astrolavos::Astrolavos*>(args);
    size_t sleep_duration;
    if (astrolavos_app)
        sleep_duration = astrolavos_app->getSleepDuration()->blinking;
    else
        sleep_duration = 1500; // Default blinking duration if no app provided
    gpio_set_direction(heltec::PIN_LED_WRITE_CTRL, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "Blinking Task started");
    while (true)
    {
        if (astrolavos_app)
            sleep_duration = astrolavos_app->getSleepDuration()->blinking;
        ESP_LOGI(TAG, "Blinking");
        gpio_hold_dis(heltec::PIN_LED_WRITE_CTRL);
        gpio_set_level(heltec::PIN_LED_WRITE_CTRL, 1);
        gpio_hold_en(heltec::PIN_LED_WRITE_CTRL);

        utils::delay_ms(sleep_duration);

        gpio_hold_dis(heltec::PIN_LED_WRITE_CTRL);
        gpio_set_level(heltec::PIN_LED_WRITE_CTRL, 0);
        gpio_hold_en(heltec::PIN_LED_WRITE_CTRL);
        utils::delay_ms(sleep_duration);
    }
}

#if 0
extern "C" void app_main()
{

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGI(TAG, "NVS Flash needs to be erased, re-initializing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "NVS Flash initialized");

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

    xTaskCreate(gnss_task, "gnss_task", 4096, &display, 5, NULL);
    xTaskCreate(battery_task, "battery_task", 4096, &display, 2, NULL);
    xTaskCreate(blinking_task, "blinking_task", 4096, nullptr, 3, NULL);
    xTaskCreate(heading_task, "heading_task", 4096, &display, 4, NULL);

    vTaskSuspend(NULL);
}
#else
extern "C" void app_main()
{
    HT_st7735 display;
    astrolavos::Astrolavos astrolavos_app;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGI(TAG, "NVS Flash needs to be erased, re-initializing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "NVS Flash initialized");

    display.init();
    display.set_backlight(80);
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 10,
        .light_sleep_enable = true,
    };

    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));

    astrolavos::astrolavos_args_t task_args = {
        .display = &display,
        .app = &astrolavos_app,
    };
    xTaskCreate(astrolavos_task, "astrolavos_task", 4096, &task_args, 5, NULL);
    xTaskCreate(gnss_astrolavos_task, "gnss_task", 4096, &astrolavos_app, 4,
                NULL);
    xTaskCreate(battery_astrolavos_task, "battery_task", 4096, &astrolavos_app,
                2, NULL);
    xTaskCreate(blinking_task, "blinking_task", 4096, &astrolavos_app, 3, NULL);
    xTaskCreate(heading_astrolavos_task, "heading_task", 4096, &astrolavos_app,
                4, NULL);
#if defined(ASTROLAVOS_MOCKUP_LORA_RECEIVER)
    xTaskCreate(loraMockupInitReceiver_task, "lora_mockup_receiver_task", 4096,
                &astrolavos_app, 1, NULL);
#endif
    xTaskCreate(lora_rx_astrolavos_task, "lora_rx_task", 4096, &astrolavos_app,
                1, NULL);
    // xTaskCreate(lora_tx_astrolavos_task, "lora_tx_task", 4096,
    // &astrolavos_app,
    //             1, NULL);

    vTaskSuspend(NULL);
}
#endif
