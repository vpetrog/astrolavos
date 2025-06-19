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
#include "esp_pm.h"
#include <Astrolavos.hpp>
#include <HT_st7735.hpp>
#include <math.h>
#include <utils.hpp>

constexpr size_t HEADING_TASK_SLEEP = 1000;

constexpr const char* TAG = "QMC5883L";

#if defined(QMC5883L_USE_QMC5883L)
constexpr uint8_t QMC5883L_REG_CTRL1 = 0x09;
constexpr uint8_t QMC5883L_REG_DATA = 0x00;
constexpr uint8_t QMC5883L_REG_SET = 0x0B;
#elif defined(QMC5883L_USE_QMC5883P)
constexpr uint8_t QMC5883L_REG_DATA = 0x01;
constexpr uint8_t QMC5883L_REG_CTRL_1 = 0x0A;
constexpr uint8_t QMC5883L_REG_CTRL_2 = 0x0B;

; // Data register
#endif

#if defined(QMC5883L_USE_QMC5883L)
/*
 * OSR (Over Sampling Rate) 128 (0b10000000)
 * RNG (Range) 2G               (0b00000000)
 * ODR (Output Data Rate) 50Hz  (0b00000100)
 * MODE (Continuous Mode)       (0b00000001)
 */
constexpr uint8_t QMC5883L_CONFIGURATION = 0b10000001;
constexpr uint8_t QMC5884L_RESET_CMD = 0x01; /* As per the datasheet */
#elif defined(QMC5883L_USE_QMC5883P)
/*
 * OSR2 = 0b00 (1)
 * OSR1 = 0b11 (8)
 * ODR = 0b01 (50Hz)
 * MODE = 0b01 (Normal Mode)
 */
constexpr uint8_t QMC5883L_CONFIGURATION_CTRL_1 = 0b00110001;
/*
 * RNG = 0B11 (2G)
 */
constexpr uint8_t QMC5883L_CONFIGURATION_CTRL_2 = 0b00000000;
#endif

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
#if defined(QMC5883L_USE_QMC5883L)
    write_reg(QMC5883L_REG_SET, 0x01);
    return write_reg(QMC5883L_REG_CTRL1, QMC5883L_CONFIGURATION);
#elif defined(QMC5883L_USE_QMC5883P)
    esp_err_t rc = 0;
    rc = write_reg(QMC5883L_REG_CTRL_1, QMC5883L_CONFIGURATION_CTRL_1);
    if (rc != ESP_OK)
        return rc;
    else
        return write_reg(QMC5883L_REG_CTRL_2, QMC5883L_CONFIGURATION_CTRL_2);
#endif
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

void heading_task(void* args)
{
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "heading_lock", &lock);
    esp_pm_lock_acquire(lock);
    HT_st7735* display = static_cast<HT_st7735*>(args);
    QMC5883L qmc5883l;
    int Y = 4 * Font_7x10.height;

    if (qmc5883l.init() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize QMC5883L");
    }
    char buf[9];

    int16_t x, y, z;
    esp_pm_lock_release(lock);
    while (true)
    {
        esp_pm_lock_acquire(lock);
        if (qmc5883l.read_raw(x, y, z) == ESP_OK)
        {
            uint16_t heading = qmc5883l.get_heading();
            ESP_LOGI(TAG, "Heading: %u", heading);
            display->unhold_pins();
            display->fill_rectangle(0, Y, 180, Y + Font_7x10.height,
                                    ST7735_BLACK);
            snprintf(buf, sizeof(buf), "%udeg", heading);
            display->write_str(0, Y, buf, Font_7x10, ST7735_WHITE,
                               ST7735_BLACK);
            display->hold_pins();
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read data");
            display->unhold_pins();
            display->write_str(0, Y, "Mag Failed", Font_7x10, ST7735_WHITE,
                               ST7735_BLACK);
            display->hold_pins();
        }
        esp_pm_lock_release(lock);
        utils::delay_ms(HEADING_TASK_SLEEP); // Adjust delay as needed
    }
}

void heading_astrolavos_task(void* args)
{
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "heading_lock", &lock);
    esp_pm_lock_acquire(lock);
    astrolavos::Astrolavos* astrolavos_app =
        reinterpret_cast<astrolavos::Astrolavos*>(args);
    QMC5883L qmc5883l;

    if (qmc5883l.init() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize QMC5883L");
        astrolavos_app->updateHealthMagnetometer(
            astrolavos::magnetometer_health_t::MAGNETOMETER_UNINITIALIZED);
        esp_pm_lock_release(lock);
        vTaskSuspend(nullptr);
    }

    int16_t x, y, z;
    esp_pm_lock_release(lock);
    while (true)
    {
        esp_pm_lock_acquire(lock);
        if (qmc5883l.read_raw(x, y, z) == ESP_OK)
        {
            uint16_t heading = qmc5883l.get_heading();
            ESP_LOGI(TAG, "Heading: %u", heading);
            astrolavos_app->updateHeading(heading);
            astrolavos_app->updateHealthMagnetometer(
                astrolavos::magnetometer_health_t::MAGNETOMETER_HEALTHY);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read data");
            astrolavos_app->updateHealthMagnetometer(
                astrolavos::magnetometer_health_t::MAGNETOMETER_ERROR);
        }
        esp_pm_lock_release(lock);
        utils::delay_ms(astrolavos_app->getSleepDuration()->heading);
    }
}
