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
#include "rf.h"
#include "defines.h"
#include <stdint.h>
#include "adc.h"

/* use UART0/SPI0 */

#define CC2420_OFF() (P4OUT &= ~0x40)
#define CC2420_ON() (P4OUT |= 0x40)
#define CC2420_ENABLE() (P4OUT &= ~0x04)
#define CC2420_DISABLE() (P4OUT |= 0x04)
#define CC2420_VREG_ON() (P4OUT |= 0x20)
#define CC2420_VREG_OFF() (P4OUT &= ~0x20)

#define PROTOCOL_OVERHEAD 5
#define PAYLOAD_SIZE 112

#define BUFFER_DELAY 2

static volatile uint16_t g_data[NUM_BUFFERS][NUM_SAMPLES];
static volatile uint16_t g_idx = 0;
static volatile uint16_t *g_flag = 0;
static volatile uint8_t g_buf_idx = 0, g_buf_usr_idx = 0;

uint16_t read_register(uint8_t addr);
void write_register(uint8_t addr, uint16_t payload);
uint8_t command_strobe(uint8_t addr);
void write_ram(uint16_t addr, uint8_t *payload, uint16_t size);
void read_ram(uint16_t addr, uint8_t *payload, uint16_t size);
void set_channel(uint8_t channel);
void write_tx_buffer(uint8_t *data, uint8_t size);
void read_rx_buffer(uint8_t *data, uint8_t size);

volatile int16_t* get_samples_rf(void) {
	/* always return inactive buffer */
	//return (volatile int16_t*) (g_data[g_buf_idx^0x01]);
	//return (volatile int16_t*) (g_data[(g_buf_idx+NUM_BUFFERS-1)%NUM_BUFFERS]);
	volatile int16_t *data =  (volatile int16_t*) (g_data[(g_buf_usr_idx)]);
	g_buf_usr_idx = (g_buf_usr_idx+1)%NUM_BUFFERS;
	return data;
}

void send_rf_data(uint16_t addr, uint8_t *data, uint16_t size) {
	/* send only samples for now -> no protocol overhead */
	uint16_t i, dat;
	uint8_t len;
	static uint8_t sequence = 0;
	DISABLE_GLOBAL_INT();
	for(i = size; i > 0; i -= len, data += len) {
		len = (i > PAYLOAD_SIZE)?PAYLOAD_SIZE:i;
		/* wait for TX to become idle */
#ifndef DEBUG
		while(FIFOP_IS_1() || SFD_IS_1());
#else
		while(FIFOP_IS_1());
#endif
		//while(command_strobe(CC2420_SNOP) & 0x08);

		/* flush TX Buffer */
		command_strobe(CC2420_SFLUSHTX);

		LED_GREEN_TOGGLE();

		dat = (len+PROTOCOL_OVERHEAD) & 0x007f;
		write_tx_buffer((uint8_t*) &dat, 1);
		dat = 0x0001;
		write_tx_buffer((uint8_t*) &dat, 2);
		dat = sequence++;
		write_tx_buffer((uint8_t*) &dat, 1);
		
		/* copy data to TX Buffer */
		write_tx_buffer(data, len);

		/* send data */
		//command_strobe(CC2420_STXONCCA);

		/* fixed tx rate, use TimerB */
		dat = TBR;

		//while(!CCA_IS_1());
			command_strobe(CC2420_STXON);
		//command_strobe(CC2420_STXONCCA);
		
		/* reset WDT */
		WDTCTL = WDTPW + WDTCNTCL;

		/* wait for TX to become active */
#ifndef DEBUG
		while(!SFD_IS_1());
#endif
		//while(!(command_strobe(CC2420_SNOP) & 0x08));
		//while(TBR-dat < CPU_FREQ_MHZ*3000);
		//delay_us(6000);
	}

	ENABLE_GLOBAL_INT();
}

