/**
 * @file Astrolavos.cpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief Astrolavos main application implementation
 * @version 0.1
 * @date 2025-06-15
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "Astrolavos.hpp"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <HT_st7735_fonts.hpp>
#include <TinyGPS++.hpp>
#include <cmath>
#include <pins.hpp>
#include <utils.hpp>

constexpr const char* TAG = "Astrolavos";

namespace astrolavos
{
extern paired_device_auto_config_t
    paired_device_auto_config[ASTROLAVOS_NUMBER_OF_DEVICES];
extern paired_device_auto_config_t this_device;

constexpr size_t ASTROLAVOS_WELCOME_SLEEP = 3 * 1000;

constexpr float EARTH_RADIUS_M = 6371000.0f; // Radius of the Earth in meters

constexpr float ASTROLAVOS_MAXIMUM_ACCEPTABLE_DISTANCE = 30000; /* 30km */

const sleep_duration_t normal_sleep_duration = {
    .heading = 1000,          /* 1 second */
    .main_app_refresh = 2000, /* 2 seconds */
    .battery = 60000,         /* 1 minute */
    .blinking = 1500          /* 3.5 seconds */
};

const sleep_duration_t isolation_sleep = {
    .heading = 300000,        /* 5 minute */
    .main_app_refresh = 5000, /* 5 seonds, this stays the same and instead it
                                 changes via an if :() */
    .battery = 300000,        /* 5 minutes */
    .blinking = 5000          /* 5 seconds */
};

void Astrolavos::updateHealthBattery(uint8_t percentage)
{

    if (percentage > 100)
    {
        ESP_LOGE(TAG, "Invalid battery percentage: %d", percentage);
        percentage = BATTERY_STATUS_UNKNOWN;
    }
    assert(xSemaphoreTake(_health_mutex, portMAX_DELAY) == pdTRUE);
    _healthStatus.battery.percentage = percentage;
    _healthStatus.battery.ts = esp_timer_get_time();
    assert(xSemaphoreGive(_health_mutex) == pdTRUE);
}

void Astrolavos::updateHealthGNSS(uint8_t num_satellites)
{
    xSemaphoreTake(_health_mutex, portMAX_DELAY);
    if (num_satellites > 96 &&
        num_satellites != GNSS_NO_SATELLITES) /* Max Number of Satellites
                                                 according to the datasheet */
    {
        ESP_LOGE(TAG, "Invalid number of satellites: %d", num_satellites);
        num_satellites = GNSS_NO_SATELLITES; // Set to no satellites if invalid
    }
    else
    {
        _healthStatus.gnss.num_satellites = num_satellites;
    }
    _healthStatus.gnss.ts = esp_timer_get_time();
    xSemaphoreGive(_health_mutex);
}
void Astrolavos::updateHealthMagnetometer(magnetometer_health_t status)
{
    xSemaphoreTake(_health_mutex, portMAX_DELAY);
    _healthStatus.magnetometer.status = status;
    _healthStatus.magnetometer.ts = esp_timer_get_time();
    xSemaphoreGive(_health_mutex);
}

int Astrolavos::updateDevice(int id, const device_data_t data)
{
    if (id < 0 || id >= ASTROLAVOS_NUMBER_OF_DEVICES)
    {
        ESP_LOGE(TAG, "Invalid device ID: %d", id);
        return ESP_ERR_INVALID_ARG;
    }

    AstrolavosPairedDevice* device = getDevice(id);
    if (!device)
    {
        ESP_LOGE(TAG, "Device with ID %d not found", id);
        return ESP_ERR_NOT_FOUND;
    }

    device->updateDevice(data);
    return ESP_OK;
}

void Astrolavos::updateHeading(float heading)
{
    if (heading < 0 || heading >= 360)
    {
        ESP_LOGE(TAG, "Invalid heading: %f", heading);
        return;
    }
    _heading.heading = heading;
    _heading.ts = esp_timer_get_time();
}

void Astrolavos::updateCoordinates(const gnss_location_t& coordinates)
{
    _coordinates.latitude = coordinates.latitude;
    _coordinates.longitude = coordinates.longitude;
    _coordinates.ts = coordinates.ts;

    ESP_LOGI(TAG, "Updated coordinates: Lat: %f, Lon: %f",
             _coordinates.latitude, _coordinates.longitude);
}

