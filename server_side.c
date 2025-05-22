#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "dma_config.h"
#include "oled_config.h"
#include "temp_sensor.h"
#include "wifi_config.h"

#define DMA_TEMP_CHANNEL 0

int main()
{
    char *ip = NULL;

    stdio_init_all();

    i2c_oled_init();

    adc_temp_init();

    dma_init();

    if (wifi_connect()) {
        printf("Conectado ao Wi-Fi\n");
        ip = get_my_ip();
    } else {
        printf("Falha na conexão Wi-Fi\n");
    }

    while (true) {
        display_temperature(get_avg_temp(&cfg_temp, DMA_TEMP_CHANNEL), ip);
    }
}