void init_rf(uint8_t channel, uint16_t pan_id, uint16_t addr, volatile uint16_t *flag) {
	for(g_idx = 0; g_idx < NUM_SAMPLES; ++g_idx) {
		g_data[0][g_idx] = 0x00;
		g_data[1][g_idx] = 0x00;
	}
	g_idx = 0;
	g_buf_idx = 0;
	g_flag = flag;
	*g_flag = 0;

	/* set CS as output, high */
	P4DIR |= 0x04;
	P4OUT |= 0x04;

	/* P1.0 is PKT_INT/FIFOP */
	P1DIR &= ~0x01;
	/* P1.3 is FIFO, P1.4 CCA */
	//P1DIR &= ~0x80;
	P1DIR &= ~0x18;
	/* P4.1 is SFD Input */
	P4DIR &= ~0x02;

	/* set cc2420 reset,vref to output */
	P4DIR |= 0x60;
	/* set to use SPI */
	P3SEL |= 0x0E;
	P3DIR |= 0x0A;
	/* Esnable SPI0 */
	/* software reset */
	U0CTL = SWRST;
	ME1 |= USPIE0;
	/* Enable RX Interrupt */
	//IE1 |= URXIE0;
	/* 8-bit character, SPI, Master */
	U0CTL |= CHAR+SYNC+MM;
	/* UCLK = SMCL, 4-wire mode */
	U0TCTL |= SSEL1 + STC + CKPH;
	/* baudrate = 4.5MHz/2 */
	U0BR0 = 0x02;
	U0BR1 = 0x00;
	/* no modulation */
	U0MCTL = 0x00;
	U0CTL &= ~SWRST;
	IFG1 &= ~URXIFG0;

	/* cc2420 off, turn vref for cc2420 on */
	CC2420_OFF();
	asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
	CC2420_VREG_ON();
	/* reset cc2420 */
	CC2420_DISABLE();
	delay_us(1000);
	CC2420_OFF();
	delay_us(1000);
	CC2420_ON();
	delay_us(1000);
	/* turn osc on */
	command_strobe(CC2420_SXOSCON);
	delay_us(1000);

	/* Configure Radio */

	/* Turn on automatic packet acknowledgment */
	//write_register(CC2420_MDMCTRL0, 0x0AF2);
	write_register(CC2420_MDMCTRL0, 0x02C2);
	debug_printf("0x%x\n", read_register(CC2420_MDMCTRL0));
	/* Set the correlation threshold = 20 */
	write_register(CC2420_MDMCTRL1, 0x0500);
	debug_printf("0x%x\n",read_register(CC2420_MDMCTRL1));
	/* Set the FIFOP threshold to 117 */
	write_register(CC2420_IOCFG0, 0x0075);
	//write_register(CC2420_IOCFG0, 0x0020);
	debug_printf("0x%x\n",read_register(CC2420_IOCFG0));
	/* Turn off "Security enable" */
	write_register(CC2420_SECCTRL0, 0x01C4);
	debug_printf("0x%x\n",read_register(CC2420_SECCTRL0));
	/* max output power */
	write_register(CC2420_TXCTRL, 0xA0FF);
	/* wait for osc */
	while(!(command_strobe(/*CC2420_SNOP*/ CC2420_SXOSCON) & 0x40));
	/* set channel */
	set_channel(channel);
	/* configure network id and node address */
	write_ram(CC2420RAM_SHORTADDR, (uint8_t*) &addr, 2);
	write_ram(CC2420RAM_PANID, (uint8_t*) &pan_id, 2);
	command_strobe(CC2420_STXCAL);
	command_strobe(CC2420_SRXON);
	/* Enable FIFOP Interrupt last */
	ENABLE_FIFOP_INT();

	debug_print("Initliazied Radio Interface\n");
}

void write_ram(uint16_t addr, uint8_t *payload, uint16_t size) {
	/* BANK|R/W|00000 */
	/* RAMBIT|7-Bit addr */
	uint16_t i;
	uint8_t data0 = ((addr & 0x18) << 3);
	uint8_t data1 = 0x80 | (addr & 0x7f);
	CC2420_ENABLE();
	TXBUF0 = data1;
	while ((U0TCTL & TXEPT) == 0);
	while(!(IFG1 & URXIFG0));
	data1 = RXBUF0;
	TXBUF0 = data0;
	while ((U0TCTL & TXEPT) == 0);
	
	for(i = 0; i  < size; ++i) {
		TXBUF0 = payload[i];
		while ((U0TCTL & TXEPT) == 0);
	}
	CC2420_DISABLE();
}

void read_ram(uint16_t  addr, uint8_t *payload, uint16_t size) {
	/* BANK|R/W|00000 */
	/* RAMBIT|7-Bit addr */
	uint16_t i;
	uint8_t data0 = ((addr & 0x18) << 3) | 0x20;
	uint8_t data1 = 0x80 | (addr & 0x7f);
	CC2420_ENABLE();
	TXBUF0 = data1;
	while ((U0TCTL & TXEPT) == 0);
	while(!(IFG1 & URXIFG0));
	data1 = RXBUF0;
	TXBUF0 = data0;
	while ((U0TCTL & TXEPT) == 0);
	
	for(i = 0; i < size; ++i) {
		TXBUF0 = 0xfe;
		while ((U0TCTL & TXEPT) == 0);
		while(!(IFG1 & URXIFG0));
		payload[i] = RXBUF0;
	}

	CC2420_DISABLE();
}

uint16_t read_register(uint8_t addr) {
	/* RAMBIT|R/W|6-Bit addr */
	uint8_t status = 0x40 | (addr & 0x3f);
	uint16_t data = 0;

	CC2420_ENABLE();
	TXBUF0 = status;
	while ((U0TCTL & TXEPT) == 0);
	while(!(IFG1 & URXIFG0));
	status = RXBUF0;

	TXBUF0 = 0xfe;
	while ((U0TCTL & TXEPT) == 0);
	while(!(IFG1 & URXIFG0));
	data = RXBUF0 << 8;

	TXBUF0 = 0xfe;
	while ((U0TCTL & TXEPT) == 0);
	while(!(IFG1 & URXIFG0));
	data |= RXBUF0;
	CC2420_DISABLE();
	return data;
}

