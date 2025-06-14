#include <HT_st7735.hpp>
#include <lora.hpp>
#include <utils.hpp>

void lora_task(void* args)
{
    HT_st7735* display = reinterpret_cast<HT_st7735*>(args);

    while (true) {
        display->fill_rectangle(0, 0, 180, 28,
                                    ST7735_BLACK);
        display->write_str(0, 0, "Broadcasting LoRa", Font_7x10,
                        ST7735_WHITE,
                        ST7735_BLACK);
        utils::delay_ms(1000);
    }
}
