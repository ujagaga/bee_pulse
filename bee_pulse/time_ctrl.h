/*
 * millis.h
 *
 *  Created on: Feb 21, 2018
 *      Author: ujagaga
 */

#ifndef TIME_CTRL_H_
#define TIME_CTRL_H_

#include <stdint.h>

extern volatile uint32_t timer1_micros;

/* Returns microseconds passed divided by 2 */
#define TCTRL_micros() 	timer1_micros

extern void TCTRL_start(void);
extern void TCTRL_setPulse(uint8_t val);
extern void TCTRL_setPause(uint8_t val);
extern void TCTRL_setActive(uint8_t val);
extern void TCTRL_setInactive(uint8_t val);
extern void TCTRL_setTotal(uint8_t val);

#endif /* TIME_CTRL_H_ */
