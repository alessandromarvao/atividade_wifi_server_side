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
    char ip[16];

    stdio_init_all();

    i2c_oled_init();

    adc_temp_init();

    dma_init();

    if (wifi_connect()) {
        uint8_t *ip_address = get_my_ip();
        
        snprintf(ip, 16, "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        printf("IP: %s\n", ip);
    }

    while (true) {
        display_temperature(get_avg_temp(&cfg_temp, DMA_TEMP_CHANNEL), get_my_ip());
    }
}
