#ifndef DMA_CONFIG_H
#define DMA_CONFIG_H

#include "hardware/dma.h"

// === Configuração global do canal DMA 0 ===
extern dma_channel_config cfg_temp;

#define DMA_TEMP_CHANNEL 0

void dma_init();
float get_avg_temp(dma_channel_config* cfg_temp, int dma_chan);

#endif