esp_err_t Astrolavos::calculateHeading(int id, float& heading)
{
    if (id < 0 || id >= ASTROLAVOS_NUMBER_OF_DEVICES)
    {
        ESP_LOGE(TAG, "Invalid device ID: %d", id);
        return ESP_ERR_INVALID_ARG;
    }

    AstrolavosPairedDevice* device = getDevice(id);
    if (!device)
    {
        ESP_LOGE(TAG, "Device with ID %d not found", id);
        return ESP_ERR_NOT_FOUND;
    }

    gnss_location_t target = device->getCoordinates();
    if (target.latitude == std::nanf("No Latitude") ||
        target.longitude == std::nanf("No Longitude"))
    {
        return ESP_ERR_INVALID_ARG;
    }

    float lat1 = _coordinates.latitude * DEG_TO_RAD;
    float lat2 = target.latitude * DEG_TO_RAD;
    float dLon = (target.longitude - _coordinates.longitude) * DEG_TO_RAD;

    float y = sin(dLon) * cos(lat2);
    float x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);

    float headingRad = atan2(y, x);
    float headingDeg = headingRad * RAD_TO_DEG;

    heading = fmodf((headingDeg + 360.0f), 360.0f);

    return ESP_OK;
}

direction_t Astrolavos::calculateDirectionQuart(float target_heading)
{
    float relativeHeading = target_heading - _heading.heading;

    while (relativeHeading < 0)
    {
        relativeHeading += 360.0f;
    }
    while (relativeHeading >= 360.0f)
    {
        relativeHeading -= 360.0f;
    }

    if (relativeHeading >= 337.5f || relativeHeading < 22.5f)
    {
        return ASTROLAVOS_DIRECTION_FRONT;
    }
    else if (relativeHeading >= 22.5f && relativeHeading < 67.5f)
    {
        return ASTROLAVOS_DIRECTION_FRONT_RIGHT;
    }
    else if (relativeHeading >= 67.5f && relativeHeading < 112.5f)
    {
        return ASTROLAVOS_DIRECTION_RIGHT;
    }
    else if (relativeHeading >= 112.5f && relativeHeading < 157.5f)
    {
        return ASTROLAVOS_DIRECTION_BACK_RIGHT;
    }
    else if (relativeHeading >= 157.5f && relativeHeading < 202.5f)
    {
        return ASTROLAVOS_DIRECTION_BACK;
    }
    else if (relativeHeading >= 202.5f && relativeHeading < 247.5f)
    {
        return ASTROLAVOS_DIRECTION_BACK_LEFT;
    }
    else if (relativeHeading >= 247.5f && relativeHeading < 292.5f)
    {
        return ASTROLAVOS_DIRECTION_LEFT;
    }
    else if (relativeHeading >= 292.5f && relativeHeading < 337.5f)
    {
        return ASTROLAVOS_DIRECTION_FRONT_LEFT;
    }
    else
    {
        return ASTROLAVOS_DIRECTION_UNKNOWN;
    }
}

void Astrolavos::printDirection(direction_t direction, char buf[3])
{
    switch (direction)
    {
        case ASTROLAVOS_DIRECTION_FRONT:
            snprintf(buf, 3, "F");
            break;
        case ASTROLAVOS_DIRECTION_FRONT_RIGHT:
            snprintf(buf, 3, "FR");
            break;
        case ASTROLAVOS_DIRECTION_RIGHT:
            snprintf(buf, 3, "R");
            break;
        case ASTROLAVOS_DIRECTION_BACK_RIGHT:
            snprintf(buf, 3, "BR");
            break;
        case ASTROLAVOS_DIRECTION_BACK:
            snprintf(buf, 3, "B");
            break;
        case ASTROLAVOS_DIRECTION_BACK_LEFT:
            snprintf(buf, 3, "BL");
            break;
        case ASTROLAVOS_DIRECTION_LEFT:
            snprintf(buf, 3, "L");
            break;
        case ASTROLAVOS_DIRECTION_FRONT_LEFT:
            snprintf(buf, 3, "FL");
            break;
        default:
            snprintf(buf, 3, "?");
            break;
    }
}

