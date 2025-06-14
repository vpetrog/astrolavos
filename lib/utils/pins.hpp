/**
 * @file pins.hpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief Heltec Wireless Tracker Pin Definitions
 * @date 2025-06-13
 *
 * @copyright Copyright (c) 2025
 *
 *
 * The Pin Wiring and definitions are sourced from
 * https://heltec.org/project/wireless-tracker/
 */

#pragma once
#include "driver/gpio.h"

namespace heltec
{

/* Button and reset */
constexpr gpio_num_t PIN_USER_SW = GPIO_NUM_0;
constexpr gpio_num_t PIN_RST =
    GPIO_NUM_NC; /* CHIP_PU is not usually controlled via GPIO */
constexpr gpio_num_t PIN_D_SEL = GPIO_NUM_48;
constexpr gpio_num_t PIN_BOOT_MODE = GPIO_NUM_47;

/* GNSS */
constexpr gpio_num_t PIN_GNSS_PPS = GPIO_NUM_36;
constexpr gpio_num_t PIN_GNSS_RST = GPIO_NUM_35;
constexpr gpio_num_t PIN_GNSS_TX = GPIO_NUM_33;
constexpr gpio_num_t PIN_GNSS_RX = GPIO_NUM_34;

/* Display */
constexpr gpio_num_t PIN_OLED_RST = GPIO_NUM_21;
constexpr gpio_num_t PIN_ST7735_CS = GPIO_NUM_38;
constexpr gpio_num_t PIN_ST7735_RST = GPIO_NUM_39;
constexpr gpio_num_t PIN_ST7735_DC = GPIO_NUM_40;
constexpr gpio_num_t PIN_ST7735_SCLK = GPIO_NUM_41;
constexpr gpio_num_t PIN_ST7735_MOSI = GPIO_NUM_42;
constexpr gpio_num_t PIN_ST7735_LED_K = GPIO_NUM_21;
constexpr gpio_num_t PIN_ST7735_VTFT_CTRL = GPIO_NUM_3;

/* UART */
constexpr gpio_num_t PIN_UART0_RX = GPIO_NUM_44;
constexpr gpio_num_t PIN_UART0_TX = GPIO_NUM_43;

/* USB */
constexpr gpio_num_t PIN_USB_D_MINUS = GPIO_NUM_19;
constexpr gpio_num_t PIN_USB_D_PLUS = GPIO_NUM_20;

/* LED Control */
constexpr gpio_num_t PIN_LED_WRITE_CTRL = GPIO_NUM_18;

/* Battery CTRL */
constexpr gpio_num_t PIN_VBAT_READ = GPIO_NUM_1;

/* ADC CTRL */
constexpr gpio_num_t PIN_ADC_CTRL = GPIO_NUM_2;

/* Crystal Settings */
constexpr gpio_num_t PIN_XTAL_32K_N = GPIO_NUM_16;
constexpr gpio_num_t PIN_XTAL_32K_P = GPIO_NUM_15;

/* LORA */
constexpr gpio_num_t PIN_LORA_DIO1 = GPIO_NUM_14;
constexpr gpio_num_t PIN_LORA_BUSY = GPIO_NUM_13;
constexpr gpio_num_t PIN_LORA_RST = GPIO_NUM_12;
constexpr gpio_num_t PIN_LORA_MISO = GPIO_NUM_11;
constexpr gpio_num_t PIN_LORA_MOSI = GPIO_NUM_10;
constexpr gpio_num_t PIN_LORA_SCK = GPIO_NUM_9;
constexpr gpio_num_t PIN_LORA_NSS = GPIO_NUM_8;

} /* namespace heltec */
