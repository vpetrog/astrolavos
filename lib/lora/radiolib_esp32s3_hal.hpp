// Copied from
// https://github.com/IanBurwell/DynamicLRS/blob/c26f7f8dcca0c1b70af0aa6aee3aba3a1652aba6/components/DLRS_LoRadio/radiolib_esp32s3_hal.hpp

#pragma once

#include <RadioLib.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/gpio_hal.h"
#include "soc/dport_reg.h"
#include "soc/rtc.h"

#define NOP() asm volatile("nop")

/*
 * Generic isr handler that takes a `void func(void)` callback as an argument
 * and calls it. This is needed as ESP-IDF expects isr callbacks to accept a
 * `void*` argument (a context), but radiolib expects the isr to have no
 * arguments.
 */
static void IRAM_ATTR isr_esp_to_radiolib(void* radiolib_func)
{
    auto callback = reinterpret_cast<void (*)()>(radiolib_func);
    if (callback)
    {
        callback();
    }
}

class EspHal : public RadioLibHal
{
public:
    inline static const char* LOG_TAG = "radiolib_hal";

    // Pass RadioLib info about what values to use for
    // input/output/low/high/rising_edge/falling_edge Note: This indirectly sets
    // the default output to push/pull (as opposed to GPIO_MODE_INPUT_OUTPUT)
    EspHal(int8_t sck, int8_t miso, int8_t mosi,
           spi_host_device_t host = SPI2_HOST)
        : RadioLibHal(GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, 0, 1,
                      GPIO_INTR_POSEDGE, GPIO_INTR_NEGEDGE),
          spiSCK(sck), spiMISO(miso), spiMOSI(mosi), host(host)
    {
        ESP_LOGI("EspHal", "Constructing");
    }

    void init() override
    {
        ESP_LOGI("EspHal", "init");
        // we only need to init the SPI here
        spiBegin();
    }

    void term() override
    {
        ESP_LOGI("EspHal", "term");
        // we only need to stop the SPI here
        spiEnd();
    }

    void pinMode(uint32_t pin, uint32_t mode) override
    {
        ESP_LOGI("EspHal", "pinMode");
        if (pin == RADIOLIB_NC)
        {
            return;
        }

        gpio_hal_context_t gpiohal;
        gpiohal.dev = GPIO_LL_GET_HW(GPIO_PORT_0);

        gpio_config_t conf = {
            .pin_bit_mask = (1ULL << pin),
            .mode = (gpio_mode_t)mode,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = (gpio_int_type_t)gpiohal.dev->pin[pin].int_type,
        };
        gpio_config(&conf);
    }

    void digitalWrite(uint32_t pin, uint32_t value) override
    {
        // ESP_LOGI("EspHal", "digitalWrite %lu %lu", pin, value);
        if (pin == RADIOLIB_NC)
        {
            return;
        }

        gpio_hold_en((gpio_num_t)pin);
        gpio_set_level((gpio_num_t)pin, value);
        gpio_hold_dis((gpio_num_t)pin);
    }

    uint32_t digitalRead(uint32_t pin) override
    {
        // ESP_LOGI("EspHal", "digitalRead %lu", pin);
        if (pin == RADIOLIB_NC)
        {
            return (0);
        }

        return (gpio_get_level((gpio_num_t)pin));
    }

    void attachInterrupt(uint32_t interruptNum, void (*interruptCb)(void),
                         uint32_t mode) override
    {
        ESP_LOGI("EspHal", "attachInterrupt %lu %lu", interruptNum, mode);
        if (interruptNum == RADIOLIB_NC)
        {
            return;
        }

        // Only install the isr service once
        if (!isr_initialized)
        {
            // ESP_ERROR_CHECK(gpio_install_isr_service((int)0));
            isr_initialized = true;
        }

        ESP_ERROR_CHECK(
            gpio_set_intr_type(static_cast<gpio_num_t>(interruptNum),
                               (gpio_int_type_t)(mode & 0x7)));

        // Use the wrapper function and pass the callback as argument
        ESP_ERROR_CHECK(gpio_isr_handler_add(
            static_cast<gpio_num_t>(interruptNum), isr_esp_to_radiolib,
            reinterpret_cast<void*>(interruptCb)));
    }

