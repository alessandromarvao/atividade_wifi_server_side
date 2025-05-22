#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "temp_sensor.h"

void adc_temp_init(){
    adc_init();
    adc_set_temp_sensor_enabled(true);
}

int adc_temperature_read() {
    adc_select_input(4);

    uint16_t raw_value = adc_read();

    // Conversão para tensão
    const float conv = 3.3f / (1 << 12); 

    float voltage = raw_value * conv;

    // O valor de cálculo para conversão é de 27.0f
    float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;
    printf("Voltagem Recebida: %f - Temperatura: %.2f\n", voltage, temperature);

    int temp = (int)roundf(temperature);

    return temp;
}