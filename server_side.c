#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "dma_config.h"
#include "oled_config.h"
#include "temp_sensor.h"

#define DMA_TEMP_CHANNEL 0

int main()
{
    stdio_init_all();

    i2c_oled_init();

    adc_temp_init();

    dma_init();

    while (true) {
        display_temperature(get_avg_temp(&cfg_temp, DMA_TEMP_CHANNEL));
    }
}