    void detachInterrupt(uint32_t interruptNum) override
    {
        ESP_LOGI("EspHal", "detachInterrupt %lu", interruptNum);
        if (interruptNum == RADIOLIB_NC)
        {
            return;
        }
        gpio_isr_handler_remove((gpio_num_t)interruptNum);
        gpio_wakeup_disable((gpio_num_t)interruptNum);
        gpio_set_intr_type((gpio_num_t)interruptNum, GPIO_INTR_DISABLE);
    }

    void delay(unsigned long ms) override
    {
        // ESP_LOGI("EspHal", "delay %lu", ms);
        vTaskDelay(ms / portTICK_PERIOD_MS);
    }

    void delayMicroseconds(unsigned long us) override
    {
        // ESP_LOGI("EspHal", "delayMicroseconds %lu", us);
        uint64_t m = (uint64_t)esp_timer_get_time();
        if (us)
        {
            uint64_t e = (m + us);
            if (m > e)
            { // overflow
                while ((uint64_t)esp_timer_get_time() > e)
                {
                    NOP();
                }
            }
            while ((uint64_t)esp_timer_get_time() < e)
            {
                NOP();
            }
        }
    }

    unsigned long millis() override
    {
        // ESP_LOGI("EspHal", "millis");
        return ((unsigned long)(esp_timer_get_time() / 1000ULL));
    }

    unsigned long micros() override
    {
        // ESP_LOGI("EspHal", "micros");
        return ((unsigned long)(esp_timer_get_time()));
    }

    long pulseIn(uint32_t pin, uint32_t state, unsigned long timeout) override
    {
        // ESP_LOGI("EspHal", "pulseIn %lu %lu %lu", pin, state, timeout);
        if (pin == RADIOLIB_NC)
        {
            return (0);
        }

        this->pinMode(pin, GPIO_MODE_INPUT);
        uint32_t start = this->micros();
        uint32_t curtick = this->micros();

        while (this->digitalRead(pin) == state)
        {
            if ((this->micros() - curtick) > timeout)
            {
                return (0);
            }
        }

        return (this->micros() - start);
    }

    void spiBegin()
    {
        // ESP_LOGI("EspHal", "spiBegin");
        // Skip if SPI has already been initialized
        if (spi_initialized)
        {
            return;
        }

        ESP_LOGI(LOG_TAG, "Initializing bus SPI%d...", this->host + 1);
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = this->spiMOSI;
        buscfg.miso_io_num = this->spiMISO;
        buscfg.sclk_io_num = this->spiSCK;
        buscfg.quadwp_io_num = -1;
        buscfg.quadhd_io_num = -1;
        buscfg.max_transfer_sz = 0;

        spi_device_interface_config_t spi_dev = {};
        spi_dev.mode = 0;
        spi_dev.clock_speed_hz = 2 * 1000 * 1000;
        spi_dev.spics_io_num = -1;
        spi_dev.queue_size = 1;

        // Initialize the SPI bus
        ESP_ERROR_CHECK(
            spi_bus_initialize(this->host, &buscfg, SPI_DMA_CH_AUTO));
        // Attach the device to the SPI bus
        ESP_ERROR_CHECK(spi_bus_add_device(this->host, &spi_dev, &spi));
        spi_initialized = true;
    }

    void spiBeginTransaction() { /*ESP_LOGI("EspHal", "spiBeginTransaction"); */}

    void spiTransfer(uint8_t* out, size_t len, uint8_t* in)
    {
        // ESP_LOGI("EspHal", "spiTransfer %p %u %p", out, len, in);
        if (len == 0)
        {
            return;
        }
        spi_transaction_t trans = {};
        trans.length = 8 * len;
        trans.tx_buffer = out;
        trans.rx_buffer = in;

        // Transmit the transaction
        ESP_ERROR_CHECK(spi_device_transmit(spi, &trans));
    }

    void spiEndTransaction()
    {
        //ESP_LOGI("EspHal", "spiEndTransaction");
        // nothing needs to be done here
    }

    void spiEnd()
    {
        // ESP_LOGI("EspHal", "spiEnd");
        spi_bus_remove_device(this->spi);
        spi_bus_free(this->host);
        spi_initialized = false;
    }

private:
    int8_t spiSCK;
    int8_t spiMISO;
    int8_t spiMOSI;
    spi_host_device_t host;
    spi_device_handle_t spi;
    bool spi_initialized = false;
    bool isr_initialized = false;
};
