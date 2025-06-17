#include <RadioLib.h>

#include <Astrolavos.hpp>
#include <lora.hpp>
#include <pins.hpp>
#include <radiolib_esp32s3_hal.hpp>
#include <utils.hpp>

static const char* TAG = "lora";
static const char* RX_TAG = "lora_rx";
static const char* TX_TAG = "lora_tx";

LoRa::LoRa()
    : hal{heltec::PIN_LORA_SCK, heltec::PIN_LORA_MISO, heltec::PIN_LORA_MOSI},
      // Consulted
      // https://github.com/IanBurwell/DynamicLRS/blob/c26f7f8dcca0c1b70af0aa6aee3aba3a1652aba6/sdkconfig.ht_tracker
      // for the pins
      mod{&hal, heltec::PIN_LORA_NSS, heltec::PIN_LORA_DIO1,
          heltec::PIN_LORA_RST, heltec::PIN_LORA_BUSY},
      radio{&mod}
{
}

void LoRa::init()
{
    int16_t err = radio.begin();
    if (err == RADIOLIB_ERR_NONE)
    {
        ESP_LOGI(TAG, "Initialized LoRa");
    }
    else
    {
        ESP_LOGE(TAG, "Init LoRa failed: %d", err);
        while (true)
        {
            utils::delay_ms(1000);
        }
    }
}

SX1262& LoRa::getRadio() { return radio; }

void lora_rx_astrolavos_task(void* args)
{
    astrolavos::Astrolavos* astrolavos_app =
        static_cast<astrolavos::Astrolavos*>(args);
    LoRa lora{astrolavos_app->getLoRa()};
    SX1262 radio{lora.getRadio()};

    // TODO: Set up receiver
    while (true)
    {
        utils::delay_ms(1000);
    }
}

static void IRAM_ATTR packet_sent(void) { ESP_LOGI(RX_TAG, "Packet set!"); }

void lora_tx_astrolavos_task(void* args)
{
    astrolavos::Astrolavos* astrolavos_app =
        static_cast<astrolavos::Astrolavos*>(args);
    LoRa lora{astrolavos_app->getLoRa()};
    SX1262 radio{lora.getRadio()};
    ESP_LOGI(RX_TAG, "LoRa TX task started");
    radio.setPacketSentAction(packet_sent);

    while (true)
    {
        ESP_LOGI(TX_TAG, "Pinging");
        int16_t err = radio.startTransmit("ping");
        if (err == RADIOLIB_ERR_NONE)
        {
            ESP_LOGI(TX_TAG, "Pinged");
        }
        else
        {
            ESP_LOGE(TX_TAG, "Failed to ping: %d", err);
        }
        utils::delay_ms(1000);
    }
}
