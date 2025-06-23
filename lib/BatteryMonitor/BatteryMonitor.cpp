/**
 * @file BatteryMonitor.hpp
 * @author Evangelos Petrongonas (vpetog@ieee.org)
 * @brief Battery monitoring functionality
 * @version 0.1
 * @date 2025-06-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "driver/gpio.h"
#include "esp_pm.h"
#include <Astrolavos.hpp>
#include <BatteryMonitor.hpp>
#include <pins.hpp>
#include <utils.hpp>

static const char* TAG = "BatteryMon";
static constexpr size_t BATTERY_TASK_SLEEP = 1 * 60 * 1000; // 1 minute

/* By defining the levels like this, we are avoiding the use of  floats and
 * runtime in the code. Every little piece of saved energy matter */
uint8_t BatteryMonitor::voltage_to_percent(float v)
{
    if (v >= 4.20f)
        return 99;
    if (v >= 4.15f)
        return 95;
    if (v >= 4.10f)
        return 90;
    if (v >= 4.05f)
        return 85;
    if (v >= 4.00f)
        return 80;
    if (v >= 3.95f)
        return 75;
    if (v >= 3.90f)
        return 70;
    if (v >= 3.85f)
        return 65;
    if (v >= 3.80f)
        return 60;
    if (v >= 3.75f)
        return 55;
    if (v >= 3.70f)
        return 50;
    if (v >= 3.65f)
        return 45;
    if (v >= 3.60f)
        return 40;
    if (v >= 3.55f)
        return 35;
    if (v >= 3.50f)
        return 30;
    if (v >= 3.45f)
        return 25;
    if (v >= 3.40f)
        return 20;
    if (v >= 3.35f)
        return 15;
    if (v >= 3.30f)
        return 10;
    if (v >= 3.25f)
        return 5;
    return 0;
}

BatteryMonitor::BatteryMonitor(adc_channel_t channel)
{
    _channel = channel;

    gpio_reset_pin(heltec::PIN_ADC_CTRL);
    gpio_set_direction(heltec::PIN_ADC_CTRL, GPIO_MODE_OUTPUT);

    adc_oneshot_unit_init_cfg_t unit_cfg{
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &_adc));

    /* configure the specific channel (GPIO 1  → ADC1_CH0) */
    adc_oneshot_chan_cfg_t chan_cfg{.atten = ADC_ATTEN_DB_12,
                                    .bitwidth = ADC_BITWIDTH_12};
    ESP_ERROR_CHECK(adc_oneshot_config_channel(_adc, _channel, &chan_cfg));
}

void BatteryMonitor::output_voltage(HT_st7735* display, float voltage)
{
    uint8_t percent = voltage_to_percent(voltage);
    ESP_LOGI(TAG, "Battery Voltage: %.2f V Percent=%u", voltage, percent);

    /* pre-compute drawing area once (6 px × ASCII char, 4 chars “100%”) */
    static int X = 160 - (Font_7x10.width *
                          3); /* Bottom Right Alignment on 160 × 80 TFT */
    static int Y = 80 - Font_7x10.height;
    char buf[5]; /* "xxx%" + NUL */
    snprintf(buf, sizeof(buf), "%u%%", percent);
    display->fill_rectangle(X, Y, 160, 80, ST7735_BLACK);
    display->write_str(X, Y, buf, Font_7x10, ST7735_WHITE, ST7735_BLACK);
}

int BatteryMonitor::get_raw()
{
    int raw;
    ESP_ERROR_CHECK(adc_oneshot_read(_adc, _channel, &raw));
    return raw;
}

float BatteryMonitor::get_voltage(int raw)
{
    constexpr float ADC_SCALE =
        3.3f * 0.000244140625; /* 1 / ADC_BITWIDTH_12 at 12db attenuation */
    constexpr float DIVIDER = 4.9f;
    return static_cast<float>(raw) * ADC_SCALE * DIVIDER;
}

void battery_task(void* args)
{
    BatteryMonitor monitor;
    HT_st7735* display = reinterpret_cast<HT_st7735*>(args);
    ESP_LOGI(TAG, "Battery Monitor Task started");
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "battery_lock", &lock);
    display->fill_rectangle(0, 0, 160, 80, ST7735_BLACK);

    while (true)
    {
        esp_pm_lock_acquire(lock);
        gpio_hold_dis(heltec::PIN_ADC_CTRL);
        gpio_set_level(heltec::PIN_ADC_CTRL, 1);
        gpio_hold_en(heltec::PIN_ADC_CTRL);

        int raw = monitor.get_raw();
        float voltage = monitor.get_voltage(raw);

        display->unhold_pins();
        monitor.output_voltage(display, voltage);
        gpio_hold_dis(heltec::PIN_ADC_CTRL);
        gpio_set_level(heltec::PIN_ADC_CTRL, 0);
        gpio_hold_en(heltec::PIN_ADC_CTRL);
        display->hold_pins();

        ESP_LOGI(TAG, "Battery Voltage: %.2f V Raw=%d", voltage, raw);
        esp_pm_lock_release(lock);
        utils::delay_ms(BATTERY_TASK_SLEEP);
    }
}

void battery_astrolavos_task(void* args)
{
    BatteryMonitor monitor;
    astrolavos::Astrolavos* astrolavos_app =
        reinterpret_cast<astrolavos::Astrolavos*>(args);
    ESP_LOGI(TAG, "Battery Monitor Task started");
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "battery_lock", &lock);

    while (true)
    {
        esp_pm_lock_acquire(lock);
        gpio_hold_dis(heltec::PIN_ADC_CTRL);
        gpio_set_level(heltec::PIN_ADC_CTRL, 1);
        gpio_hold_en(heltec::PIN_ADC_CTRL);

        int raw = monitor.get_raw();
        float voltage = monitor.get_voltage(raw);

        astrolavos_app->updateHealthBattery(
            monitor.voltage_to_percent(voltage));

        gpio_hold_dis(heltec::PIN_ADC_CTRL);
        gpio_set_level(heltec::PIN_ADC_CTRL, 0);
        gpio_hold_en(heltec::PIN_ADC_CTRL);

        ESP_LOGI(TAG, "Battery Voltage: %.2f V Raw=%d", voltage, raw);
        esp_pm_lock_release(lock);
        utils::delay_ms(astrolavos_app->getSleepDuration()->battery);
    }
}