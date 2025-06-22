/**
 * @file gps.cpp
 * @author Evangelos Petrongonas ( vpetrog@ieee.org)
 * @brief
 * @version 0.1
 * @date 2025-06-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "TinyGPS++.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Astrolavos.hpp>
#include <HT_st7735.hpp>
#include <gnss.hpp>
#include <pins.hpp>
#include <utils.hpp>

static const char* TAG = "GNSS";

TinyGPSPlus gps;

const size_t BUF_SIZE = 1024;
constexpr size_t GNSS_TASK_LOCATED_SLEEP = 15 * 1000;
constexpr size_t GNSS_TASK_SCANNING_SLEEP = 1 * 1000;
constexpr size_t GNSS_POWER_UP_SLEEP = 3 * 1000;

void gnss_power_up()
{
    gpio_set_level(heltec::PIN_GNSS_RST, 1);
    utils::delay_ms(15);
}

void gnss_power_down()
{
    gpio_set_level(heltec::PIN_GNSS_RST, 0);
    utils::delay_ms(15);
}

void gnss_task(void* args)
{
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "gnss_lock", &lock);
    esp_pm_lock_acquire(lock);
    utils::delay_ms(1000);
    HT_st7735* display = reinterpret_cast<HT_st7735*>(args);

    uart_config_t uart_config = {.baud_rate = 115200,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1};
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, heltec::PIN_GNSS_RX, heltec::PIN_GNSS_TX,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 8, nullptr, 0);

    uint8_t data[BUF_SIZE];
    esp_pm_lock_release(lock);
    while (true)
    {
        esp_pm_lock_acquire(lock);
        gnss_power_up();
        utils::delay_ms(GNSS_POWER_UP_SLEEP);
        int len =
            uart_read_bytes(UART_NUM_1, data, BUF_SIZE, pdMS_TO_TICKS(100));

        for (int i = 0; i < len; i++)
            gps.encode(data[i]);

        if (gps.location.isUpdated())
        {
            char buffer[180 * 3];
            snprintf(buffer, sizeof(buffer),
                     "No Satellites: %lu\nLat: %.5f\nLon: %.5f",
                     gps.satellites.value(), gps.location.lat(),
                     gps.location.lng());

            ESP_LOGI(TAG, "%s", buffer);
            display->unhold_pins();
            display->fill_rectangle(0, 0, 180, 28, ST7735_BLACK);
            display->write_str(0, 0, buffer, Font_7x10, ST7735_WHITE,
                               ST7735_BLACK);
            gnss_power_down();
            display->hold_pins();
            esp_pm_lock_release(lock);
            utils::delay_ms(GNSS_TASK_LOCATED_SLEEP);
        }
        else
        {
            display->unhold_pins();
            display->fill_rectangle(0, 0, 180, 28, ST7735_BLACK);
            display->write_str(0, 0, "Waiting for GNSS Data", Font_7x10,
                               ST7735_WHITE, ST7735_BLACK);
            ESP_LOGI(TAG, "Waiting for GNSS data... %d bytes", len);
            display->hold_pins();
            esp_pm_lock_release(lock);
            utils::delay_ms(GNSS_TASK_SCANNING_SLEEP);
        }
    }
}

void gnss_astrolavos_task(void* args)
{
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "gnss_lock", &lock);
    esp_pm_lock_acquire(lock);
    utils::delay_ms(1000);
    astrolavos::Astrolavos* astrolavos_app =
        reinterpret_cast<astrolavos::Astrolavos*>(args);

    uart_config_t uart_config = {.baud_rate = 115200,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1};
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, heltec::PIN_GNSS_RX, heltec::PIN_GNSS_TX,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 8, nullptr, 0);

    uint8_t data[BUF_SIZE];
    esp_pm_lock_release(lock);
    while (true)
    {
        esp_pm_lock_acquire(lock);
        gnss_power_up();
        utils::delay_ms(GNSS_POWER_UP_SLEEP);
        int len =
            uart_read_bytes(UART_NUM_1, data, BUF_SIZE, pdMS_TO_TICKS(100));

        for (int i = 0; i < len; i++)
            gps.encode(data[i]);

        if (gps.location.isUpdated())
        {
            ESP_LOGI(TAG,
                     "GNSS Data Updated,nUM Satellites: %lu, "
                     "Lat: %.5f, Lon: %.5f",
                     gps.satellites.value(), gps.location.lat(),
                     gps.location.lng());
            astrolavos_app->updateHealthGNSS(
                static_cast<uint8_t>(gps.satellites.value()));
            astrolavos_app->updateCoordinates(
                {static_cast<float>(gps.location.lat()),
                 static_cast<float>(gps.location.lng()),
                 static_cast<uint32_t>(esp_timer_get_time())});
            gnss_power_down();
            esp_pm_lock_release(lock);
            utils::delay_ms(GNSS_TASK_LOCATED_SLEEP);
        }
        else
        {
            astrolavos_app->updateHealthGNSS(astrolavos::GNSS_NO_SATELLITES);
            esp_pm_lock_release(lock);
            utils::delay_ms(GNSS_TASK_SCANNING_SLEEP);
        }
    }
}