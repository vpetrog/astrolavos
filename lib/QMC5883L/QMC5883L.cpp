/**
 * @file QMC5883L.cpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief
 * @version 0.1
 * @date 2025-06-15
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "QMC5883L.hpp"
#include "esp_log.h"
#include <math.h>

constexpr uint8_t QMC5883L_REG_CTRL1 = 0x09;
constexpr uint8_t QMC5883L_REG_DATA = 0x00;
constexpr uint8_t QMC5883L_REG_SET = 0x0B;

/*
 * OSR (Over Sampling Rate) 128 (0b10000000)
 * RNG (Range) 2G               (0b00000000)
 * ODR (Output Data Rate) 10Hz  (0b00000000)
 * MODE (Continuous Mode)       (0b00000001)
 */
constexpr uint8_t QMC5883L_CONFIGURATION = 0b10000001;
constexpr uint8_t QMC5884L_RESET_CMD = 0x01; /* As per the datasheet */

QMC5883L::QMC5883L(i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
    : _port(port), _sda(sda), _scl(scl)
{
}

esp_err_t QMC5883L::init()
{
    i2c_config_t conf = {.mode = I2C_MODE_MASTER,
                         .sda_io_num = _sda,
                         .scl_io_num = _scl,
                         .sda_pullup_en = GPIO_PULLUP_ENABLE,
                         .scl_pullup_en = GPIO_PULLUP_ENABLE,
                         .master = {.clk_speed = 400000}};
    ESP_ERROR_CHECK(i2c_param_config(_port, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(_port, conf.mode, 0, 0, 0));

    write_reg(QMC5883L_REG_SET, 0x01);
    return write_reg(QMC5883L_REG_CTRL1, QMC5883L_CONFIGURATION);
}

esp_err_t QMC5883L::write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_write_to_device(_port, QMC5883L_ADDR, buf, 2,
                                      100 / portTICK_PERIOD_MS);
}

esp_err_t QMC5883L::read_bytes(uint8_t reg, uint8_t* data, size_t len)
{
    return i2c_master_write_read_device(_port, QMC5883L_ADDR, &reg, 1, data,
                                        len, 100 / portTICK_PERIOD_MS);
}

esp_err_t QMC5883L::read_raw(int16_t& x, int16_t& y, int16_t& z)
{
    uint8_t buf[6];
    esp_err_t err = read_bytes(QMC5883L_REG_DATA, buf, 6);
    if (err != ESP_OK)
        return err;

    x = (int16_t)(buf[1] << 8 | buf[0]);
    y = (int16_t)(buf[3] << 8 | buf[2]);
    z = (int16_t)(buf[5] << 8 | buf[4]);

    return ESP_OK;
}

float QMC5883L::get_heading()
{
    int16_t x, y, z;
    if (read_raw(x, y, z) != ESP_OK)
        return NAN;

    float angle = atan2((float)y, (float)x) * 180.0 / M_PI;
    if (angle < 0)
        angle += 360.0;
    return angle;
}
