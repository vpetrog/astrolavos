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
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <HT_st7735.hpp>
#include <gnss.hpp>
#include <pins.hpp>
#include <utils.hpp>

static const char* TAG = "GNSS";

TinyGPSPlus gps;

const size_t BUF_SIZE = 1024;
constexpr size_t GNSS_TASK_SLEEP_TIMER = 30 * 1000000ULL;

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
    while (true)
    {
        gnss_power_up();
        utils::delay_ms(3000);
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

            display->fill_rectangle(0, 0, 180, 28, ST7735_BLACK);
            display->write_str(0, 0, buffer, Font_7x10, ST7735_WHITE,
                               ST7735_BLACK);
            gnss_power_down();
            display->hold_pins();
            esp_sleep_enable_timer_wakeup(GNSS_TASK_SLEEP_TIMER);
            esp_light_sleep_start();
        }
        else
        {
            display->fill_rectangle(0, 0, 180, 28, ST7735_BLACK);
            display->write_str(0, 0, "Waiting for GNSS Data", Font_7x10,
                               ST7735_WHITE, ST7735_BLACK);
            ESP_LOGI(TAG, "Waiting for GPS data... %d bytes", len);
        }

        utils::delay_ms(1000);
    }
}
