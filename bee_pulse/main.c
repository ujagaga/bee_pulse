/*
 * main.c
 *
 *  Created on: Dec 1, 2017
 *      Author: ujagaga
 */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "reg_helper.h"
#include "GFX.h"
#include "TouchScreen.h"
#include "main.h"

#include "time_ctrl.h"
#include "uart.h"


#define ON		true
#define OFF		false
#define LONG_PRESS_TIME		(2500)

static timing_t timeConfig = {0};
uint16_t main_ID;

uint8_t EEMEM savedPulse = 10;		/* x1us */
uint8_t EEMEM savedPause = 3;		/* x1ms */
uint8_t EEMEM savedActive = 11;		/* x10ms */
uint8_t EEMEM savedInactive = 10;	/* x0.1s */
uint8_t EEMEM savedTotal = 10;		/* x1min */

gfx_btn btn_pulseUp;
gfx_btn btn_pulseDown;
gfx_btn btn_pauseUp;
gfx_btn btn_pauseDown;
gfx_btn btn_activeUp;
gfx_btn btn_activeDown;
gfx_btn btn_inactiveUp;
gfx_btn btn_inactiveDown;
gfx_btn btn_totalUp;
gfx_btn btn_totalDown;
gfx_btn btn_start;

#define BTN_COUNT				(10u)

typedef struct{
	gfx_btn* btnPtr;			/* Pointer to the button */
	uint32_t touctTimestamp;	/* time of the last detected touch */
	uint8_t* ctrlVarPtr;		/* pointer to the variable to control with this button */
	uint8_t increment;			/* Increment to add to the variable when this button is pressed */
	uint16_t cursorY;			/* location on the screen of the variable to re-draw */
}btnUpdate_t;

btnUpdate_t touchArray[BTN_COUNT] = {{0}};

bool timerStarted = false;

static void refreshScr(void){
	GFX_setCursor(2, 2);
	GFX_setTextSize(2);

	GFX_setTextColor(BLUE, SCR_CLR);
	GFX_printStr("Puls:\n");
	GFX_setTextColor(GREEN, SCR_CLR);
	GFX_setTextSize(4);
	touchArray[0].cursorY = GFX_getCursorY();
	touchArray[1].cursorY = GFX_getCursorY();
	GFX_printDec(timeConfig.pulse);
	GFX_setTextSize(2);
	GFX_setCursor(130, GFX_getCursorY() + 12);
	GFX_printStr("us ");
	GFX_setCursor(3, GFX_getCursorY() + 24);
	GFX_drawFastHLine(GFX_getCursorX(), GFX_getCursorY() - 4, GFX_getWidth(), RED);

	GFX_setTextColor(BLUE, SCR_CLR);
	GFX_printStr("Pauza:\n");
	GFX_setTextColor(GREEN, SCR_CLR);
	GFX_setTextSize(4);
	touchArray[2].cursorY = GFX_getCursorY();
	touchArray[3].cursorY = GFX_getCursorY();
	GFX_printDec(timeConfig.pause);
	GFX_setTextSize(2);
	GFX_setCursor(130, GFX_getCursorY() + 12);
	GFX_printStr("ms ");
	GFX_setCursor(3, GFX_getCursorY() + 24);
	GFX_drawFastHLine(GFX_getCursorX(), GFX_getCursorY() - 4, GFX_getWidth(), RED);

	GFX_setTextColor(BLUE, SCR_CLR);
	GFX_printStr("Activan:\n");
	GFX_setTextColor(GREEN, SCR_CLR);
	GFX_setTextSize(4);
	touchArray[4].cursorY = GFX_getCursorY();
	touchArray[5].cursorY = GFX_getCursorY();
	GFX_printDec(timeConfig.active * 1000);
	GFX_setTextSize(2);
	GFX_setCursor(130, GFX_getCursorY() + 12);
	GFX_printStr("ms ");
	GFX_setCursor(3, GFX_getCursorY() + 24);
	GFX_drawFastHLine(GFX_getCursorX(), GFX_getCursorY() - 4, GFX_getWidth(), RED);

	GFX_setTextColor(BLUE, SCR_CLR);
	GFX_printStr("Neactivan:\n");
	GFX_setTextColor(GREEN, SCR_CLR);
	GFX_setTextSize(4);
	touchArray[6].cursorY = GFX_getCursorY();
	touchArray[7].cursorY = GFX_getCursorY();
	GFX_printDec(timeConfig.inactive * 1000);
	GFX_setTextSize(2);
	GFX_setCursor(130, GFX_getCursorY() + 12);
	GFX_printStr("ms ");
	GFX_setCursor(3, GFX_getCursorY() + 24);
	GFX_drawFastHLine(GFX_getCursorX(), GFX_getCursorY() - 4, GFX_getWidth(), RED);

	GFX_setTextColor(BLUE, SCR_CLR);
	GFX_printStr("Trajanje:\n");
	GFX_setTextColor(GREEN, SCR_CLR);
	GFX_setTextSize(4);
	touchArray[8].cursorY = GFX_getCursorY();
	touchArray[9].cursorY = GFX_getCursorY();
	GFX_printDec(timeConfig.total);
	GFX_setTextSize(2);
	GFX_setCursor(130, GFX_getCursorY() + 12);
	GFX_printStr("min ");
	GFX_setCursor(3, GFX_getCursorY() + 24);
	GFX_drawFastHLine(GFX_getCursorX(), GFX_getCursorY() - 4, GFX_getWidth(), RED);
}

