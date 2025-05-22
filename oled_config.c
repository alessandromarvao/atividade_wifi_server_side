#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "oled_config.h"

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

bool is_i2c_running;

extern uint8_t ssd[];

const uint8_t big_char_degree[64] = {
    0x0E,0x00, 0x11,0x00, 0x11,0x00, 0x0E,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00
  };

/**
 * Inicializa a conexão i2C com o display.
 */
void i2c_oled_init()
{
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();
}

void ssd1306_clear_display(uint8_t *ssd) {
    memset(ssd, 0, ssd1306_buffer_length);
    struct render_area area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&area);
    render_on_display(ssd, &area);
    sleep_ms(10);
}

/**
 * Imprime os valores passados como parâmetro no display OLED
 */
void display_temperature(float temp, char *ip)
{
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    char temp_formatted[30];
    int vert_alignment;

    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];

    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    ssd1306_clear_display(ssd); 

    // Formatação da string com o valor do sensor
    // Ex.: TEMP: 25.3 C
    snprintf(temp_formatted, sizeof(temp_formatted), "%.1f C", temp);

    // Imprime a mensagem no display OLED
    ssd1306_draw_string(ssd, 28, 0, ip);
    ssd1306_draw_string(ssd, 22, 24, "Temperatura");
    ssd1306_draw_string(ssd, 50, 43, temp_formatted);

    render_on_display(ssd, &frame_area);
}
