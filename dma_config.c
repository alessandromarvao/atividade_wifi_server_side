#include <stdio.h>
#include <stdbool.h>
#include "dma_config.h"
#include "hardware/adc.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

dma_channel_config cfg_temp;

// Quantidade de amostras de temperatura coletadas durante 0,5 segundos
#define AMOSTRAS 4000
// 0,5 segundos
#define DURACAO_AMOSTRAGEM_US 500000

static uint16_t buffer_temp[AMOSTRAS];
volatile bool dma_temp_done = false;

/**
 * @brief Handler de interrupção do canal DMA 0.
 *
 * Esta função é chamada automaticamente quando o canal 0 do DMA
 * completa a transferência das amostras do ADC para o buffer.
 * Ela limpa a flag da interrupção e ativa o sinalizador
 * 'dma_temp_done' para que o laço principal saiba que pode
 * prosseguir.
 */
void dma_handler_temp()
{
    dma_hw->ints0 = 1u << 0; // Limpa a interrupção do canal 0
    dma_temp_done = true;    // Sinaliza conclusão para o executor
}

/**
 * @brief Converte valor do ADC para temperatura em °C.
 *
 * @param raw Valor bruto de 12 bits lido do ADC.
 * @return float Temperatura em graus Celsius.
 */
static float convert_to_celsius(uint16_t raw)
{
    const float conv = 3.3f / (1 << 12); // Conversão para tensão
    float voltage = raw * conv;
    return 27.0f - (voltage - 0.706f) / 0.001721f;
}

void dma_init()
{
    // Configura o canal DMA 0 para transferir dados do ADC
    cfg_temp = dma_channel_get_default_config(DMA_TEMP_CHANNEL);
    channel_config_set_transfer_data_size(&cfg_temp, DMA_SIZE_16); // 16 bits
    channel_config_set_read_increment(&cfg_temp, false);           // ADC FIFO fixo
    channel_config_set_write_increment(&cfg_temp, true);           // Buffer se move
    channel_config_set_dreq(&cfg_temp, DREQ_ADC);                  // dispara com ADC

    dma_channel_set_irq0_enabled(DMA_TEMP_CHANNEL, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler_temp);
    irq_set_enabled(DMA_IRQ_0, true);
}

/**
 * Inicia uma transferência via DMA de um bloco do sensor de temperatura.
 * @param buffer Buffer de destino.
 * @param cfg Configuração do canal DMA.
 * @param dma_chan Canal DMA utilizado.
 */
void start_dma_stack(uint16_t *buffer, dma_channel_config *cfg, int dma_chan)
{
    // Canal 4 → sensor interno
    adc_select_input(4);
    adc_fifo_drain();
    adc_run(false);
    adc_fifo_setup(true, true, 1, false, false);
    adc_run(true);

    dma_channel_configure(
        dma_chan,
        cfg,
        buffer,
        &adc_hw->fifo,
        AMOSTRAS,
        true);
}

/**
 * @brief Coleta de temperatura a cada 0,5s.
 *
 * @param cfg_temp Configuração do canal DMA.
 * @param dma_chan Número do canal DMA utilizado.
 * @return float Temperatura média calculada ao final do intervalo.
 */
float get_avg_temp(dma_channel_config *cfg_temp, int dma_chan)
{
    float soma = 0.0f;
    uint32_t total_amostras = 0;

    absolute_time_t inicio = get_absolute_time();

    while (absolute_time_diff_us(inicio, get_absolute_time()) < DURACAO_AMOSTRAGEM_US)
    {
        dma_temp_done = false;
        start_dma_stack(buffer_temp, cfg_temp, dma_chan);
        while (!dma_temp_done)
            __wfi();    // Aguarda fim do DMA
        adc_run(false); // Desliga o ADC

        for (int i = 0; i < AMOSTRAS; i++)
        {
            soma += convert_to_celsius(buffer_temp[i]);
        }

        total_amostras += AMOSTRAS;
    }

    return soma / total_amostras;
}