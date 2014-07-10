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

#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>

/* every 100us with 4.5MHz  */
#define SAMPLING_RATE 450

/* 500 samples for a delay of 50ms and smapling rate of 8kHz, multiple of 112 */
#define NUM_SAMPLES 504

extern void init_adc(volatile uint16_t* flag);
extern int16_t* get_samples(void);

#endif
