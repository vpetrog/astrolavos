/**
 * @file AstrolavosPairedDevice.cpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief  IMplementatoin of the Astrolavos paired devices methods
 * @version 0.1
 * @date 2025-06-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "AstrolavosPairedDevice.hpp"
#include "esp_log.h"
#include <cmath>
#include <cstring>

namespace astrolavos

{
AstrolavosPairedDevice::AstrolavosPairedDevice()
{
    _id = ID_ASTROLAVOS_NOT_INITIALIZED; // Default ID for uninitialized device
    _colour = 0x0000;
    _is_active = false;
    _name[0] = '\0';
    _coordinates.latitude = std::nanf("Not Initialised");
    _coordinates.longitude = std::nanf("Not Initialised");
}

int AstrolavosPairedDevice::getId() { return _id; }

void AstrolavosPairedDevice::updateCoordinates(gnss_location_t coordinates)
{
    _coordinates = coordinates;
}

gnss_location_t AstrolavosPairedDevice::getCoordinates()
{
    return _coordinates;
}

void AstrolavosPairedDevice::configure(int id, uint16_t colour,
                                       const char* name)
{
    _id = id;
    _colour = colour;
    setName(name);
    _is_active = true;
    _coordinates.latitude = std::nanf("Not Initialised");
    _coordinates.longitude = std::nanf("Not Initialised");
}

void AstrolavosPairedDevice::setColour(uint16_t new_colour)
{
    _colour = new_colour;
}

uint16_t AstrolavosPairedDevice::getColour() { return _colour; }

const char* AstrolavosPairedDevice::getName() { return _name; }

void AstrolavosPairedDevice::setName(const char* new_name)
{
    if (!new_name)
    {
        _name[0] = '\0';
        return;
    }

    size_t max_copy = sizeof(_name) - 1;
    size_t len = strnlen(new_name, max_copy);

    if (len == max_copy)
    {
        ESP_LOGW("AstrolavosPairedDevice", "Name too long, truncating");
    }

    // Copy and NUL-terminate
    memcpy(_name, new_name, len);
    _name[len] = '\0';
}

bool AstrolavosPairedDevice::isActive() { return _is_active; }
void AstrolavosPairedDevice::setActive(bool active) { _is_active = active; }
} // namespace astrolavos