esp_err_t Astrolavos::calculateDistance(int id, float& distance)
{
    if (id < 0 || id >= ASTROLAVOS_NUMBER_OF_DEVICES)
    {
        ESP_LOGE(TAG, "Invalid device ID: %d", id);
        return ESP_ERR_INVALID_ARG;
    }
    AstrolavosPairedDevice* device = getDevice(id);
    if (!device)
    {
        ESP_LOGE(TAG, "Device with ID %d not found", id);
        return ESP_ERR_NOT_FOUND;
    }
    gnss_location_t target = device->getCoordinates();
    if (std::isnan(target.latitude) || std::isnan(target.longitude))
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (std::isnan(_coordinates.latitude) || std::isnan(_coordinates.longitude))
    {
        ESP_LOGE(TAG, "Astrolavos coordinates not set");
        return ESP_ERR_INVALID_STATE;
    }

    /* Haversine Calculation of distance */
    float lat1 = _coordinates.latitude * DEG_TO_RAD;
    float lat2 = target.latitude * DEG_TO_RAD;
    float lon1 = _coordinates.longitude * DEG_TO_RAD;
    float lon2 = target.longitude * DEG_TO_RAD;
    float dLat = lat2 - lat1;
    float dLon = lon2 - lon1;

    float sinDLat = sin(dLat / 2.0f);
    float sinDLon = sin(dLon / 2.0f);

    float a_hav = sinDLat * sinDLat + cos(lat1) * cos(lat2) * sinDLon * sinDLon;

    float c = 2.0f * atan2(sqrt(a_hav), sqrt(1.0f - a_hav));
    distance = EARTH_RADIUS_M * c;
    if (distance > ASTROLAVOS_MAXIMUM_ACCEPTABLE_DISTANCE)
        return ESP_FAIL;
    return ESP_OK;
}

void Astrolavos::initialisePairedDevices()
{
    int i = 0;
    for (auto& device : _devices)
    {
        /* Skip Initialising Ourselves */
        if (i == _id)
            i++;

        if (i < ASTROLAVOS_NUMBER_OF_DEVICES)
        {
            device.configure(paired_device_auto_config[i].id,
                             paired_device_auto_config[i].colour,
                             paired_device_auto_config[i].name);
            ESP_LOGI(TAG, "Device %d configured: %s", i, device.getName());
        }
        else
        {
            ESP_LOGE(TAG, "Too many devices configured, skipping device %d", i);
        }
        i++;
    }
}

const sleep_duration_t* Astrolavos::getSleepDuration()
{
    return _sleep_duration;
}

AstrolavosPairedDevice* Astrolavos::getDevice(int id)
{
    if (id < 0)
    {
        ESP_LOGE(TAG, "Invalid device ID: %d", id);
        return nullptr;
    }
    for (auto& device : _devices)
    {
        if (device.getId() == id)
        {
            return &device;
        }
    }
    return nullptr;
}

gnss_location_t Astrolavos::getCoordinates() { return _coordinates; }

int Astrolavos::getId() { return _id; }

void Astrolavos::triggerIsolationMode() { _isolation_mode_triggered = true; }

bool Astrolavos::isIsolationModeTriggered()
{
    return _isolation_mode_triggered;
}

void Astrolavos::updateIsolationMode()
{
    _isolation_mode_triggered = false;
    _isolation_mode = !_isolation_mode;
    if (_isolation_mode)
    {
        _sleep_duration = &isolation_sleep;
        _display->turn_off();
    }
    else
    {
        _sleep_duration = &normal_sleep_duration;
        _display->turn_on();
    }
}

bool Astrolavos::getIsolationMode() { return _isolation_mode; }

bool Astrolavos::getIWantToMeet() { return _i_want_to_meet; }

void Astrolavos::updateIWantToMeet(bool i_want_to_meet)
{
    _i_want_to_meet = i_want_to_meet;
}

bool Astrolavos::isBooted() { return _is_booted; }

bool Astrolavos::isSetupRequested() { return _setup_requested; }

void Astrolavos::requestSetup() { _setup_requested = true; }

void Astrolavos::setupMode()
{
    ESP_LOGI(TAG, "Entering Setup Mode");
    utils::delay_ms(3000);
    esp_restart();
}

