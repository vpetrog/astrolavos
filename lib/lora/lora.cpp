#include "esp_pm.h"

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
    // mod = Module(&hal, heltec::PIN_LORA_NSS, heltec::PIN_LORA_DIO1,
    //              heltec::PIN_LORA_RST, heltec::PIN_LORA_BUSY);
    // radio = SX1262(&mod);
    _lock = xSemaphoreCreateMutex();
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

SX1262* LoRa::getRadio()
{
    xSemaphoreTake(_lock, portMAX_DELAY);
    return &radio;
}

void LoRa::putRadio() { xSemaphoreGive(_lock); }

static constexpr size_t BUF_SIZE = 1024;
uint8_t buf[BUF_SIZE];

void lora_rx_astrolavos_task(void* args)
{
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "lora_rx_lock", &lock);

    utils::delay_ms(10000);
    astrolavos::Astrolavos* astrolavos_app =
        static_cast<astrolavos::Astrolavos*>(args);
    LoRa* lora = astrolavos_app->getLoRa();

    while (true)
    {
        esp_pm_lock_acquire(lock);
        ESP_LOGI(TX_TAG, "Receiving");
        SX1262* radio = lora->getRadio();
        int16_t err = radio->receive(buf, BUF_SIZE);
        lora->putRadio();
        if (err == RADIOLIB_ERR_NONE)
        {
            // TODO: Check if buf is NULL terminated
            ESP_LOGI(RX_TAG, "Received: \"%s\"", buf);
        }
        else
        {
            ESP_LOGE(RX_TAG, "Failed to receive: %d", err);
        }
        esp_pm_lock_release(lock);
        utils::delay_ms(3000);
    }
}

static volatile bool packet_sent = false;
static void IRAM_ATTR packet_sent_cb(void) { packet_sent = true; }

void lora_tx_astrolavos_task(void* args)
{
    esp_pm_lock_handle_t lock;
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "lora_tx_lock", &lock);

    utils::delay_ms(10000);
    astrolavos::Astrolavos* astrolavos_app =
        static_cast<astrolavos::Astrolavos*>(args);
    LoRa* lora = astrolavos_app->getLoRa();
    ESP_LOGI(TX_TAG, "LoRa TX task started");
    // radio.setPacketSentAction(packet_sent_cb);

    while (true)
    {
        esp_pm_lock_acquire(lock);
        ESP_LOGI(TX_TAG, "Pinging");
        SX1262* radio = lora->getRadio();
        int16_t err = radio->transmit("ping");
        lora->putRadio();
        if (err == RADIOLIB_ERR_NONE)
        {
            ESP_LOGI(TX_TAG, "Pinged");
        }
        else
        {
            ESP_LOGE(TX_TAG, "Failed to ping: %d", err);
        }
        esp_pm_lock_release(lock);
        utils::delay_ms(1000);
    }
}
