/**
 * @file QMC5883L.hpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief Magnetometer Driver Header
 * @version 0.1
 * @date 2025-06-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"

class QMC5883L
{
public:
    QMC5883L(i2c_port_t port = I2C_NUM_0, gpio_num_t sda = GPIO_NUM_43,
             gpio_num_t scl = GPIO_NUM_44);
    esp_err_t init();
    esp_err_t read_raw(int16_t& x, int16_t& y, int16_t& z);
    float get_heading();

private:
    i2c_port_t _port;
    gpio_num_t _sda, _scl;
    static constexpr uint8_t QMC5883L_ADDR = 0x0D;

    esp_err_t write_reg(uint8_t reg, uint8_t val);
    esp_err_t read_bytes(uint8_t reg, uint8_t* data, size_t len);
};

void heading_task(void* args);
