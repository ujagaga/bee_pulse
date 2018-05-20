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
#include <stdbool.h>
#include <stddef.h>
#include "reg_helper.h"
#include "TouchScreen.h"
#include "time_ctrl.h"
#include "main.h"

timing_t* timingSetup = NULL;

static uint8_t pulseCount;
static uint8_t activeCount;
static bool pulseFlag;
static bool runFlag;
static uint32_t startTimestamp;
volatile uint32_t timer1_micros;

static void out(bool state){
	if(state){
		OUT_PORT1 |= (1 << OUT_PIN1);
		OUT_PORT2 &= ~(1 << OUT_PIN2);
	}else{
		OUT_PORT2 |= (1 << OUT_PIN2);
		OUT_PORT1 &= ~(1 << OUT_PIN1);
	}
}

/* Triggers every 2 us */
ISR (TIMER1_COMPA_vect)
{
    timer1_micros++;

    if(runFlag){
    	if(pulseFlag){
    		if(pulseCount < timingSetup->pulse){
				out(true);
			}else{
				out(false);
			}
    		pulseCount++;

    		if(pulseCount == (timingSetup->pulse + timingSetup->pause)){
    			pulseCount = 0;
			}
    	}else{
    		out(false);
    	}

    	activeCount++;
    }else{
    	out(false);
    }
}

void TCTRL_init(timing_t* timingParameters){

	timingSetup = timingParameters;

	pulseCount = 0;
	activeCount = 0;
	pulseFlag = false;
	runFlag = false;
	startTimestamp = 0;
	timer1_micros = 0;

	PIN_LOW(OUT_PORT1, OUT_PIN1);
	PIN_OUTPUT(OUT_PORT1, OUT_PIN1);
	PIN_HIGH(OUT_PORT2, OUT_PIN2);
	PIN_OUTPUT(OUT_PORT2, OUT_PIN2);

	// CTC mode, Clock/8
	TCCR1B |= (1 << WGM12) | (1 << CS11);

	// Load the high byte, then the low byte
	// into the output compare
	OCR1AH = (CTC_MATCH_OVERFLOW >> 8);
	OCR1AL = CTC_MATCH_OVERFLOW;

	// Enable the compare match interrupt
	TIMSK1 |= (1 << OCIE1A);

}


void TCTRL_start(void)
{
	if(timingSetup != NULL){
		startTimestamp = timer1_micros;
		runFlag = true;
		sei();
	}
}

void TCTRL_process(void)
{
	TSPoint point;

	if(runFlag){
		if(activeCount > timingSetup->active){
			/* We have more time so we can test touch */
			TS_getPoint(&point);

			if(point.z > 0){
				cli();
				runFlag = false;
				out(false);
				MAIN_timerControl(false);
			}
		}

		if((timer1_micros - startTimestamp) < timingSetup->total){
			if(activeCount < (timingSetup->active + timingSetup->inactive)){
				if(activeCount < timingSetup->active){
					pulseFlag = true;
				}else{
					pulseFlag = false;
				}
			}else{
				activeCount = 0;
			}
		}else{
			cli();
			pulseFlag = false;
			runFlag = false;
			out(false);
			MAIN_timerControl(false);
		}
	}else{
		TS_getPoint(&point);
		MAIN_processButtons(&point);
	}
}



