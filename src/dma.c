#include <msp430.h>
#include "dma.h"
#include "adc.h"
#include "defines.h"

//#define CONT_MODE

#ifdef USE_DMA

static int16_t g_samples[NUM_BUFFERS][NUM_SAMPLES];
static volatile uint16_t g_idx = 0;
static volatile uint16_t* g_flag;
static volatile uint8_t g_buf_idx = 0;

#define STEPS NUM_SAMPLES

void init_dma(volatile uint16_t* flag) {
	unsigned int i;
	
	for(i = 0; i < NUM_SAMPLES; ++i) {
		g_samples[0][i] = 0;
		g_samples[1][i] = 0;
	}

	g_flag = flag;
	*flag = 0;

#ifdef TX
	/* DMA0/ADC */
	DMA0CTL = 0;
	/* select memory conv. from adc12 as trigger for DMA0 */
	DMACTL0 = DMA0TSEL_6;
	/* set DMA src */
	DMA0SA = (unsigned int) &ADC12MEM0;
	/* set DMA dst */
	DMA0DA = (unsigned int) g_samples[0];
	/* number of data, this case words */
	DMA0SZ = STEPS;
	/* set repeated sing. transfer,inc dst,inc word wise,enable ints */
	DMA0CTL = DMADT_4 + DMADSTINCR_3 + DMAEN + DMAIE;
#else
	/* DMA1/DAC */
	DMA1CTL = 0;
#ifdef DAC_TB
	/* use TBCCR0 */
	DMACTL0 |= DMA1TSEL_8;
#else
	/* use TACCR0 */
	DMACTL0 |= DMA1TSEL_7;
#endif
	/* set DMA src arbitrary */
	DMA1SA = (unsigned int) g_samples[0];
	/* set DMA dst */
	DMA1DA = (unsigned int) &DAC12_0DAT;
	/* number of data, this case words */
	DMA1SZ = STEPS;
	/* set sing. transfer,inc src,inc word wise,enable ints */
	/* DMAEN needs to be here, won't work otherwise!! */
	DMA1CTL = DMASRCINCR_3 + DMAIE; // + DMAEN;

#endif
	debug_print("Initialized DMA Controller\n");
}

volatile int16_t* get_samples_dma(void) {
	/* always return sample buffer we are not currently using! */
	return (g_samples[(g_buf_idx+NUM_BUFFERS-1)%NUM_BUFFERS]);
}

volatile int dma_is_ready(void) {
	if(DMA1CTL & DMAEN)
		return 0;
	else
		return 1;
}

void set_dma_data(volatile int16_t *data, volatile uint16_t num_data) {
	/* if new data shall be written before current are written, then LED */
#ifndef CONT_MODE
	if(DMA1CTL & DMAEN) {
		LED_RED_ON();
		return;
	}
	DMA1CTL &= ~DMAEN;
	/* set DMA source */
	DMA1SA = (unsigned int) data;
	DMA1SZ = (unsigned int) num_data;
	/* enable DMA */
	DMA1CTL |= DMAEN;
	/* reset CCFIG!! */
	TACCTL0 &= ~0x01;
	//LED_BLUE_TOGGLE();
#else
	uint16_t i, k = (g_buf_idx+NUM_BUFFERS-2)%NUM_BUFFERS;
	for(i = 0; i < num_data; ++i) {
		g_samples[k][i] = data[i];
	}

#endif
}

#pragma vector=DACDMA_VECTOR
__interrupt void DACDMA_ISR (void)
{
	if(DMA0CTL & DMAIFG) {
		LED_BLUE_TOGGLE();
		/* DMA0/ADC */
		g_idx = (g_idx+STEPS)%NUM_SAMPLES;
		//set_dma_data(g_samples[g_buf_idx], NUM_SAMPLES);
		if(g_idx == 0) {
			g_buf_idx = (g_buf_idx+0x01)%NUM_BUFFERS;
			*g_flag = 1;
		}
		DMA0DA = (uint16_t) &g_samples[g_buf_idx][g_idx];

		/* clear DMA0 int flag*/
		DMA0CTL &= ~DMAIFG;
	}

	if(DMA1CTL & DMAIFG) {
		LED_BLUE_TOGGLE();
		/* DMA1/DAC */
#ifdef CONT_MODE
		g_buf_idx = (g_buf_idx+1)%NUM_BUFFERS;
		DMA1SA = g_samples[g_buf_idx];
		DMA1CTL |= DMAEN;
		TACCTL0 &= ~0x01;
#endif

		DMA1CTL &= ~DMAIFG;
	}

	if(DAC12_0CTL & DAC12IFG) {
		DAC12_0CTL &= ~DAC12IFG;
	}
}

#endif
