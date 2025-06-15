/**
 * @file AstrolavosPairedDevice.hpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief Astrolavos paired device class
 * @version 0.1
 * @date 2025-06-15
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once

#include "Astrolavos_types.hpp"

namespace astrolavos
{
class AstrolavosPairedDevice
{
public:
    AstrolavosPairedDevice() = default;
    AstrolavosPairedDevice(int id, char name[6], uint16_t color);

    gnss_location_t getCoordinates();
    void updateCoordinates(gnss_location_t coordinates);
    void configure(int id, uint16_t colour, const char* name);
    uint16_t getColour();
    void setColour(uint16_t new_colour);
    const char* getName();
    void setName(const char* new_name);
    bool isActive();
    void setActive(bool active);
    int getId();

private:
    int _id;                /* Unique identifier for the device */
    gnss_location_t _coordinates; /* GNSS coordinates of the device */
    uint16_t _colour;             /* Color of the device in RGB565 format */
    char _name[7];                /* Name Associated to the device */
    bool _is_active; /* Is the device active? TODO: Do not show unused devices
                       future extension */
    /*TODO: Probably we will need more fields for LoRa */
};

} // namespace astrolavos