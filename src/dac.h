#ifndef _DAC_H_
#define _DAC_H_

#include <stdint.h>

extern void init_dac(void);
extern void set_dac_data(volatile int16_t *data, uint16_t num);

#endif
