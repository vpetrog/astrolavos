/**
 * @file lora.cpp
 * @author Fotis Xenakis (foxen@windowslive.com)
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief LoRa Module via RadioLib
 * @version 0.1
 * @date 2025-06-21
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "esp_pm.h"

#include <RadioLib.h>

#include "esp_random.h"
#include <Astrolavos.hpp>
#include <lora.hpp>
#include <pins.hpp>
#include <radiolib_esp32s3_hal.hpp>
#include <utils.hpp>

static const char* TAG = "lora";
static const char* RX_TAG = "lora_rx";
static const char* TX_TAG = "lora_tx";

constexpr size_t SX1262_BOOT_TIME_DELAY = 5000;
constexpr uint16_t SX1262_MAX_RANDOM_TX_DELAY = 2000; /* 2 Seconds */
volatile bool receivedFlag = false;

#ifndef ASTROLAVOS_LORA_SYNC_WORD
#define ASTROLAVOS_LORA_SYNC_WORD 0x74
#endif

extern TaskHandle_t rxTaskHandle;
LoRa::LoRa()
    : hal{heltec::PIN_LORA_SCK, heltec::PIN_LORA_MISO, heltec::PIN_LORA_MOSI},
      // Consulted
      // https://github.com/IanBurwell/DynamicLRS/blob/c26f7f8dcca0c1b70af0aa6aee3aba3a1652aba6/sdkconfig.ht_tracker
      // for the pins
      mod{&hal, heltec::PIN_LORA_NSS, heltec::PIN_LORA_DIO1,
          heltec::PIN_LORA_RST, heltec::PIN_LORA_BUSY},
      radio{&mod}
{
    ESP_LOGI(TAG, "Constructing");
}

void LoRa::init()
{
    // TODO: Move to constructor?
    // hal = EspHal(heltec::PIN_LORA_SCK, heltec::PIN_LORA_MISO,
    //              heltec::PIN_LORA_MOSI);
    // Consulted
    // https://github.com/IanBurwell/DynamicLRS/blob/c26f7f8dcca0c1b70af0aa6aee3aba3a1652aba6/sdkconfig.ht_tracker
    // for the pins
    _lock = xSemaphoreCreateMutex();
    int16_t err = radio.begin();
    radio.setSyncWord(ASTROLAVOS_LORA_SYNC_WORD);

    if (err == RADIOLIB_ERR_NONE)
    {
        ESP_LOGI(TAG, "Initialized LoRa");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to initialize LoRa: %d", err);
        vTaskSuspend(nullptr);
    }
}

SX1262* LoRa::getRadio()
{
    xSemaphoreTake(_lock, portMAX_DELAY);
    return &radio;
}

void LoRa::putRadio() { xSemaphoreGive(_lock); }
