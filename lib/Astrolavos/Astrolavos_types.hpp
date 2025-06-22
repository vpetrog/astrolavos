/**
 * @file Astrolavos_types.hpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief Types and constants for the Astrolavos shared memory
 * @version 0.1
 * @date 2025-06-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>

#ifndef ASTROLAVOS_NUMBER_OF_DEVICES
#define ASTROLAVOS_NUMBER_OF_DEVICES 4 // Default number of devices#
#endif

#ifndef ASTROLAVOS_MAGIC_CODE
#define ASTROLAVOS_MAGIC_CODE 0xE7F4 /* Magic code to check validity */
#endif
namespace astrolavos
{

constexpr uint8_t ID_ASTROLAVOS_NOT_INITIALIZED =
    0xff; /* Astrolavos not initialized */

constexpr uint8_t BATTERY_STATUS_UNKNOWN = 0xff; /* Battery status unknown */

typedef struct
{
    uint8_t
        percentage; /* Battery percentage (0-100) or BATTERY_STATUS_UNKNOWN */
    int64_t ts;     /* Timestamp of the last update in usec */
} battery_status_t;

constexpr uint8_t GNSS_NO_SATELLITES = 0xFF; /* No satellites in view */

constexpr uint8_t GNSS_MAX_SATELLITES =
    96; /* Max Number of Satellites according to UC650 datasheet */

typedef struct
{
    uint8_t num_satellites; /* Number of satellites in view or
                               GNSS_NO_SATELLITES if no signal is received */
    int64_t ts;             /* Timestamp of the last update in usec */

} gnss_status_t;

typedef enum
{
    MAGNETOMETER_HEALTHY = 0,      /* Magnetometer healthy */
    MAGNETOMETER_ERROR = 1,        /* Magnetometer error */
    MAGNETOMETER_UNINITIALIZED = 2 /* Magnetometer not initialized */
} magnetometer_health_t;

typedef struct
{
    magnetometer_health_t status; /* Magnetometer status */
    int64_t ts;                   /* Timestamp of the last update in usec */
} magnetometer_status_t;

/* TODO: Add LoRa related health monitoring*/

typedef struct
{
    battery_status_t battery;           /* Battery status */
    gnss_status_t gnss;                 /* GNSS status */
    magnetometer_status_t magnetometer; /* Magnetometer status */
    int64_t ts; /* Timestamp of the last update in usec */
    /* TODO: Add LoRa Status */
} health_status_t;

typedef struct
{
    float latitude;  /* Latitude in degrees or NaN if not available */
    float longitude; /* Longitude in degrees or NaN if not available */
    int64_t ts;      /* Timestamp of the last update in usec */
} gnss_location_t;

typedef struct
{
    float heading; /* Heading in degrees (0-360)  or Nan If not available */
    int64_t ts;    /* Timestamp of the last update in usec */
} heading_t;

typedef enum
{
    ASTROLAVOS_DIRECTION_FRONT,
    ASTROLAVOS_DIRECTION_FRONT_LEFT,
    ASTROLAVOS_DIRECTION_LEFT,
    ASTROLAVOS_DIRECTION_BACK_LEFT,
    ASTROLAVOS_DIRECTION_BACK,
    ASTROLAVOS_DIRECTION_BACK_RIGHT,
    ASTROLAVOS_DIRECTION_RIGHT,
    ASTROLAVOS_DIRECTION_FRONT_RIGHT,
    ASTROLAVOS_DIRECTION_UNKNOWN
} direction_t;

typedef struct
{
    std::size_t heading;
    std::size_t main_app_refresh;
    std::size_t battery;
    std::size_t blinking;
    std::size_t lora_rx;
    std::size_t lora_tx;
} sleep_duration_t;

typedef struct
{
    int id;
    const char name[6];
    uint16_t colour;
} paired_device_auto_config_t;

typedef struct
{
    gnss_location_t coordinates; /* GNSS coordinates of the device */
    bool wants_to_meet; /* Indicates whether this device wants to meet */

    /*TODO: We should probably add a checksum or some sort of signature to
     * validate the authenticity/validity */
} device_data_t;

typedef struct
{
    uint16_t magic;        /* Magic NUmber to check validity */
    uint8_t id;            /* Sender ID */
    device_data_t payload; /* The actual Payload */

} application_message_t;

/* TODO: Any LoRa Related Structs */

} // namespace astrolavos
