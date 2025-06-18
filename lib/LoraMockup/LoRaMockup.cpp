/**
 * @file LoRaMockup.cpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief
 * @version 0.1
 * @date 2025-06-17
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "LoRaMockup.hpp"
#include "esp_pm.h"
#include "esp_random.h"
#include <Astrolavos.hpp>
#include <cmath>
#include <cstdlib>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <utils.hpp>

constexpr const char* TAG_RECEIVER = "LoRaMockupReceiver";
constexpr const char* TAG_TRANSMITTER = "LoRaMockupTransmitter";

constexpr int LORA_MOCKUP_RECEIVER_TASK_STACK_SLEEP_BASE = 2000;
constexpr int LORA_MOCKUP_TRANSMITTER_TASK_STACK_SLEEP = 15000;

#if defined(ASTROLAVOS_MOCKUP_LORA_RECEIVER)
void loraMockupInitReceiver_task(void* args)
{
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "loraMockupInitReceiver_lock",
                       &lock);
    esp_pm_lock_acquire(lock);
    astrolavos::Astrolavos* astrolavos_app =
        static_cast<astrolavos::Astrolavos*>(args);
    ESP_LOGI(TAG_RECEIVER, "Starting LoRa Mockup Receiver Task");

    constexpr float METERS_TO_DEGREES_LAT =
        1.0 / 111000.0; /* Approximate conversion factor for latitude */
    constexpr float METERS_TO_DEGREES_LON = 0.00001476; /* Near Berlin */
    constexpr float MAX_OFFSET_METERS = 5000.0;
    constexpr float MAX_OFFSET_DEGREES_LAT =
        METERS_TO_DEGREES_LAT * MAX_OFFSET_METERS;
    constexpr float MAX_OFFSET_DEGREES_LON =
        METERS_TO_DEGREES_LON * MAX_OFFSET_METERS;

    esp_pm_lock_release(lock);
    while (true)
    {
        esp_pm_lock_acquire(lock);
        astrolavos::gnss_location_t current_coords =
            astrolavos_app->getCoordinates();

        if (std::isnan(current_coords.latitude) ||
            std::isnan(current_coords.longitude))
        {
            ESP_LOGW(TAG_RECEIVER,
                     "No valid coordinates available, waiting...");
            utils::delay_ms(LORA_MOCKUP_RECEIVER_TASK_STACK_SLEEP_BASE);
            continue;
        }

        int target_device_id;
        do
        {
            target_device_id = esp_random() % ASTROLAVOS_NUMBER_OF_DEVICES;
        } while (target_device_id == astrolavos_app->getId());

        /* generate the sign */
        int sign_lat = (esp_random() % 2) ? 1 : -1;
        int sign_lon = (esp_random() % 2) ? 1 : -1;

        /* Generate random coordinates by adding random offsets [0, 5000] meters
         */
        float random_multiplier_lat =
            sign_lat * (esp_random() * 1.0) / (UINT32_MAX * 1.0);
        float random_multiplier_lon =
            sign_lon * (esp_random() * 1.0) / (UINT32_MAX * 1.0);

        float lat_offset = MAX_OFFSET_DEGREES_LAT * random_multiplier_lat;
        float lon_offset = MAX_OFFSET_DEGREES_LON * random_multiplier_lon;

        astrolavos::gnss_location_t random_coords;
        random_coords.latitude = current_coords.latitude + lat_offset;
        random_coords.longitude = current_coords.longitude + lon_offset;
        random_coords.ts = esp_timer_get_time();

        bool wants_to_meet =
            (esp_random() % 4) ? false : true; /* 25% chance to wants to meet */

        astrolavos::device_data_t random_data = {
            .coordinates = random_coords, .wants_to_meet = wants_to_meet};
        esp_err_t result =
            astrolavos_app->updateDevice(target_device_id, random_data);

        if (result == ESP_OK)
        {
            ESP_LOGI(TAG_RECEIVER,
                     "Updated device %d coordinates: lat=%.6f, lon=%.6f, "
                     "wants_to_meet=%s",
                     target_device_id, random_coords.latitude,
                     random_coords.longitude, wants_to_meet ? "true" : "false");
        }
        
        else
        {
            ESP_LOGE(TAG_RECEIVER, "Failed to update device %d coordinates",
                     target_device_id);
        }

        /* Random delay before the next update */
        int random_delay = esp_random() % 3501; // [0, 3500]
        esp_pm_lock_release(lock);

        utils::delay_ms(LORA_MOCKUP_RECEIVER_TASK_STACK_SLEEP_BASE +
                        random_delay);
    }
}

void loraMockupInitTransmitter_task(void* args)
{
    /* TODO currently not Used */
    ESP_LOGE(TAG_TRANSMITTER,
             "LoRa Mockup Transmitter Task is not implemented.");
    vTaskDelete(NULL);
}
#else
void loraMockupInitReceiver_task(void* args)
{
    ESP_LOGE(TAG_RECEIVER, "LoRa Mockup Receiver Task is not implemented.");
    vTaskDelete(NULL);
}
void loraMockupInitTransmitter_task(void* args)
{
    ESP_LOGE(TAG_TRANSMITTER,
             "LoRa Mockup Transmitter Task is not implemented.");
    vTaskDelete(NULL);
}
#endif
