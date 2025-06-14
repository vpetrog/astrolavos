/**
 * @file utils.hpp
 * @author Evangelos Petrongonas (vpetrog@ieee.org)
 * @brief Various common utilities that are used throughout the project
 * @version 0.1
 * @date 2025-06-13
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "freertos/FreeRTOS.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

namespace utils
{
/* -----------------------------  Helper macros ----------------------------- */
static inline void delay_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
static inline void delay_us(uint32_t us) { vTaskDelay(max(pdMS_TO_TICKS(us) / 1000, 1)); }
}