void Astrolavos::refreshHealthBar()
{
    /* The Display row is 160 pixels Wide, with at a Font size of 7x10
     * we have a total of 22 characters that can fit in a row. The health
     * bar will have the following format:
     * Batt:xx% GNSS:xx Mag:x
     */
    char buf[23];
    const int Y = 80 - Font_7x10.height;
    _display->unhold_pins();
    _display->fill_rectangle(0, Y, 160, 80, ST7735_BLACK);
    _display->hold_pins();
    char gnss_fixed[3];
    xSemaphoreTake(_health_mutex, portMAX_DELAY);
    if (_healthStatus.gnss.num_satellites == GNSS_NO_SATELLITES)
    {
        snprintf(gnss_fixed, sizeof(gnss_fixed), "xx");
    }
    else
    {
        snprintf(gnss_fixed, sizeof(gnss_fixed), "%d",
                 _healthStatus.gnss.num_satellites % 100);
    }

    char mag_status[4];
    switch (_healthStatus.magnetometer.status)
    {
        case MAGNETOMETER_HEALTHY:
            snprintf(mag_status, sizeof(mag_status), "%u",
                     static_cast<uint16_t>(_heading.heading) % 1000);
            break;
        case MAGNETOMETER_ERROR:
            snprintf(mag_status, sizeof(mag_status), "xxx");
            break;
        case MAGNETOMETER_UNINITIALIZED:
            snprintf(mag_status, sizeof(mag_status), "---");
            break;
        default:
            snprintf(mag_status, sizeof(mag_status), "???");
            break;
    }
    snprintf(buf, sizeof(buf), "Bat:%d%% Sat:%s Hdg:%s",
             _healthStatus.battery.percentage % 100, gnss_fixed, mag_status);
    xSemaphoreGive(_health_mutex);
    _display->unhold_pins();
    _display->write_str(0, Y, buf, Font_7x10, ST7735_WHITE, ST7735_BLACK);
    _display->hold_pins();

    ESP_LOGI(TAG, "Health Bar: %s", buf);
}

void Astrolavos::refreshDevice(int id)
{
    if (id < 0)
    {
        ESP_LOGE(TAG, "Invalid device ID: %d", id);
        return;
    }

    AstrolavosPairedDevice* device = getDevice(id);
    if (!device)
        return;

    bool is_valid = true;
    bool i_want_to_meet;
    float distance;
    float target_absolute_heading;
    if (calculateDistance(id, distance) != ESP_OK ||
        calculateHeading(id, target_absolute_heading) != ESP_OK)
        is_valid = false;

    const int Y = Font_7x10.height * id; /* Assume id [0,4] */
    char buf_name[7];
    char buf_data[17];
    char direction_buf[3];
    uint16_t bg_color = ST7735_BLACK;
    snprintf(buf_name, sizeof(buf_name), "%s", device->getName());
    if (is_valid)
    {
        printDirection(calculateDirectionQuart(target_absolute_heading),
                       direction_buf);
        int target_absolute_heading_int =
            static_cast<int>(target_absolute_heading);
        int distance_int = static_cast<int>(distance);
        snprintf(buf_data, sizeof(buf_data), " %dm go %s (%d)", distance_int,
                 direction_buf, target_absolute_heading_int);
        i_want_to_meet = device->getWantsToMeet();
        if (i_want_to_meet)
            bg_color = ST7735_WHITE;

        ESP_LOGI(TAG,
                 "Device %d: Distance: %dm, Absolute Heading: %d°, WTM: %s", id,
                 static_cast<int>(distance),
                 static_cast<int>(target_absolute_heading),
                 i_want_to_meet ? "True" : "False");
    }
    else
    {
        snprintf(buf_data, sizeof(buf_data), ": No Data");
    }
    _display->unhold_pins();
    _display->fill_rectangle(0, Y, 160, Font_7x10.height, ST7735_BLACK);
    _display->write_str(0, Y, buf_name, Font_7x10, device->getColour(),
                        bg_color);
    _display->write_str(Font_7x10.width * strlen(device->getName()), Y,
                        buf_data, Font_7x10, device->getColour(), ST7735_BLACK);

    _display->hold_pins();
    ESP_LOGI(TAG, "Device %d: %s%s", id, buf_name, buf_data);

    /*TODO: We could perhaps do something with the freshness */
}

void Astrolavos::refreshIwantToMeet()
{
    /*Print our WantToMeet Mode*/
    const uint16_t Y_WTM = 80 - (2 * Font_7x10.height);
    _display->unhold_pins();
    _display->fill_rectangle(0, Y_WTM, 160, Font_7x10.height, ST7735_BLACK);
    if (_i_want_to_meet)
    {
        _display->fill_rectangle(0, Y_WTM, 3 * Font_7x10.width,
                                 Font_7x10.height, _color);
        char wtm_buf[] = "I Want To Meet\0";
        _display->write_str(4 * Font_7x10.width, Y_WTM, wtm_buf, Font_7x10,
                            _color, ST7735_BLACK);
        _display->fill_rectangle(20 * Font_7x10.width, Y_WTM, 160,
                                 Font_7x10.height, _color);
    }
    _display->hold_pins();
    ESP_LOGI(TAG, "I Want To Meet: %s", _i_want_to_meet ? "True" : "False");
}

