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

#ifndef _DAC_H_
#define _DAC_H_

#include <stdint.h>

extern void init_dac(void);
extern void set_dac_data(volatile int16_t *data, uint16_t num);

#endif
