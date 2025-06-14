#pragma once

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_timer.h"

#include <utils.hpp>
#include "esp32-hal-gpio.h"

// --------------------------- io_pin_remap.h ----------------------------------
// TODO: Confirm there is no pin remapping, so this definition is enough
#define digitalPinToGPIONumber(digitalPin) (digitalPin)

// ----------------------------- Arduino.h -------------------------------------
#define LSBFIRST 0
#define MSBFIRST 1

#define NOT_A_PIN        -1
#define NOT_A_PORT       -1
#define NOT_AN_INTERRUPT -1
#define NOT_ON_TIMER     0

#define NUM_DIGITAL_PINS SOC_GPIO_PIN_COUNT  // All GPIOs
#define digitalPinToInterrupt(p)   ((((uint8_t)digitalPinToGPIONumber(p)) < NUM_DIGITAL_PINS) ? digitalPinToGPIONumber(p) : NOT_AN_INTERRUPT)

typedef bool boolean;
typedef uint8_t byte;

void delay(unsigned long ms)
{
    utils::delay_ms(ms);
}

void delayMicroseconds(unsigned int us)
{
    utils::delay_us(us);
}

void pinMode(uint8_t pin, uint8_t mode)
{
    gpio_config_t pGPIOConfig = {};
    pGPIOConfig.pin_bit_mask = 1 << pin;
    assert(mode == INPUT || mode == OUTPUT);
    pGPIOConfig.mode = (mode == INPUT) ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT;
    ESP_ERROR_CHECK(gpio_config(&pGPIOConfig));
}

void digitalWrite(uint8_t pin, uint8_t val)
{
    gpio_set_level(static_cast<gpio_num_t>(pin), val);
}

void yield()
{
    // TODO: Confirm this is the yield we want
    vPortYield();
}

unsigned long millis()
{
    return esp_timer_get_time() / 1000;
}