void write_register(uint8_t addr, uint16_t payload) {
	/* RAMBIT|R/W|6-Bit addr */
	uint8_t status = (addr & 0x3f);

	CC2420_ENABLE();
	TXBUF0 = status;
	while ((U0TCTL & TXEPT) == 0);
	while(!(IFG1 & URXIFG0));
	status = RXBUF0;

	TXBUF0 = (uint8_t) ((payload >> 8) & 0x00ff);
	while ((U0TCTL & TXEPT) == 0);

	TXBUF0 = (uint8_t) (payload & 0x00ff);
	while ((U0TCTL & TXEPT) == 0);
	CC2420_DISABLE();
}

void read_rx_fifo(uint8_t *data, uint8_t size) {
	/* RAMBIT|R/W|6-Bit addr */
	uint8_t status = 0x40 | (CC2420_RXFIFO & 0x3f);
	uint8_t i;
	CC2420_ENABLE();
	TXBUF0 = status;
	//while ((U0TCTL & TXEPT) == 0);
	while(!(IFG1 & URXIFG0));
	status = RXBUF0;

	for(i = 0; i < size; ++i) {
		TXBUF0 = 0x00;
		//while ((U0TCTL & TXEPT) == 0);
		while(!(IFG1 & URXIFG0));
		data[i] = RXBUF0;
	}
	CC2420_DISABLE();
}

void write_tx_buffer(uint8_t *data, uint8_t size) {
	/* RAMBIT|R/W|6-Bit addr */
	uint8_t status = (CC2420_TXFIFO & 0x3f);
	uint8_t i;

	CC2420_ENABLE();
	TXBUF0 = status;
	while ((U0TCTL & TXEPT) == 0);
	while(!(IFG1 & URXIFG0));
	status = RXBUF0;

	for(i = 0; i < size; ++i) {
		TXBUF0 = data[i];
		while ((U0TCTL & TXEPT) == 0);
	}

	CC2420_DISABLE();
}
uint8_t command_strobe(uint8_t addr) {
	/* 00|6-Bit addr */
	uint8_t data = addr & 0x0f;
	CC2420_ENABLE();
	TXBUF0 = data;
	while ((U0TCTL & TXEPT) == 0);
	while(!(IFG1 & URXIFG0));
	data = RXBUF0;
	CC2420_DISABLE();
	/* return status */
	return data;
}

void set_channel(uint8_t channel) {
	uint16_t f;
	
	/* Derive frequency programming from the given channel number */
	/* substract the base channel */
	f = (uint16_t) (channel - 11);
	/* muliply with 5, which is channel spacing */
	f = f + (f << 2);
	/* 357 is 2405-2048, 0x4000 is LOCK_THR = 1 */
	f = f + 357 + 0x4000;
	
	//DISABLE_GLOBAL_INT();
	write_register(CC2420_FSCTRL, f);
	//ENABLE_GLOBAL_INT();
}

#pragma vector=PORT1_VECTOR
__interrupt void fifo_rx(void) {
	uint8_t len, i;
	uint8_t protocol[PROTOCOL_OVERHEAD];
	CLEAR_FIFOP_INT();
	LED_GREEN_TOGGLE();
	/* some checks */

	/* check for overflow */
	if(FIFOP_IS_1() && !(FIFO_IS_1())) {
		command_strobe(CC2420_SFLUSHRX);
		command_strobe(CC2420_SFLUSHRX);
		return;
	}

	/* process data */
	/* first byte is length of packet */
	read_rx_fifo(&len, 1);
	len = len & 0x7f;
	len -= PROTOCOL_OVERHEAD;
	/* read protocol data, 3 bytes header, 2 bytes footer */
	read_rx_fifo(protocol, 3);
	read_rx_fifo((uint8_t*) &g_data[g_buf_idx][g_idx], len);
	read_rx_fifo(protocol+3, 2);
	/* transform to num of words */
	g_idx += len/2;
	/* workaround */
	g_data[g_buf_idx][g_idx-1] = g_data[g_buf_idx][g_idx-2];
	if( g_idx == NUM_SAMPLES ) {
		g_buf_idx = (g_buf_idx+0x1)%NUM_BUFFERS;
		g_idx = 0;
		if((g_buf_idx - g_buf_usr_idx + NUM_BUFFERS)%NUM_BUFFERS >= BUFFER_DELAY)
			*g_flag = 1;
	}
	//command_strobe(CC2420_SFLUSHRX);
	//command_strobe(CC2420_SFLUSHRX);
	/* reset WDT */
	//WDTCTL = WDTPW + WDTCNTCL;
}

#pragma vector=USART0RX_VECTOR
__interrupt void usart0_rx(void)
{
	IFG1 &= ~URXIFG0;
}
