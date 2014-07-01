#ifndef _DEFINES_H_
#define _DEFINES_H_

#include "usb_serial.h"
#include <msp430.h>

#define LED_RED_ON() (P5OUT &= ~0x10)
#define LED_RED_OFF() (P5OUT |= 0x10)
#define LED_RED_TOGGLE() (P5OUT ^= 0x10)
#define LED_GREEN_ON() (P5OUT &= ~0x20)
#define LED_GREEN_OFF() (P5OUT |= 0x20)
#define LED_GREEN_TOGGLE() (P5OUT ^= 0x20)
#define LED_BLUE_ON() (P5OUT &= ~0x40)
#define LED_BLUE_OFF() (P5OUT |= 0x40)
#define LED_BLUE_TOGGLE() (P5OUT ^= 0x40)

#define CPU_FREQ_MHZ (4.5)

#define NUM_BUFFERS 3

#define ENABLE_GLOBAL_INT() __enable_interrupt()
#define DISABLE_GLOBAL_INT() __disable_interrupt()

#ifndef DAC_TB
extern void delay_us(uint16_t us);
#endif

/* timers are working on same rate,
   but depending on mode only on is active
*/
#ifdef TX
#define TIMER TAR
#else
#define TIMER TBR
#endif

#ifdef DEBUG

#include <stdio.h>
#define debug_print(x) usb_send_string(x)
#define debug_printf usb_printf

#else

#define debug_print(x)
#define debug_printf

#endif

#endif
