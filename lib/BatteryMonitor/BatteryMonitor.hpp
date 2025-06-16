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

#pragma once

#include "HT_st7735.hpp"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class BatteryMonitor
{
public:
    BatteryMonitor(adc_channel_t channel = ADC_CHANNEL_0);
    void output_voltage(HT_st7735* display, float voltage);
    int get_raw();
    float get_voltage(int raw);
    uint8_t voltage_to_percent(float v);

private:
    adc_oneshot_unit_handle_t _adc;
    float _divider;
    adc_channel_t _channel;
    TickType_t _period;
};

void battery_task(void* args);
void battery_astrolavos_task(void* args);
