#include <msp430.h>
#include "usb_serial.h"
#include "defines.h"
#include <stdarg.h>
#include "rf.h"

/* use UART1 */

void init_usb_serial(void) {
	/* set to use UART */
	P3SEL |= 0xC0;
	/* Enable USART1 TXD/RXD */
	ME2 |= UTXE1 + URXE1;
	/* 8-bit character */
	U1CTL |= CHAR;
	/* UCLK = SMCL */
	U1TCTL |= SSEL1;
	/* baudrate = 4800, 4.5MHz/4800 */
	//U1BR0 = 0xaa;
	//U1BR1 = 0x03;
	/* baudrate = 115200, 4.5MHz/115200 */
	U1BR0 = 0x27;
	U1BR1 = 0x00;
	/* no modulation */
	U1MCTL = 0x00;
	/* Initialize USART state machine */
	U1CTL &= ~SWRST;
	/* Enable USART1 RX interrupt */
	IE2 |= URXIE1;
	IFG2 &= ~URXIFG1;

	debug_print("Initliazied USB Serial\n");
}

#pragma vector=USART1RX_VECTOR
__interrupt void usart1_rx (void)
{
	uint8_t status;
	LED_BLUE_TOGGLE();
	IFG2 &= ~URXIFG1;
	//command_strobe(CC2420_SRXON);
	command_strobe(CC2420_SFLUSHRX);
	status = command_strobe(CC2420_SFLUSHRX);
	usb_printf("Status: 0x%x\n", status);
}

void usb_send_string(char *data) {
	volatile char *dat = data;
	while(*dat != 0x00) {
		while(!(IFG2 & UTXIFG1));
		TXBUF1 = *dat;
		++dat;
	}
}

void usb_send_data(char *data, uint16_t size) {
	uint16_t i = size;
	volatile char *dat = data;
	while(i > 0) {
		while(!(IFG2 & UTXIFG1));
		TXBUF1 = *dat;
		++dat;
		--i;
	}
}

void usb_printf(char *fmt, ...) {
	va_list args;
	char buf[64];
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	usb_send_string(buf);
	va_end(args);
}
