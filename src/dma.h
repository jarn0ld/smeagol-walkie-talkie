#ifndef _DMA_H_
#define _DMA_H_

#include <stdint.h>

extern void init_dma(volatile uint16_t* flag);
extern volatile int16_t* get_samples_dma(void);
extern void set_dma_data(volatile int16_t *data, volatile uint16_t num_data);
extern volatile int dma_is_ready(void);

#endif
