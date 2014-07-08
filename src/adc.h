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