void usr_button_isr_handler(void* args)
{
    astrolavos::Astrolavos* astrolavos_app =
        static_cast<astrolavos::Astrolavos*>(args);
    if (astrolavos_app)
    {
        if (astrolavos_app->isBooted())
            astrolavos_app->triggerIsolationMode();
        else
            astrolavos_app->requestSetup();
    }
}

void iwtm_isr_handler(void* args)
{
    astrolavos::Astrolavos* astrolavos_app =
        static_cast<astrolavos::Astrolavos*>(args);
    if (astrolavos_app)
    {
        astrolavos_app->updateIWantToMeet(!astrolavos_app->getIWantToMeet());
    }
}

void Astrolavos::initSwitchInterrupt()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << static_cast<uint64_t>(heltec::PIN_USR_SWITCH),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE // falling edge
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0); // pass 0 to use default
    gpio_isr_handler_add(heltec::PIN_USR_SWITCH, usr_button_isr_handler, this);
}

void Astrolavos::initIWTMInterrupt()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << static_cast<uint64_t>(heltec::PIN_IWTM_SWITCH),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE // falling edge
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0); // pass 0 to use default
    gpio_isr_handler_add(heltec::PIN_IWTM_SWITCH, iwtm_isr_handler, this);
}

void Astrolavos::init(HT_st7735* display)
{
    _sleep_duration = &normal_sleep_duration;
    _display = display;
    _id = this_device.id;
    strncpy(_name, this_device.name, sizeof(_name) - 1);
    _color = this_device.colour;
    _health_mutex = xSemaphoreCreateMutex();
    _healthStatus.battery = {BATTERY_STATUS_UNKNOWN, 0};
    _healthStatus.gnss = {GNSS_NO_SATELLITES, 0};
    _healthStatus.magnetometer = {MAGNETOMETER_UNINITIALIZED, 0};
    _coordinates = {std::nanf("No Latitude"), std::nanf("No Longitude"), 0};
    ESP_LOGI(TAG, "Initializing Astrolavos");
    _display->unhold_pins();
    _display->fill_screen(ST7735_BLACK);
    _display->fill_rectangle(0, 0, 160, Font_11x18.height, _color);
    _display->fill_rectangle(0, 80 - Font_11x18.height, 160, 80, _color);
    _display->write_str(25, Font_11x18.height, "Astrolavos", Font_11x18, _color,
                        ST7735_BLACK);
    char buf[4 + 6 + 1]; /* 4 for "Hey ", 6 for name, 1 for null terminator */
    snprintf(buf, sizeof(buf), "Hey %s", _name);
    _display->write_str(3 * Font_11x18.width, Font_11x18.height * 2, buf,
                        Font_11x18, _color, ST7735_BLACK);
    _display->hold_pins();
    initialisePairedDevices();
    initSwitchInterrupt();
    utils::delay_ms(ASTROLAVOS_WELCOME_SLEEP);
    _display->unhold_pins();
    _display->fill_screen(ST7735_BLACK);
    _display->hold_pins();
    initIWTMInterrupt();

    _is_booted = true;
    ESP_LOGI(TAG, "Astrolavos initialized with ID: %d, Name: %s", _id, _name);
}
} // namespace astrolavos

void astrolavos_task(void* args)
{
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "astrolavos_lock", &lock);
    esp_pm_lock_acquire(lock);
    astrolavos::astrolavos_args_t* task_args =
        reinterpret_cast<astrolavos::astrolavos_args_t*>(args);
    astrolavos::Astrolavos* astrolavos_app = task_args->app;
    HT_st7735* display = reinterpret_cast<HT_st7735*>(task_args->display);
    ESP_LOGI(TAG, "Astrolavos Task started");
    astrolavos_app->init(display);
    if (astrolavos_app->isSetupRequested())
    {
        ESP_LOGI(TAG, "Setup requested, entering setup mode");
        astrolavos_app->setupMode();
        esp_pm_lock_release(lock);
    }
    while (true)
    {
        esp_pm_lock_acquire(lock);
        if (astrolavos_app->isIsolationModeTriggered())
        {
            ESP_LOGI(TAG, "Isolation mode triggered");
            astrolavos_app->updateIsolationMode();
        }
        if (!astrolavos_app->getIsolationMode())
        {
            astrolavos_app->refreshHealthBar();
            astrolavos_app->refreshIwantToMeet();
            for (int i = 0; i < ASTROLAVOS_NUMBER_OF_DEVICES; i++)
            {
                astrolavos_app->refreshDevice(i);
            }
        }
        esp_pm_lock_release(lock);
        utils::delay_ms(astrolavos_app->getSleepDuration()->main_app_refresh);
    }
}
