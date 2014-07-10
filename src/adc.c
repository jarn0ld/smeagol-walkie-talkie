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
#include "adc.h"
#include "defines.h"

#ifdef TX
/* Ping-Pong bffering needed! */
#ifndef USE_DMA
static int16_t g_samples[NUM_BUFFERS][NUM_SAMPLES];
static volatile uint16_t* g_flag;
static volatile uint16_t g_sample_idx = 0;
static volatile uint8_t g_buf_idx = 0;
#endif

void init_adc(volatile uint16_t* flag) {
#ifndef USE_DMA
	unsigned int i;
	for(i = 0; i < NUM_SAMPLES; ++i) {
		g_samples[0][i] = 0;
		g_samples[1][i] = 0;
	}
#endif
	
	/* Select alternative function as A0 input */
	P6SEL |= 0x01;
	/* ADC12 ON, REF ON */
	ADC12CTL0 = ADC12ON+SHT0_1+REF2_5V+REFON;
	/* Use Timer as SAMPCOM,reap. sin.g channel,SMCLK for sampl.,Triggered by TA1 */
	ADC12CTL1 = SHP+CONSEQ_2+SHS_1+ADC12SSEL_3;
	/* A0 as input and VR+=VCC VR-=GND */
	ADC12MCTL0 = INCH_0;
	
#ifndef USE_DMA
	/* Enable Interrupt for MEM0/ADC12IFG.0 */
	ADC12IE = 0x0001;
#else
	/* clear interrupt flags */
	ADC12IFG = 0x00;
	/* Disable ADC12IFG.0 */
	ADC12IE = 0x0000;
#endif
	/* Enable Conversion */
	ADC12CTL0 |= ENC;


#ifndef USE_DMA
	*flag = 0;
	g_flag = flag;
#endif

	/* some magic to let ref settle */
	//TACCR0 = 1500;                            // Delay to allow Ref to settle
	//TACCTL0 |= CCIE;                          // Compare-mode interrupt.
	//TACCTL1 |= CCIE;                          // Compare-mode interrupt.
	//TACTL = TASSEL_1 | MC_1 | TAIE;                  // TACLK = ACLK, Up mode.
	//_BIS_SR(LPM3_bits + GIE);                 // Wait for delay, Enable interrupts
	//TACCTL0 &= ~CCIE;                         // Disable timer

	/* timer setup */
	P2SEL |= BIT3;                            // Set pin for Timer A1
	P2DIR |= 0x08;				  // Set TA1 as output
	/* convert on set on r1 and reset on r0 */
	TACCR0 = SAMPLING_RATE;                   // Init TACCR0 w/ sample prd=CCR0+1
	TACCR1 = SAMPLING_RATE-20;                // Trig for ADC12 sample & convert
	TACCTL1 = OUTMOD_3;                       // Set/reset
	TACTL = TACLR;
	TACTL |=  MC_1 | TASSEL_2;// | TAIE;          // SMCLK, clear TAR, up mode
	//TACCTL0 |= CCIE;
	
	debug_print("Initialized ADC\n");
}

/* called once for ref setteling setup */
#pragma vector=TIMERA0_VECTOR
__interrupt void ta0_isr(void) {
	static uint16_t i = 0;
	if(i == NUM_SAMPLES) {
		i = 0;
	}
	++i;
}

#ifndef USE_DMA
volatile int16_t* get_samples(void) {
	/* always return sample buffer we are not currently using! */
	return (g_samples[(g_buf_idx+NUM_BUFFERS-1)%NUM_BUFFERS]);
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR (void) {
	g_samples[g_buf_idx][g_sample_idx++] = ADC12MEM0;

	if (g_sample_idx == NUM_SAMPLES)
	{
		/* stop conversion first */
		ADC12CTL0 &= ~ENC;
		g_sample_idx = 0;
		g_buf_idx = (g_buf_idx+1)%NUM_BUFFERS;
		*g_flag = 1;
		ADC12CTL0 |= ENC;
	}
}
#endif
#endif
