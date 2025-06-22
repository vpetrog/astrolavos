/**
 * @file    QMC5883L.hpp
 * @author  Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief   Magnetometer Driver Header with calibration support
 * @version 0.1
 * @date    2025-06-15
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"

constexpr const char* NVS_CALIBRATION_NAMESPACE = "qmc5883l";
constexpr const char* NVS_CALIBRATION_KEY = "cal_data";

/** Calibration parameters container */
typedef struct
{
    int16_t minX, maxX, minY, maxY, minZ, maxZ;
    float offsetX, offsetY, offsetZ;
    float scaleX, scaleY, scaleZ;
} calibration_data_t;

class QMC5883L
{
public:
    QMC5883L(i2c_port_t port = I2C_NUM_0, gpio_num_t sda = GPIO_NUM_45,
             gpio_num_t scl = GPIO_NUM_46);
    esp_err_t init();
    esp_err_t read_raw(int16_t& x, int16_t& y, int16_t& z);
    float get_heading();

    /**
     * @brief  Collect `samples` while you rotate the sensor,
     *         then compute hard- and soft-iron corrections.
     * @param  samples   Number of readings to gather (0 skips sampling)
     * @param  delay_ms  Delay (ms) between each sample
     */
    esp_err_t calibrate(uint16_t samples = 500, uint16_t delay_ms = 100);

    /**
     * @brief  Load pre-computed extrema (e.g. from flash),
     *         then recompute offsets & scales.
     */
    void setCalibrationData(int16_t xMin, int16_t xMax, int16_t yMin,
                            int16_t yMax, int16_t zMin, int16_t zMax);

    /** @brief  Retrieve last-used calibration data */
    const calibration_data_t& getCalibrationData() const { return _cal; }

    /** @brief  Save _cal struct to NVS under given namespace & key */
    esp_err_t saveCalibration(const char* ns = NVS_CALIBRATION_NAMESPACE,
                              const char* key = NVS_CALIBRATION_KEY) const;

    /** @brief  Load _cal struct from NVS; recompute offsets & scales */
    esp_err_t loadCalibration(const char* ns, const char* key);

    /**
     * @brief  Clear the calibration data from NVS.
     *
     * @param ns
     * @param key
     * @return esp_err_t
     */
    esp_err_t eraseCalibration(const char* ns = NVS_CALIBRATION_NAMESPACE,
                               const char* key = NVS_CALIBRATION_KEY);

private:
    i2c_port_t _port;
    gpio_num_t _sda, _scl;
#if defined(QMC5883L_USE_QMC5883L)
    static constexpr uint8_t QMC5883L_ADDR = 0x0D;
#elif defined(QMC5883L_USE_QMC5883P)
    static constexpr uint8_t QMC5883L_ADDR = 0x2C;
#endif
    esp_err_t write_reg(uint8_t reg, uint8_t val);
    esp_err_t read_bytes(uint8_t reg, uint8_t* data, size_t len);

    // Calibration state
    calibration_data_t _cal{};
    bool _isCalibrated = false;

    /** @brief  Subtract bias and apply scale to one raw sample */
    void applyCalibration(int16_t& x, int16_t& y, int16_t& z);

    /** @brief  Like read_raw(), but applies calibration if available */
    esp_err_t read_calibrated(int16_t& x, int16_t& y, int16_t& z);
};

void heading_task(void* args);
void heading_astrolavos_task(void* args);