static void drawButtons(void){
	touchArray[0].btnPtr = &btn_pulseUp;
	touchArray[0].ctrlVarPtr = &timeConfig.pulse;

	touchArray[1].btnPtr = &btn_pulseDown;
	touchArray[1].ctrlVarPtr = &timeConfig.pulse;

	touchArray[2].btnPtr = &btn_pauseUp;
	touchArray[2].ctrlVarPtr = &timeConfig.pause;

	touchArray[3].btnPtr = &btn_pauseDown;
	touchArray[3].ctrlVarPtr = &timeConfig.pause;

	touchArray[4].btnPtr = &btn_activeUp;
	touchArray[4].ctrlVarPtr = &timeConfig.active;

	touchArray[5].btnPtr = &btn_activeDown;
	touchArray[5].ctrlVarPtr = &timeConfig.active;

	touchArray[6].btnPtr = &btn_inactiveUp;
	touchArray[6].ctrlVarPtr = &timeConfig.inactive;

	touchArray[7].btnPtr = &btn_inactiveDown;
	touchArray[7].ctrlVarPtr = &timeConfig.inactive;

	touchArray[8].btnPtr = &btn_totalUp;
	touchArray[8].ctrlVarPtr = &timeConfig.total;

	touchArray[9].btnPtr = &btn_totalDown;
	touchArray[9].ctrlVarPtr = &timeConfig.total;

	uint8_t i;
	for(i = 0; i < BTN_COUNT; i++){
		touchArray[i].btnPtr->fillcolor = BLUE;
		touchArray[i].btnPtr->outlinecolor = WHITE;
		touchArray[i].btnPtr->textcolor = BLACK;
		touchArray[i].btnPtr->textsize = 2;
		touchArray[i].btnPtr->width = 35;
		touchArray[i].btnPtr->height = 30;
		touchArray[i].touctTimestamp = 0;

		if((i % 2) == 0){
			touchArray[i].btnPtr->x = 175;
			touchArray[i].btnPtr->y = 15 + 52 * (i / 2);
			touchArray[i].btnPtr->label = "+";
			touchArray[i].increment = 1;
		}else{
			touchArray[i].btnPtr->x = 220;
			touchArray[i].btnPtr->y = 35 + 52 * (i / 2);
			touchArray[i].btnPtr->label = "-";
			touchArray[i].increment = -1;
		}
		GFX_btnDraw(touchArray[i].btnPtr, false);
	}

	btn_start.fillcolor = GREEN;
	btn_start.label = "Start";
	btn_start.outlinecolor = WHITE;
	btn_start.textcolor = BLACK;
	btn_start.textsize = 2;
	btn_start.x = 120;
	btn_start.y = 290;
	btn_start.width = 220;
	btn_start.height = 40;
	GFX_btnDraw(&btn_start, false);

}

