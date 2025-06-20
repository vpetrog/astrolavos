#pragma once

#include <RadioLib.h>

#include <radiolib_esp32s3_hal.hpp>

class LoRa
{
public:
    LoRa();
    void init();
    SX1262& getRadio();
    void putRadio();

private:
    EspHal hal;
    Module mod;
    SX1262 radio;
    SemaphoreHandle_t _lock;
};

void lora_rx_astrolavos_task(void* args);
void lora_tx_astrolavos_task(void* args);
