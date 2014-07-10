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
