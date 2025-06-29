/**
 * @file Astrolavos.hpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief Astrolavos main application header
 * @version 0.1
 * @date 2025-06-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include "AstrolavosPairedDevice.hpp"
#include "Astrolavos_types.hpp"
#include <HT_st7735.hpp>
#include <QMC5883L.hpp>
#include <array>
#include <lora.hpp>

namespace astrolavos
{

class Astrolavos
{
public:
    void init(HT_st7735* display, LoRa* lora);

    /**
     * @brief Update the battery health status.
     *
     * @param percentage Battery percentage (0-100) or BATTERY_STATUS_UNKNOWN if
     * the status is unknown.
     */
    void updateHealthBattery(uint8_t percentage);

    /**
     * @brief Update the GNSS health status.
     *
     * @param num_satellites Number of satellites in view, or GNSS_NO_SATELLITES
     * if no signal is received.
     */
    void updateHealthGNSS(uint8_t num_satellites);

    /**
     * @brief Update the magnetometer health status.
     *
     * @param magnetometer_health_t
     */
    void updateHealthMagnetometer(magnetometer_health_t status);

    /**
     * @brief Update a device with the received data
     *
     * @param id Unique identifier for the device.
     * @param data The data received
     * @return esp_err_t 0 on success, or an error code on failure.TODO: error
     * codes
     */
    esp_err_t updateDevice(int id, const device_data_t& data);

    /**
     * @brief Update the heading of astrolavos
     *
     * @param heading the heading in degrees [0, 360) or NaN if the heading
     * cannot be calculated.
     */
    void updateHeading(float heading);

    /**
     * @brief Update our current coordinates
     *
     * @param coordinates
     */
    void updateCoordinates(const gnss_location_t& coordinates);

    /**
     * @brief Refresh the health bar on the display based on the current health
     * status.
     *
     */
    void refreshHealthBar();

    /**
     * @brief Refresh the display with our I Want To Meet status.
     *
     */
    void refreshIwantToMeet();

    /**
     * @brief Refresh the display with the latest information towards the device
     * with the given ID.
     *
     * @param id the target device ID to refresh
     */
    void refreshDevice(int id);

    /**
     * @brief Get the Sleep Duration
     *
     * @return const sleep_duration_t*
     */
    const sleep_duration_t* getSleepDuration();

    /**
     * @brief Get the current coordinates
     *
     * @return gnss_location_t
     */
    gnss_location_t getCoordinates();

    /**
     * @brief Get the ID of astrolavos
     *
     * @return int the ID of astrolavos or ID_ASTROLAVOS_NOT_INITIALIZED
     */
    int getId();

    /**
     * @brief Trigger the isolation mode. A very minimal method to be triggered
     * from the ISR
     *
     */
    void triggerIsolationMode();

    /**
     * @brief Trigger the IWTM mode. A very minimal method to be triggered
     * from the ISR
     *
     */
    void triggerIWTM();

    /**
     * @brief Check if the isolation mode is triggered
     *
     * @return true
     * @return false
     */
    bool isIsolationModeTriggered();

    /**
     * @brief Check if the I Want to Meet Mode is triggered
     *
     * @return true
     * @return false
     */
    bool isIWMTTriggered();

    /**
     * @brief Update the mode of operation of Astrolavos. depending on whether
     * we are transitioning to isolation or normal mode. Currently things that
     * are changing are the sleep durations and the display.
     *
     */
    void updateIsolationMode();

    /**
     * @brief Get our current isolation mode.
     *
     * @return true if isolation mode is set
     * @return false otherwise
     */
    bool getIsolationMode();

    /**
     * @brief Handle the IWTM mode trigger.
     */
    void updateIWantToMeet();

    /**
     * @brief Check if Astrolavos is booted.
     *
     */
    bool isBooted();

    /**
     * @brief Check if the user has indicated that they want to perform setup
     *
     * @return true
     * @return false
     */
    bool isSetupRequested();

    /**
     * @brief Request to enter setup instead of the main application.
     *
     */
    void requestSetup();

    /**
     * @brief Astrolavos goes into setup mode. IN this mode the user can
     * configure various settings and operational parameters.
     *
     */
    void setupMode();

    /**
     * @brief Set a pointer to the magnetometer instance.
     *
     * @param magnetometer
     */
    void setMagnetometer(QMC5883L* magnetometer);

    /**
     * @brief Get LoRa.
     *
     * @return LoRa object
     */
    LoRa* getLoRa();

    /**
     * @brief Construct an application message to be sent via LoRa.
     *
     * @return application_message_t
     */
    application_message_t constructMessage();

    /**
     * @brief Handles a received message from LoRa.
     *
     * @param msg
     */
    void handleReceivedMessage(application_message_t msg);