static void drawCfgVar(uint8_t id)
{
	GFX_setTextSize(4);
	GFX_setCursor(0,   touchArray[id].cursorY);

	uint16_t outVal = *touchArray[id].ctrlVarPtr;

	if((id > 3) && (id < 8)){
		outVal *= 1000;
	}

	GFX_printDec(outVal);
}


static void init( void ){
	/* Init uart for debugging */
	UART_init();

	/* Read timing configuration from EEPROM and initialize counter. */
	timeConfig.pulse = eeprom_read_byte(&savedPulse);
	if((timeConfig.pulse < 10) || (timeConfig.pulse > 200)){
		timeConfig.pulse = 10;
	}

	timeConfig.pause = eeprom_read_byte(&savedPause);
	if((timeConfig.pause < 3) || (timeConfig.pause > 100)){
		timeConfig.pause = 3;
	}

	timeConfig.active = eeprom_read_byte(&savedActive);
	if((timeConfig.active < 1) || (timeConfig.active > 20)){
		timeConfig.active = 1;
	}

	timeConfig.inactive = eeprom_read_byte(&savedInactive);
	if((timeConfig.inactive < 10) || (timeConfig.inactive > 50)){
		timeConfig.inactive = 10;
	}

	timeConfig.total = eeprom_read_byte(&savedTotal);
	if((timeConfig.total < 10) || (timeConfig.total > 60)){
		timeConfig.total = 10;
	}

	TCTRL_init(&timeConfig);

	/* Initialize display */
	main_ID = GFX_init();
	UART_printStr("\n\rLCD ID: 0x");
	UART_printHex(main_ID);

	GFX_setRotation(2);
	GFX_fillScreen(BLACK);
	GFX_setTextColor(GREEN, GREEN);

	refreshScr();
	drawButtons();
}

void MAIN_timerControl(bool start)
{
	if(start){
		eeprom_update_byte(&savedPulse, timeConfig.pulse);
		eeprom_update_byte(&savedPulse, timeConfig.pause);
		eeprom_update_byte(&savedPulse, timeConfig.active);
		eeprom_update_byte(&savedPulse, timeConfig.inactive);
		eeprom_update_byte(&savedPulse, timeConfig.total);

		btn_start.label = "Stop";
		btn_start.fillcolor = RED;
		btn_start.textcolor = YELLOW;
		GFX_btnDraw(&btn_start, false);
		btn_start.label = "Start";
		btn_start.fillcolor = GREEN;
		btn_start.textcolor = BLACK;
		timerStarted = true;

//		TCTRL_start();
	}else{
		btn_start.label = "Start";
		btn_start.fillcolor = GREEN;
		btn_start.textcolor = BLACK;

		GFX_btnDraw(&btn_start, false);
		timerStarted = false;
	}
}


void MAIN_processButtons(TSPoint* touchPoint){

	uint32_t timestamp = TCTRL_micros();

	uint8_t i;
	for(i = 0; i < BTN_COUNT; i++){
		GFX_btnUpdate(touchArray[i].btnPtr, touchPoint);

		if(GFX_btnJustPressed(&btn_pulseUp)){
			*touchArray[i].ctrlVarPtr += touchArray[i].increment;
			touchArray[i].touctTimestamp = timestamp;
			drawCfgVar(i);
		}

		if(GFX_btnIsPressed(&btn_pulseUp)){
			if((timestamp - touchArray[i].touctTimestamp) > LONG_PRESS_TIME){
				*touchArray[i].ctrlVarPtr += touchArray[i].increment * 10;
				touchArray[i].touctTimestamp = timestamp;
				drawCfgVar(i);
			}
		}
	}
	GFX_btnUpdate(&btn_start, touchPoint);

	if(GFX_btnJustReleased(&btn_start)){
		MAIN_timerControl(ON);
	}else if(timerStarted){
		MAIN_timerControl(OFF);
	}
}

int main( void )
{
	init();

	for(;;){
		TCTRL_process();
	}

	return 0;
}
