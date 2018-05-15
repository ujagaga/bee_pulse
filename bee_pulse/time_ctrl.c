/*
 * millis.c
 *
 *  Created on: Feb 21, 2018
 *      Author: ujagaga
 */


#define CTC_MATCH_OVERFLOW ((F_CPU / 1000000) / 4) /* 2us */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

static uint8_t pulseTime = 0;
static uint8_t pauseTime = 0;
static uint8_t activeTime = 0;
static uint8_t inactiveTime = 0;
static uint8_t totalTime = 0;

volatile uint32_t timer1_micros;

/* Triggers every 2 us */
ISR (TIMER1_COMPA_vect)
{
    timer1_micros++;
}

void TCTRL_start(void){
	// CTC mode, Clock/8
	TCCR1B |= (1 << WGM12) | (1 << CS11);

	// Load the high byte, then the low byte
	// into the output compare
	OCR1AH = (CTC_MATCH_OVERFLOW >> 8);
	OCR1AL = CTC_MATCH_OVERFLOW;

	// Enable the compare match interrupt
	TIMSK1 |= (1 << OCIE1A);

	sei();
}

void TCTRL_setPulse(uint8_t val)
{
	pulseTime = val;
}

void TCTRL_setPause(uint8_t val)
{
	pauseTime = val;
}

void TCTRL_setActive(uint8_t val)
{
	activeTime = val;
}

void TCTRL_setInactive(uint8_t val)
{
	inactiveTime = val;
}

void TCTRL_setTotal(uint8_t val)
{
	totalTime = val;
}

