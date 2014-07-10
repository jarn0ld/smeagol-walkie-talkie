/**
 * Digital Audio Transmission v1.0
 * Copyright (c) 2014 live141

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <msp430.h>
#include <stdint.h>
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "rf.h"
#include "defines.h"

volatile uint16_t g_sample_flag = 0;

#ifndef DAC_TB
void delay_us(uint16_t us) {
	uint16_t tresh = us*CPU_FREQ_MHZ;
	/* SMCLK, clear TBR, cont mode, enable ints */
	TBCTL = TBCLR;
	TBCTL = MC_2 | TBSSEL_2;
	
	while(TBR <= tresh);
}
#else
void delay_us(uint16_t us) {
	//uint16_t tresh = us*CPU_FREQ_MHZ;
	/* SMCLK, clear TBR, cont mode, enable ints */
	//TBCTL = TBCLR;
	//TBCTL = MC_2 | TBSSEL_2;
	
	//while(TBR <= tresh);
}
#endif

void check_for_clock(void) {
	volatile unsigned int i;
	do
	{
		IFG1 &= ~OFIFG;                           // Clear OSCFault flag
		for (i = 0xFF; i > 0; i--);               // Time for flag to set
	}
	while ((IFG1 & OFIFG));                   // OSCFault flag still set?
}

int main(void) {
	volatile int16_t* samples;
	unsigned int i;
	DISABLE_GLOBAL_INT();
	/* stop watchdog timer */
	WDTCTL = WDTPW +WDTHOLD;
	/* SET CPU to 5MHz */
	/* max DCO
	   MCLK = DCOCLK
	   SMCLK = DCOCLK
	   ACLK = 8KHz
	*/
	DCOCTL = DCO0 + DCO1 + DCO2;
	BCSCTL1 = RSEL0 + RSEL1 + RSEL2 + XT2OFF;
	BCSCTL2 = 0x00;

	delay_us(10000);

	/* activate Active Mode */
	__bic_SR_register(LPM4_bits);

	/* set LEDs when loaded */
	P5SEL = 0x00;
	P5DIR = 0x70;
	LED_RED_ON();
	LED_GREEN_OFF();
	LED_BLUE_OFF();

	check_for_clock();
	init_usb_serial();
#ifdef USE_DMA
	init_dma(&g_sample_flag);
#endif

#ifdef TX
	init_adc(&g_sample_flag);
#else
	init_dac();
#endif
	init_rf(RF_CHANNEL, PAN_ID, NODE_ADDR, &g_sample_flag);

	debug_print("Successfully booted.\n");
	/* set LEDS to signalize finished initilizing */
	LED_RED_OFF();
	ENABLE_GLOBAL_INT();

#ifdef TX
	/* TX */
	while(1) {
		if(g_sample_flag == 1) {
			g_sample_flag = 0;
#ifdef USE_DMA
			/* get samples */
			samples = get_samples_dma();
#else
			/* get samples */
			samples = get_samples();
#endif
			/* send oder radio, 2*num_words */
			send_rf_data(RF_RX_ADDR, (uint8_t*) samples, NUM_SAMPLES*2);
		}
		/* reset WDT */
		WDTCTL = WDTPW + WDTCNTCL;

	}
#else
	/* RX */
	while(1) {
		if(g_sample_flag == 1) {
			g_sample_flag = 0;
			samples = get_samples_rf();
#if 0
			uint8_t err = 0;
			for(i = 0; i < NUM_SAMPLES; ++i) {
				//samples[i] = 4095-7*i;
				usb_printf("%d\n", samples[i]);
				//if( ((uint16_t) samples[i]) > 4095) {
				//	usb_printf("i=%u\n", i);
				//	++err;
				//}
			}
			usb_printf("#error: %u\n", err);
			usb_printf("\n\n");
#endif			
			set_dma_data(samples, NUM_SAMPLES);
		}
		/* reset WDT */
		WDTCTL = WDTPW + WDTCNTCL;
	}
#endif
	return 0;
}