private:
    /**
     * @brief Calculate the heading to the device with the given ID.
     *
     * @param id
     * @return float the degrees [0, 360) or -1 if the heading cannot be
     * calculated.
     */
    esp_err_t calculateHeading(int id, float& heading);

    /**
     * @brief Calculate the direction quarterion based on the heading angle.
     *
     * @param heading_angle
     * @return direction_t
     */
    direction_t calculateDirectionQuart(float heading);

    /**
     * @brief  Print the direction + null terminate
     *
     * @param direction
     * @param buf a buf of size 3 to hold the direction string and null
     * termination
     */
    void printDirection(direction_t direction, char buf[3]);

    /**
     * @brief Calculate the distance to the device with the given ID.
     *
     * @param id
     * @return float the distance in meters or -1 if the distance cannot be
     * calculated.
     */
    esp_err_t calculateDistance(int id, float& distance);

    /**
     * @brief Initialise the device array with any device specific settings
     *
     */
    void initialisePairedDevices();

    /**
     * @brief Get the Device object
     *
     * @param id
     * @return AstrolavosPairedDevice*
     */
    AstrolavosPairedDevice* getDevice(int id);

    /**
     * @brief Initialize the switch interrupt for the Astrolavos system.
     *
     * This function sets up the GPIO pin for the switch and configures the
     * interrupt handler to respond to switch events. The switch will be used
     * to toggle the isolation mode on/off
     */
    void initSwitchInterrupt();

    /**
     * @brief Initialize the I Want To Meet (IWTM) interrupt.
     *
     * This function sets up the GPIO pin for the switch and configures the
     * interrupt handler to respond to switch events. The switch will be used
     * to toggle the IWTM mode on/off
     */
    void initIWTMInterrupt();

    std::array<AstrolavosPairedDevice, ASTROLAVOS_NUMBER_OF_DEVICES>
        _devices;                  /* Array of paired devices */
    health_status_t _healthStatus; /* Health status of the Astrolavos system */
    heading_t _heading;            /* Heading information of Astrolavos */
    HT_st7735* _display;           /* Reference to the display */
    QMC5883L* _magnetometer = nullptr; /* Pointer to Magnetometer instance */
    gnss_location_t _coordinates;      /* Coordinates of Astrolavos */
    int _id = ID_ASTROLAVOS_NOT_INITIALIZED; /* Our ID processed */
    char _name[6];                           /* Name of the Astrolavos device */
    uint16_t _color = 0x0000;
    SemaphoreHandle_t _health_mutex = nullptr;
    bool _isolation_mode_triggered = false; /* Isolation mode flag */
    bool _isolation_mode = false;           /* Isolation mode */
    const sleep_duration_t* _sleep_duration = nullptr;
    bool _i_want_to_meet = false; /* Indicates whether I want to meet */
    bool _i_want_to_meet_mode_triggered = false; /* I Want To Meet mode flag */
    bool _is_booted = false;       /* Indicates whether Astrolavos is booted */
    bool _setup_requested = false; /* Indicates whether setup is requested */
    LoRa* _lora;
};

typedef struct
{
    HT_st7735* display;
    LoRa* lora;
    astrolavos::Astrolavos* app; /* Reference to the Astrolavos instance */
} astrolavos_args_t;

} // namespace astrolavos

void astrolavos_task(void* args);
