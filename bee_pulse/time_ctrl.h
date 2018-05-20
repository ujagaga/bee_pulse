/*
 * millis.h
 *
 *  Created on: Feb 21, 2018
 *      Author: ujagaga
 */

#ifndef TIME_CTRL_H_
#define TIME_CTRL_H_

#include <stdint.h>

typedef struct{
	uint8_t pulse;
	uint8_t pause;
	uint8_t active;
	uint8_t inactive;
	uint8_t total;
}timing_t;

extern volatile uint32_t timer1_micros;

/* Returns microseconds */
#define TCTRL_micros() 		(timer1_micros << 2)

extern void TCTRL_init(timing_t* timingParameters);
extern void TCTRL_process(void);
extern void TCTRL_start(void);

#endif /* TIME_CTRL_H_ */
