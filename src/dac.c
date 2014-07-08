#include <msp430.h>
#include "dac.h"
#include "adc.h"

#ifdef RX

static volatile int16_t *g_data = 0;
static volatile uint16_t g_num = 0;

void init_dac(void) {
	/* init DAC */
	/* use P6.6/DAC0 for DAC */
	P6SEL |= 0x40;
	P6DIR |= 0x40;
	P6SEL |= 0x01;

	/* ADC12 ON, REF ON, 1.5V ref*/
	ADC12CTL0 = ADC12ON+SHT0_1+/*REF2_5V+*/REFON;

	/* 12bit, DAC12 on and hihg speed, 1x ref V */
	DAC12_0CTL = /*DAC12RES*/ + DAC12AMP_7 + DAC12IR;

#ifdef DAC_TB
	/* init Timer */
	TBCCR0 = SAMPLING_RATE;
	/* SMCLK, clear TBR, upmode, enable ints */
	TBCTL = TBCLR;
	TBCTL = MC_1 | TBSSEL_2;
#ifndef USE_DMA
	TBCCTL0 |= CCIE;                           // Compare-mode interrupt.
#else
	TBCCTL0 = 0;
#endif
#else
	/* init Timer */
	TACCR0 = SAMPLING_RATE;
	/* SMCLK, clear TBR, upmode, enable ints */
	TACTL = TACLR;
	TACTL = MC_1 | TASSEL_2;
#ifndef USE_DMA
	TACCTL0 |= CCIE;                           // Compare-mode interrupt.
#else
	TACCTL0 = 0;
#endif
#endif
}

#ifndef USE_DMA
void set_dac_data(volatile int16_t *data, uint16_t num) {
	g_data = data;
	g_num = (volatile uint16_t) num;
}

#ifdef DAC_TB
#pragma vector=TIMERB0_VECTOR
__interrupt void tb0_isr(void) {
	/* check if we have data to sent */
	if( g_num <= 0 || g_data == 0 )
		return;
	
	/* output data to DAC0/P6.6 */
	DAC12_0DAT = *g_data;

	--g_num;
	++g_data;
}
#else
#pragma vector=TIMERA0_VECTOR
__interrupt void ta0_isr(void) {
	/* check if we have data to sent */
	if( g_num <= 0 || g_data == 0 )
		return;
	/* output data to DAC0/P6.6 */
	DAC12_0DAT = *g_data;

	--g_num;
	++g_data;
}
#endif
#endif
#endif
